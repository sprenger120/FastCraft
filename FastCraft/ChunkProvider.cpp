/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011  Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "ChunkProvider.h"
#include <Poco/Exception.h>
#include <Poco/DeflatingStream.h>
#include <Poco/Exception.h>
#include "Structs.h"
#include "NetworkIO.h"
#include "PlayerThread.h"
#include "ChunkMath.h"
#include "ChunkRoot.h"

using std::cout;
using std::endl;
using std::memcpy;
using std::stringstream;
using Poco::DeflatingOutputStream;


ChunkProvider::ChunkProvider(ChunkRoot* pChunkRoot,NetworkIO* pNetworkIO) :
_vSpawnedChunks(0),
	_ViewDistance(10),
	_ChunkSet(10),
	_stringStrm(),
	_deflatingStrm(_stringStrm,Poco::DeflatingStreamBuf::STREAM_ZLIB,-1)
{
	_pChunkRoot = pChunkRoot;
	_pNetwork = pNetworkIO;
	_fNewConnection = false;
	_fConnected = false;
}

ChunkProvider::~ChunkProvider() {
	_vSpawnedChunks.clear();
}

void ChunkProvider::newConnection() {
	if (_fConnected) {
		throw Poco::RuntimeException("ChunkProvider: Already connected");
	}
	_fConnected = true;
	_fNewConnection = true;
	_ChunkSet.clear();
}

void ChunkProvider::Disconnect() {
	_fConnected = false;
	_vSpawnedChunks.clear();
}

void ChunkProvider::HandleMovement(const EntityCoordinates& PlayerCoordinates) {
	if (!isConnected()) { return; }

	_PlayerCoordinates = ChunkMath::toChunkCoords(PlayerCoordinates);


	if (_fNewConnection) {
		_fNewConnection = false;
		_ChunkSet.regenerate(_PlayerCoordinates);
	}



	if (isFullyCircleSpawned()) { //Full circle is spawned
		if (_ChunkSet.isUp2Date(_PlayerCoordinates)) {
			//Player doesn't leave it's chunk and all chunks were delivered
			return;
		}else{
			//All chunks were delivered but  the player leaved it's chunk
			_ChunkSet.regenerate(_PlayerCoordinates);
			CheckSpawnedChunkList();
			if (!CheckChunkSet()) {
				throw Poco::RuntimeException("Chunk delivering failed: 1");
			}
		}
	}else {
		if (_ChunkSet.isUp2Date(_PlayerCoordinates)) {
			if (!CheckChunkSet()) {
				throw Poco::RuntimeException("Chunk delivering failed: 2");
			}
			return;
		}else{
			_ChunkSet.regenerate(_PlayerCoordinates);
			CheckSpawnedChunkList();
			if (CheckChunkSet()) {
				throw Poco::RuntimeException("Chunk delivering failed: 3");
			}
		}
	}
}

bool ChunkProvider::isFullyCircleSpawned() {
	return (_vSpawnedChunks.size() >= _ChunkSet.CalculateChunkCount());
}


void ChunkProvider::sendChunk(MapChunk* pChunk) {
	//Network Stuff

	if (!isConnected()) {
		throw Poco::Exception("Not connected");
	}


	if (pChunk==NULL) {
		cout<<"NULLCHUNK"<<"\n";
		throw Poco::Exception("Nullchunk");
	}

	sendPreChunk_Spawn(pChunk->X,pChunk->Z);

	_pNetwork->addByte(0x33);
	_pNetwork->addInt((pChunk->X)<<4);
	_pNetwork->addShort(0);
	_pNetwork->addInt((pChunk->Z)<<4);
	_pNetwork->addByte(15);
	_pNetwork->addByte(127);
	_pNetwork->addByte(15);

	//deflate
	_deflatingStrm.write(pChunk->Blocks,FC_CHUNK_BLOCKCOUNT);
	_deflatingStrm.write(pChunk->Metadata,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(pChunk->BlockLight,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(pChunk->SkyLight,FC_CHUNK_NIBBLECOUNT);

	_deflatingStrm.flush();

	_pNetwork->addInt(_stringStrm.str().length());
	_pNetwork->Str().append(_stringStrm.str());


	_stringStrm.clear();
	_deflatingStrm.clear();

	_pNetwork->Flush();

	//Add to spawned list
	ChunkCoordinates Coord;
	Coord.X = pChunk->X;
	Coord.Z = pChunk->Z;

	_vSpawnedChunks.push_back(Coord);
}


void ChunkProvider::sendPreChunk_Spawn(int X,int Z){
	if (!isConnected()) {
		throw Poco::Exception("Not connected");
	}

	_pNetwork->addByte(0x32);
	_pNetwork->addInt(X);
	_pNetwork->addInt(Z);
	_pNetwork->addBool(true);
	_pNetwork->Flush();
}

void ChunkProvider::sendPreChunk_Despawn(int X,int Z){
	if (!isConnected()) {
		throw Poco::Exception("Not connected");
	}

	cout<<"despawn chunk"<<"\n";

	_pNetwork->addByte(0x32);
	_pNetwork->addInt(X);
	_pNetwork->addInt(Z);
	_pNetwork->addBool(false);
	_pNetwork->Flush();


	//Remove from spawned list
	for (int x=0;x<=_vSpawnedChunks.size()-1;x++) {
		if(_vSpawnedChunks[x].X == X && _vSpawnedChunks[x].Z == Z) {
			_vSpawnedChunks.erase(_vSpawnedChunks.begin()+x);
			break;
		}
	}
}

bool ChunkProvider::isConnected() {
	return _fConnected;
}

bool ChunkProvider::isSpawned(ChunkCoordinates coord) {
	if(_vSpawnedChunks.size() == 0 ) {return false;}
	for ( int x=0;x<=_vSpawnedChunks.size()-1;x++) {
		if (_vSpawnedChunks[x].X == coord.X && _vSpawnedChunks[x].Z == coord.Z) {
			return true;
		}
	}
	return false;
}

bool ChunkProvider::CheckChunkSet() {
	//Check chunk set
	int iSentChunks=0;

	try {
		//Check chunk who player stands on
		if ( ! isSpawned(_PlayerCoordinates)) {
			sendChunk(_pChunkRoot->getChunk(_PlayerCoordinates.X,_PlayerCoordinates.Z));
			iSentChunks++;
			if (iSentChunks==3) {
				return true;
			}
		}

		for (int x=0;x<=_ChunkSet.CalculateChunkCount()-1;x++) {
			if ( ! isSpawned(_ChunkSet.at(x))) {
				sendChunk(_pChunkRoot->getChunk( _ChunkSet.at(x).X,_ChunkSet.at(x).Z));
				iSentChunks++;
				if (iSentChunks==3) {
					return true;
				}
			}
		}
	}catch(Poco::RuntimeException&err ) {
		cout<<"error:"<<err.message()<<"\n";
		return false;
	}
	return true;
}


void ChunkProvider::CheckSpawnedChunkList() {
	for (int x=0;x<=_vSpawnedChunks.size()-1;x++) {
		if(  ChunkMath::Distance(_PlayerCoordinates,_vSpawnedChunks[x]) > _ViewDistance) {
			sendPreChunk_Despawn(_vSpawnedChunks[x].X,_vSpawnedChunks[x].Z);
		}
	}
}
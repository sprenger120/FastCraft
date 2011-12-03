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
#include "PackingThread.h"

using std::cout;
using std::endl;
using std::memcpy;
using std::stringstream;
using Poco::DeflatingOutputStream;


ChunkProvider::ChunkProvider(ChunkRoot* pChunkRoot,NetworkIO* pNetworkIO,PackingThread* pPackingThread) :
_vSpawnedChunks(0),
	_ViewDistance(10)
{
	_pChunkRoot = pChunkRoot;
	_pNetwork = pNetworkIO;
	_pPackingThread = pPackingThread;
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
		_oldPlayerCoordinates = _PlayerCoordinates;
	}


	bool fFullCircle = isFullyCircleSpawned();
	bool fMoved = playerChangedPosition();

	if (fFullCircle && !fMoved) {
		return;
	}


	if ( fFullCircle && fMoved || //all chunks delivered, moved to another chunk
		!fFullCircle && fMoved || //not all chunks delvered,  in same chunk 
		!fFullCircle && !fMoved // not all chunks delvered, moved to another chunk	
		) {
			if (fMoved) {
				CheckSpawnedChunkList();
			}

			if (!CheckChunkSet()) {
				throw Poco::RuntimeException("Chunk delivering failed");
			}
	}

	_oldPlayerCoordinates = _PlayerCoordinates;
}

bool ChunkProvider::isFullyCircleSpawned() {
	return (_vSpawnedChunks.size() >= CalculateChunkCount());
}


void ChunkProvider::sendSpawn(int X,int Z) {
	_pNetwork->Lock();

	_pNetwork->addByte(0x32);
	_pNetwork->addInt(X);
	_pNetwork->addInt(Z);
	_pNetwork->addBool(true);
	_pNetwork->Flush();

	_pNetwork->UnLock();
}

void ChunkProvider::sendDespawn(int X,int Z){
	if (!isConnected()) {
		throw Poco::Exception("Not connected");
	}

	_pNetwork->Lock();
	_pNetwork->addByte(0x32);
	_pNetwork->addInt(X);
	_pNetwork->addInt(Z);
	_pNetwork->addBool(false);
	_pNetwork->Flush();
	_pNetwork->UnLock();

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
	MapChunk* pChunk;
	PackJob Job;
	ChunkCoordinates SquareStart,SquareEnd,Temp;
	
	vector<PackJob> vJobs;


	Job.pNetwork = _pNetwork;

	try {
		//Check chunk who player stands on
		if ( ! isSpawned(_PlayerCoordinates)) {
			pChunk = _pChunkRoot->getChunk(_PlayerCoordinates.X,_PlayerCoordinates.Z);
			if (pChunk==NULL) {
				cout<<"nullpointer"<<"\n";
				//Todo: end of world reached message in players chat
				return true;
			}

			Job.X = _PlayerCoordinates.X;
			Job.Z = _PlayerCoordinates.Z;
			Job.pChunk = pChunk;

			sendSpawn(Job.X,Job.Z);
			
			vJobs.push_back(Job);

			AddChunkToList(_PlayerCoordinates.X,_PlayerCoordinates.Z);
		}

		SquareStart.X = _PlayerCoordinates.X - (_ViewDistance/2) - 1;
		SquareStart.Z = _PlayerCoordinates.Z - (_ViewDistance/2) - 1;

		SquareEnd.X = SquareStart.X + _ViewDistance;
		SquareEnd.Z = SquareStart.Z + _ViewDistance;

		for ( int X = SquareStart.X;X<=SquareEnd.X;X++) {
			for ( int Z = SquareStart.Z;Z<=SquareEnd.Z;Z++) {
				Temp.X = X;
				Temp.Z = Z;
				
				if ( ! isSpawned(Temp)) {
					pChunk = _pChunkRoot->getChunk(X,Z);
					if (pChunk==NULL) {
						cout<<"nullpointer"<<"\n";
						//Todo: end of world reached message in players chat
						return true;
					}

					Job.X = X;
					Job.Z = Z;
					Job.pChunk = pChunk;

					sendSpawn(X,Z);
					vJobs.push_back(Job);

					AddChunkToList(X,Z);
				}
			}
		}
	}catch(Poco::RuntimeException&err ) {
		cout<<"error:"<<err.message()<<"\n";
		return false;
	}


	for (int x=0;x<=vJobs.size()-1;x++) {
		_pPackingThread->AddJob(vJobs[x]);
	}

	vJobs.clear();
	return true;
}


void ChunkProvider::CheckSpawnedChunkList() {
	if (_vSpawnedChunks.size() == 0) { return; }
	for (int x=0;x<=_vSpawnedChunks.size()-1;x++) {
		if(  ChunkMath::Distance(_PlayerCoordinates,_vSpawnedChunks[x]) > (_ViewDistance/2)) {
			sendDespawn(_vSpawnedChunks[x].X,_vSpawnedChunks[x].Z);
		}
	}
}

bool ChunkProvider::playerChangedPosition() {
	if (_oldPlayerCoordinates.X == _PlayerCoordinates.X && _oldPlayerCoordinates.Z == _PlayerCoordinates.Z) {
		return false;
	}else{
		return true;
	}
}

int ChunkProvider::CalculateChunkCount() {
	int iVD = _ViewDistance / 2;
	return iVD * 4 + (iVD * iVD) * 4 + 1; 
}

void ChunkProvider::AddChunkToList(int X,int Z) {
	ChunkCoordinates Coord;
	Coord.X = X;
	Coord.Z = Z;

	_vSpawnedChunks.push_back(Coord);
}
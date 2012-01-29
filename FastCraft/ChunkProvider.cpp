/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011 - 2012 Michael Albrecht aka Sprenger120

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
#include "NetworkOutRoot.h"
#include "NetworkOut.h"
#include "PlayerThread.h"
#include "ChunkMath.h"
#include "World.h"
#include "SettingsHandler.h"
#include "Constants.h"
#include "WorldStorage.h"

using std::cout;
using std::endl;
using std::memcpy;
using std::stringstream;
using Poco::DeflatingOutputStream;


ChunkProvider::ChunkProvider(NetworkOutRoot& rNetwork,PackingThread& rPackingThread,PlayerThread* pPlayer) :
	_rNetwork(rNetwork),
	_rPackingThread(rPackingThread),
	_vSpawnedChunks(0)
{
	_pPlayer = pPlayer;
	_pWorld = NULL;
}

ChunkProvider::~ChunkProvider() {
	_vSpawnedChunks.clear();
	_vToBeSendChunks.clear();
}

void ChunkProvider::HandleNewPlayer() {
	_pWorld = WorldStorage::getWorldByName(_pPlayer->getWorldWhoIn());
}

void ChunkProvider::HandleDisconnect() {
	_vSpawnedChunks.clear();
	_vToBeSendChunks.clear();
}

void ChunkProvider::HandleMovement(const EntityCoordinates& PlayerCoordinates) {
	_PlayerCoordinates = ChunkMath::toChunkCoords(PlayerCoordinates);


	if (isChunkListEmpty()) {
		if (!CheckChunkCircle()) {
			throw Poco::RuntimeException("Chunk delivering failed");
		}

		_oldPlayerCoordinates = _PlayerCoordinates;
		return;
	}


	if (_oldPlayerCoordinates.X != _PlayerCoordinates.X ||  _oldPlayerCoordinates.Z != _PlayerCoordinates.Z) {
		CheckSpawnedChunkList(); //Check spawned chunks and despawn if too distant
		if (!CheckChunkCircle()) { //check player's chunk circle and spawn if there is a hole
			throw Poco::RuntimeException("Chunk delivering failed");
		}
		_oldPlayerCoordinates = _PlayerCoordinates;
		return;
	}
}

bool ChunkProvider::isChunkListEmpty() {
	return _vSpawnedChunks.empty();
}


void ChunkProvider::sendSpawn(int X,int Z) {
	NetworkOut Out(&_rNetwork);

	Out.addByte(0x32);
	Out.addInt(X);
	Out.addInt(Z);
	Out.addBool(true);

	Out.Finalize(FC_QUEUE_HIGH);
}

void ChunkProvider::sendDespawn(int X,int Z){
	NetworkOut Out(&_rNetwork);

	Out.addByte(0x32);
	Out.addInt(X);
	Out.addInt(Z);
	Out.addBool(false);

	Out.Finalize(FC_QUEUE_HIGH);
}


bool ChunkProvider::isSpawned(ChunkCoordinates coord) {
	if(_vSpawnedChunks.empty()) {return false;}
	for ( int x=0;x<=_vSpawnedChunks.size()-1;x++) {
		if (_vSpawnedChunks[x].X == coord.X && _vSpawnedChunks[x].Z == coord.Z) {
			return true;
		}
	}
	return false;
}

bool ChunkProvider::CheckChunkCircle() {
	MapChunk* pChunk;
	ChunkCoordinates CircleRoot,Temp;
	PackJob Job;
	int iViewDistance = SettingsHandler::getViewDistance();

	Job.pNetwork = &(_rNetwork);
	Job.pPlayer = _pPlayer;

	/*
	Check already queued chunks
	*/
	if (!_vToBeSendChunks.empty()) {
		bool fDespawn=false; 

		for (int x=_vToBeSendChunks.size()-1;x>=0;x--) {
			if (_vToBeSendChunks[x].X < _PlayerCoordinates.X - iViewDistance) {fDespawn=true;}
			if (_vToBeSendChunks[x].X > _PlayerCoordinates.X + iViewDistance) {fDespawn=true;}
			if (_vToBeSendChunks[x].Z < _PlayerCoordinates.Z - iViewDistance) {fDespawn=true;}
			if (_vToBeSendChunks[x].Z > _PlayerCoordinates.Z + iViewDistance) {fDespawn=true;}

			if(fDespawn) {
				sendDespawn(_vToBeSendChunks[x].X,_vToBeSendChunks[x].Z);
				_vToBeSendChunks.erase(_vToBeSendChunks.begin()+x);
			}
			fDespawn = false;
		}
	}


	try {
		if (!isSpawned(_PlayerCoordinates)) { //Check chunk who player stands on
			pChunk = _pWorld->getChunkByChunkCoordinates(_PlayerCoordinates.X,_PlayerCoordinates.Z);

			Job.X = _PlayerCoordinates.X;
			Job.Z = _PlayerCoordinates.Z;

			Job.pChunk = pChunk;
			sendSpawn(Job.X,Job.Z);
			_vToBeSendChunks.push_back(Job); //Queue chunk
			AddChunkToList(_PlayerCoordinates.X,_PlayerCoordinates.Z);
		}	

		int X;
		int Z;

		for ( int iCircle = 1;iCircle<=iViewDistance;iCircle++) {
			X = CircleRoot.X = _PlayerCoordinates.X + iCircle;
			Z = CircleRoot.Z = _PlayerCoordinates.Z + iCircle;

			int iStep = 1;

			while(1) {
				Temp.X = X;
				Temp.Z = Z;

				if ( ! isSpawned(Temp)) {
					pChunk = _pWorld->getChunkByChunkCoordinates(X,Z);
					
					Job.X = X;
					Job.Z = Z;
					Job.pChunk = pChunk;

					sendSpawn(X,Z);
					_vToBeSendChunks.push_back(Job);
					AddChunkToList(X,Z);
				}

				switch (iStep){
				case 1:
					if (X == CircleRoot.X-(iCircle*2)) {
						Z--;
						iStep++;
						break;
					}
					X--;
					break;
				case 2:
					if (Z == CircleRoot.Z-(iCircle*2)) {
						X++;
						iStep++;
						break;
					}
					Z--;
					break;
				case 3:
					if (X == CircleRoot.X) {
						Z++;
						iStep++;
						break;			
					}
					X++;
					break;
				case 4:
					if (Z == CircleRoot.Z-1) {
						iStep++;
						break;			
					}
					Z++;
					break;
				}


				if (iStep==5) {
					break;
				}

			}
		}
	}catch(Poco::RuntimeException&err ) {
		cout<<"CheckChunkCircle error:"<<err.message()<<"\n";
		return false;
	}
	return true;
}


void ChunkProvider::CheckSpawnedChunkList() {
	if (_vSpawnedChunks.empty()) { return; }

	bool fDespawn=false; 
	int iViewDistance = SettingsHandler::getViewDistance(),x;

	for (x=_vSpawnedChunks.size()-1;x>=0;x--) {
		if (_vSpawnedChunks[x].X < _PlayerCoordinates.X - iViewDistance) {fDespawn=true;}
		if (_vSpawnedChunks[x].X > _PlayerCoordinates.X + iViewDistance) {fDespawn=true;}
		if (_vSpawnedChunks[x].Z < _PlayerCoordinates.Z - iViewDistance) {fDespawn=true;}
		if (_vSpawnedChunks[x].Z > _PlayerCoordinates.Z + iViewDistance) {fDespawn=true;}

		if(fDespawn) {
			sendDespawn(_vSpawnedChunks[x].X,_vSpawnedChunks[x].Z);
			_vSpawnedChunks.erase(_vSpawnedChunks.begin()+x);
		}

		fDespawn = false;
	}
}

void ChunkProvider::AddChunkToList(int X,int Z) {
	ChunkCoordinates Coord;
	Coord.X = X;
	Coord.Z = Z;

	_vSpawnedChunks.push_back(Coord);
}

void ChunkProvider::NextChunk() {
	if (_vToBeSendChunks.empty()) {
		return;
	}
	if(_pPlayer->getChunksInQueue() > 20) { return; }
	PackJob & rJob = _vToBeSendChunks.front();
	_rPackingThread.AddJob(rJob);
	_vToBeSendChunks.erase(_vToBeSendChunks.begin());
}
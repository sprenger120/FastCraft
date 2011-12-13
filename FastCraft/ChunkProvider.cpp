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
#include "SettingsHandler.h"

using std::cout;
using std::endl;
using std::memcpy;
using std::stringstream;
using Poco::DeflatingOutputStream;


ChunkProvider::ChunkProvider(ChunkRoot& rChunkRoot,NetworkIO& rNetworkIO,PackingThread& rPackingThread,PlayerThread* pPlayer) :
	_rChunkRoot(rChunkRoot),
	_rNetwork(rNetworkIO),
	_rPackingThread(rPackingThread),
	_vSpawnedChunks(0)
{
	_pPlayer = pPlayer;
}

ChunkProvider::~ChunkProvider() {
	_vSpawnedChunks.clear();
}

void ChunkProvider::HandleNewPlayer() {
	_fNewPlayer = true;
}

void ChunkProvider::HandleDisconnect() {
	_vSpawnedChunks.clear();
	_fNewPlayer = false;
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
		cout<<"moved"<<"\n";
		CheckSpawnedChunkList(); //Check spawned chunks and despawn if too distant
		if (!CheckChunkCircle()) { //check player's chunk circle and spawn if there is a hole
			throw Poco::RuntimeException("Chunk delivering failed");
		}
		_oldPlayerCoordinates = _PlayerCoordinates;
		return;
	}
}

bool ChunkProvider::isChunkListEmpty() {
	return (_vSpawnedChunks.size() == 0);
}


void ChunkProvider::sendSpawn(int X,int Z) {
	_rNetwork.Lock();

	_rNetwork.addByte(0x32);
	_rNetwork.addInt(X);
	_rNetwork.addInt(Z);
	_rNetwork.addBool(true);
	_rNetwork.Flush();

	_rNetwork.UnLock();
}

void ChunkProvider::sendDespawn(int X,int Z){
	_rNetwork.Lock();
	_rNetwork.addByte(0x32);
	_rNetwork.addInt(X);
	_rNetwork.addInt(Z);
	_rNetwork.addBool(false);
	_rNetwork.Flush();
	_rNetwork.UnLock();
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

bool ChunkProvider::CheckChunkCircle() {
	MapChunk* pChunk;
	PackJob Job;
	ChunkCoordinates CircleRoot,Temp;
	int iPlayerChunkVectorIndex = -1;

	vector<PackJob> vJobs;


	Job.pNetwork = &(_rNetwork);

	try {
		//Check chunk who player stands on
		if ( ! isSpawned(_PlayerCoordinates)) {
			pChunk = _rChunkRoot.getChunk(_PlayerCoordinates.X,_PlayerCoordinates.Z);
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
			iPlayerChunkVectorIndex = vJobs.size()-1;

			AddChunkToList(_PlayerCoordinates.X,_PlayerCoordinates.Z);
		}
		int iViewDistance = SettingsHandler::getViewDistance();

		int X;
		int Z;

		for ( int iCircle = 1;iCircle<=iViewDistance;iCircle++) {
			X = CircleRoot.X = _PlayerCoordinates.X + iCircle;
			Z = CircleRoot.Z = _PlayerCoordinates.Z + iCircle;

			int iStep = 1;

			while(1) {
				Temp.X = X;
				Temp.Z = Z;

				//cout<<"X:"<<X<<" Z:"<<Z<<" Step:"<<iStep<<"\n";


				if ( ! isSpawned(Temp)) {
					pChunk = _rChunkRoot.getChunk(X,Z);
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
		cout<<"error:"<<err.message()<<"\n";
		return false;
	}

	if (iPlayerChunkVectorIndex != -1) {
		//Send player chunk, before send him his position
		_rPackingThread.AddJob(vJobs[iPlayerChunkVectorIndex]);
		vJobs.erase(vJobs.begin() + iPlayerChunkVectorIndex);
	}

	if (_fNewPlayer) {
		_pPlayer->sendLowClientPosition();
		_fNewPlayer = false;
	}

	for (int x=0;x<=vJobs.size()-1;x++) {
		_rPackingThread.AddJob(vJobs[x]);
	}

	vJobs.clear();
	return true;
}


void ChunkProvider::CheckSpawnedChunkList() {
	if (_vSpawnedChunks.size() == 0) { return; }
	
	bool fDespawn=false;
	int iViewDistance = SettingsHandler::getViewDistance();



	for (int x=0;x<=_vSpawnedChunks.size()-1;x++) {
			

		if (_vSpawnedChunks[x].X < _PlayerCoordinates.X - iViewDistance) {fDespawn=true;}
		if (_vSpawnedChunks[x].X > _PlayerCoordinates.X + iViewDistance) {fDespawn=true;}
		if (_vSpawnedChunks[x].Z < _PlayerCoordinates.Z - iViewDistance) {fDespawn=true;}
		if (_vSpawnedChunks[x].Z > _PlayerCoordinates.Z + iViewDistance) {fDespawn=true;}

		if(fDespawn) {
			cout<<"Despawn Chunk X:"<<_vSpawnedChunks[x].X<<" Z:"<<_vSpawnedChunks[x].Z<<
				" PlayerChunk X:"<<_PlayerCoordinates.X<<" Z:"<<_PlayerCoordinates.Z<<"\n";


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
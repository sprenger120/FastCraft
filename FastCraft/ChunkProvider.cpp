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
#include "FCException.h"
#include <Poco/DeflatingStream.h>
#include "FCException.h"
#include "Structs.h"
#include "NetworkOutRoot.h"
#include "NetworkOut.h"
#include "PlayerThread.h"
#include "ChunkMath.h"
#include "World.h"
#include "Constants.h"
#include "MinecraftServer.h"
#include <Poco\DeflatingStream.h>
#include <sstream>
#include "MapChunk.h"

using std::cout;
using std::endl;
using std::memcpy;
using std::stringstream;
using Poco::DeflatingOutputStream;

char ChunkProvider::_sDespawnCode[] = {0x1, //ground up continuous
	0x0,0x0, //primary bit field  
	0x0,0x0, //add bit field
	0x0,0x0,0x0,0xc,  //data length
	0x78,0x9C,0x63,0x64,0x1C,0xD9,0x00,0x00,0x81,0x80,0x01,0x01};

ChunkProvider::ChunkProvider(NetworkOutRoot& rNetwork,
							 PlayerThread* pPlayer,
							 MinecraftServer* pServer
							 ) :
_rNetwork(rNetwork),
	ServerThreadBase("ChunkProvider")
{
	_pPlayer = pPlayer;
	_pWorld = NULL;
	_pMCServer = pServer;
	_fHasToClear = false;
	startThread(this);
}

ChunkProvider::~ChunkProvider() {
	killThread();
}

void ChunkProvider::HandleNewPlayer() {
	_pWorld = _pPlayer->getWorld();
}

void ChunkProvider::HandleDisconnect() {
	_fHasToClear = true;
}

void ChunkProvider::run() {
	_iThreadStatus = FC_THREADSTATUS_RUNNING;

	while (_iThreadStatus == FC_THREADSTATUS_RUNNING) {
		Poco::Thread::sleep(10);
		if (_fHasToClear) {
			_vSpawnedChunks.clear();
			_vToBeSendChunks.clear();
			_fHasToClear = false;
		}
		if (!_pPlayer->isSpawned()) {continue;}

		_PlayerCoordinates = ChunkMath::toChunkCoords(_pPlayer->getCoordinates());

		try {
			if ((_vSpawnedChunks.empty() && _vToBeSendChunks.empty()) ||
				(_oldPlayerCoordinates.X != _PlayerCoordinates.X ||  _oldPlayerCoordinates.Z != _PlayerCoordinates.Z)) 
			{
				_oldPlayerCoordinates = _PlayerCoordinates;
				CheckChunkList(_vSpawnedChunks); //Check spawned chunks and despawn if too distant
				CheckChunkCircle(); //Spawn new chunks
			}

			NextChunks();
		}catch(FCException) {
			continue;
		}

	}

	_iThreadStatus = FC_THREADSTATUS_DEAD;
}

bool ChunkProvider::isSpawned(ChunkCoordinates& coord) {
	if(_vSpawnedChunks.empty()) {return false;}
	for ( int x=0;x<=_vSpawnedChunks.size()-1;x++) {
		if (_vSpawnedChunks[x].X == coord.X && _vSpawnedChunks[x].Z == coord.Z) {
			return true;
		}
	}
	return false;
}

void ChunkProvider::CheckChunkList(vector<ChunkCoordinates>& rVec,bool fShouldDespawn) {
	if(rVec.empty()) {return;}
	bool fDespawn=false; 
	int iViewDistance = FC_VIEWDISTANCE,x;

	for (x=rVec.size()-1;x>=0;x--) {
		if (rVec[x].X < _PlayerCoordinates.X - iViewDistance) {fDespawn=true;}
		if (rVec[x].X > _PlayerCoordinates.X + iViewDistance) {fDespawn=true;}
		if (rVec[x].Z < _PlayerCoordinates.Z - iViewDistance) {fDespawn=true;}
		if (rVec[x].Z > _PlayerCoordinates.Z + iViewDistance) {fDespawn=true;}

		if(fDespawn) {
			try {
				if (fShouldDespawn) {despawnChunk(rVec[x]);}
			}catch(FCException& ex) {
				ex.rethrow();
			}
			rVec.erase(rVec.begin()+x);
		}

		fDespawn = false;
	}
}



void ChunkProvider::despawnChunk(ChunkCoordinates& coords) {
	try {
		NetworkOut Out(_pPlayer->getNetworkOutRoot());
		Out.addByte(0x33);
		Out.addInt(coords.X);
		Out.addInt(coords.Z);
		Out.getStr().append(_sDespawnCode,sizeof(_sDespawnCode));
		Out.Finalize(FC_QUEUE_LOW);		
	}catch(FCException& ex) {
		ex.rethrow();
	}
}


void ChunkProvider::CheckChunkCircle() {
	ChunkCoordinates CircleRoot,Temp;
	char iViewDistance = char(FC_VIEWDISTANCE);


	CheckChunkList(_vToBeSendChunks,false); //Check already queued chunks
	if (!isSpawned(_PlayerCoordinates)) {addSpawnJob(_PlayerCoordinates.X,_PlayerCoordinates.Z);} //Chunk who player stands on
	int X;
	int Z;

	for ( int iCircle = 1;iCircle<=iViewDistance;iCircle++) {
		X = CircleRoot.X = _PlayerCoordinates.X + iCircle;
		Z = CircleRoot.Z = _PlayerCoordinates.Z + iCircle;

		int iStep = 1;

		while(1) {
			Temp.X = X;
			Temp.Z = Z;

			if (!isSpawned(Temp)) {addSpawnJob(X,Z);}

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
}


void ChunkProvider::addSpawnJob(int X,int Z) {
	ChunkCoordinates Coord;
	Coord.X = X;
	Coord.Z = Z;

	_vToBeSendChunks.push_back(Coord);
}

void ChunkProvider::NextChunks() {
	if (_vToBeSendChunks.empty()) {return;}
	if(_pPlayer->getChunksInQueue() > 20) { return; }

	try {
		/*Poco::Stopwatch time;
		time.start();*/

		short iSendQueue = _vToBeSendChunks.size() > 5 ? 5 : _vToBeSendChunks.size();
		std::stringstream _stringStrm;
		Poco::DeflatingOutputStream _deflatingStrm(_stringStrm,Poco::DeflatingStreamBuf::STREAM_ZLIB,-1);
		MapChunk* pChk;
		NetworkOut Out(_pPlayer->getNetworkOutRoot());
		vector<pair<short,ChunkCoordinates>> vMetadata;


		for(;iSendQueue>0;iSendQueue--) {
			ChunkCoordinates ChkCoords = _vToBeSendChunks.front();
			pChk = _pWorld->getChunk(ChkCoords.X,ChkCoords.Z);

			_vToBeSendChunks.erase(_vToBeSendChunks.begin());
			_vSpawnedChunks.push_back(ChkCoords);

			if (pChk->isEmpty()) {continue;}

			vMetadata.push_back(std::make_pair(pChk->packData(_deflatingStrm),ChkCoords));
		}

		if (vMetadata.empty()) {return;}
		_deflatingStrm.flush();

		short iTransferredChunks=0;

		Out.addByte(0x38);
		Out.addShort((iTransferredChunks=vMetadata.size()));
		Out.addInt(_stringStrm.str().length());
		Out.getStr().append(_stringStrm.str());

		while(!vMetadata.empty()) {
			pair<short,ChunkCoordinates>& rJob = vMetadata.front();

			Out.addInt(rJob.second.X);
			Out.addInt(rJob.second.Z);
			Out.addShort(rJob.first);
			Out.addShort(0);

			vMetadata.erase(vMetadata.begin());
		}
		Out.Finalize(FC_QUEUE_LOW);

	/*	time.stop();
		cout<<"transferred "<<iTransferredChunks<<" chunks in "<<(time.elapsed()/1000)<<"ms\n";*/
	}catch(FCException& ex) {
		ex.rethrow();
	}
}


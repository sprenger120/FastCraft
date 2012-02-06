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

#include "PlayerPool.h"
#include "PlayerThread.h"
#include "SettingsHandler.h"
#include "PackingThread.h"
#include "EntityFlags.h"
#include "MathHelper.h"
#include "PlayerInventory.h"
#include "EntityCoordinates.h"
#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include <iostream>
#include <cmath>
#include <Poco/String.h>
#include "ChunkMath.h"
#include "WorldStorage.h"
#include "PlayerEventBase.h"
#include "PlayerEvents.h"

using Poco::Thread;
using std::cout;

bool PlayerPool::_fReady = false;

PlayerPool::PlayerPool(PackingThread& rPackingThread):
_vPlayerThreads(0),
	_ThreadPool("PlayerThreads",1,SettingsHandler::getPlayerSlotCount()),
	_qEvents(),
	_PackingThread(rPackingThread) 
{	
	int iSlotCount = SettingsHandler::getPlayerSlotCount();

	_vPlayerThreads.resize(iSlotCount); //Resize to slot count
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = NULL;
	}
	_fRunning=false;
}

PlayerPool::~PlayerPool() {
	if (_fRunning) {shutdown();}
}


void PlayerPool::run() {
	//Create Player Threads
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = new PlayerThread(this,_PackingThread);

		_ThreadPool.defaultPool().start(*_vPlayerThreads[x]);
	}
	_fReady=true;

	_fRunning=true;
	PlayerEventBase* p;

	while (_fRunning) {
		while(_qEvents.empty()) {
			Thread::sleep(50);
		}
		try {
			p = _qEvents.front();
			p->Execute(_vPlayerThreads,this);
			delete p;
			_qEvents.pop();
		} catch(Poco::RuntimeException& ex) {
			cout<<"PlayerPool::run exception:"<<ex.message()<<"\n";
			_qEvents.pop();
		}
	}
}


bool PlayerPool::isAnySlotFree() {
	if (getFreeSlot() == -1) {
		return false;
	}else{
		return true;
	}
}

void PlayerPool::Assign(Poco::Net::StreamSocket& StrmSocket) {
	int iSlot;

	iSlot = getFreeSlot();
	if (iSlot == -1) {return;}

	_vPlayerThreads[iSlot]->Connect(StrmSocket);
}


int PlayerPool::getFreeSlot() {
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) { //Release objects
		if (_vPlayerThreads[x] == NULL) {continue;}
		if (!_vPlayerThreads[x]->isAssigned()) {
			return x;
		}
	}
	return -1;
}

void PlayerPool::addEvent(PlayerEventBase* pEvent) {
	_qEvents.push(pEvent);
}

void PlayerPool::sendMessageToAll(string str) {
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
			_vPlayerThreads[x]->insertChat(str);
		}
	}
}

vector<string> PlayerPool::ListPlayers(int iMax) {
	int iElements = 0;
	vector<string> vNames(0);

	if (iMax <= 0) {
		return vNames;
	}

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
			vNames.push_back(_vPlayerThreads[x]->getUsername());
			iElements++;
			if (iElements == iMax) {
				break;
			}
		}
	}

	return vNames;
}

EntityPlayer PlayerPool::buildEntityPlayerFromPlayerPtr(PlayerThread* pPlayer) {
	EntityPlayer Player;

	Player._Coordinates = pPlayer->getCoordinates();
	Player._sName = pPlayer->getUsername();
	Player._Flags = pPlayer->getFlags();

	PlayerInventory & Inventory =  pPlayer->getInventory();

	Player._aHeldItems[0] = Inventory.getSlot(36 + Inventory.getSlotSelection());
	Player._aHeldItems[1] = Inventory.getSlot(8);
	Player._aHeldItems[2] = Inventory.getSlot(7);
	Player._aHeldItems[3] = Inventory.getSlot(6);
	Player._aHeldItems[4] = Inventory.getSlot(5);
	return Player;
}


PlayerThread* PlayerPool::getPlayerByName(string Name,PlayerThread* pCaller) {
	if (_vPlayerThreads.empty()) { return NULL; }
	
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if (_vPlayerThreads[x] == pCaller) { continue; }
		if (Poco::icompare(_vPlayerThreads[x]->getUsername(),Name) == 0) {
			return _vPlayerThreads[x];
		}
	}

	return NULL;
}


bool PlayerPool::willHurtOther(BlockCoordinates blockCoord,PlayerThread* pPlayer) {
	if (_vPlayerThreads.empty()) {return false;}

	BlockCoordinates playerCoord;

	for ( int x = 0;x<=_vPlayerThreads.size()-1;x++){
		if ((!_vPlayerThreads[x]->isAssigned()) || (!_vPlayerThreads[x]->isSpawned()) || _vPlayerThreads[x] == pPlayer) {continue;}

		playerCoord = ChunkMath::toBlockCoords(_vPlayerThreads[x]->getCoordinates());

		if ((playerCoord.X == blockCoord.X && playerCoord.Z == blockCoord.Z) && (playerCoord.Y == blockCoord.Y ||  playerCoord.Y+1 == blockCoord.Y)) {
			return true;
		}else{
			return false;
		}
	}
	return false;
}

void PlayerPool::shutdown() {
	if (!_fRunning) {return;}
	_fRunning=false;
	while(!_fRunning){ //Wait till _fRunning turns true
	}
	_fRunning=false;

	cout<<"\tShutting down PlayerThreads\n";
	
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) { //Release objects
		if (_vPlayerThreads[x] != NULL) {
			
			cout<<"\t\t#:"<<x<<"...";
			_vPlayerThreads[x]->shutdown();
			delete _vPlayerThreads[x];;
			cout<<"Done"<<"\n";
		}
	}

	cout<<"\tDone\n"<<std::flush;
}

bool PlayerPool::isReady() {
	return _fReady;
}
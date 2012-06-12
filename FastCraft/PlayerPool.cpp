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
#include "PackingThread.h"
#include "EntityFlags.h"
#include "MathHelper.h"
#include "PlayerInventory.h"
#include "EntityCoordinates.h"
#include <Poco/Thread.h>
#include "FCRuntimeException.h"
#include <iostream>
#include <cmath>
#include <Poco/String.h>
#include "ChunkMath.h"
#include "PlayerEventBase.h"
#include "PlayerEvents.h"
#include "MinecraftServer.h"

using Poco::Thread;
using std::cout;


PlayerPool::PlayerPool(MinecraftServer* pServer) :
_vPlayerThreads(pServer->getPlayerSlotCount()),
	ServerThreadBase("PlayerPool"),
	_qEvents(),
	_PackingThread()
{	
	_pMinecraftServer = pServer;

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = new PlayerThread(_PackingThread,pServer);
	}	
	startThread(this);
}

PlayerPool::~PlayerPool() {
	killThread();

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		delete _vPlayerThreads[x];
	}
	
	
	while (!_qEvents.empty()) {
		delete _qEvents.front();
		_qEvents.pop();
	}
}


void PlayerPool::run() {
	_iThreadStatus = FC_THREADSTATUS_RUNNING;
	PlayerEventBase* p;

	while (_iThreadStatus==FC_THREADSTATUS_RUNNING) {		
		if (_qEvents.empty()) {
			Thread::sleep(50);
			continue;
		}

		try {
			p = _qEvents.front();
			p->Execute(_vPlayerThreads,this);
			delete p;
			_qEvents.pop();
		} catch(FCRuntimeException& ex) {
			cout<<"PlayerPool::run exception:"<<ex.getMessage()<<"\n";
			_qEvents.pop();
		}
	}
	_iThreadStatus = FC_THREADSTATUS_DEAD;
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
	while(!_vPlayerThreads[iSlot]->isAssigned()) {Thread::sleep(10);} //Wait till assigned flag is set
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

vector<PlayerThread*>&  PlayerPool::ListPlayers() {
	return _vPlayerThreads;
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

short PlayerPool::getConnectedPlayerCount() {
	if (_vPlayerThreads.empty()) {return 0;}
	short count = 0;
	for (short x=0;x<=_vPlayerThreads.size()-1;x++) {
		if (_vPlayerThreads[x]->isSpawned()) {count++;}
	}
	return count;
}
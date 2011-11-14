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

#include "PlayerPool.h"
#include "PlayerThread.h"
#include "SettingsHandler.h"


#include <Poco/Thread.h>

PlayerPool::PlayerPool(SettingsHandler* pSettingsHandler):
_vPlayerThreads(0),
_ThreadPool("PlayerThreads",1,pSettingsHandler->getMaxClients()),
_EntityProvider(),
_ServerTime()
{
	int iSlotCount = pSettingsHandler->getMaxClients();

	_vPlayerThreads.resize(iSlotCount); //Resize to slot count

	//Create Threads
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = new PlayerThread(pSettingsHandler,&_EntityProvider,&_ServerTime,this);

		_ThreadPool.defaultPool().start(*_vPlayerThreads[x]);
	}
}

PlayerPool::~PlayerPool() {
	_ThreadPool.stopAll(); //Stop all threads

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) { //Release objects
		delete _vPlayerThreads[x];
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
		if (!_vPlayerThreads[x]->isAssigned()) {
		return x;
		}
	}
	return -1;
}


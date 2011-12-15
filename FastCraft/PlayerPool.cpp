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
#include "PackingThread.h"

#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include <iostream>

using Poco::Thread;
using std::cout;

PlayerPool::PlayerPool(PackingThread& rPackingThread):
_vPlayerThreads(0),
	_ThreadPool("PlayerThreads",1,SettingsHandler::getPlayerSlotCount()),
	_qEventQueue(),
	_ChunkRoot(),
	_PackingThread(rPackingThread) 
{	
	try {
		_ChunkRoot.generateMap(0,0,20,20);
	} catch (Poco::RuntimeException) {
		throw Poco::RuntimeException("Chunk generation failed");
	}

	int iSlotCount = SettingsHandler::getPlayerSlotCount();

	_vPlayerThreads.resize(iSlotCount); //Resize to slot count

	//Create Threads
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = new PlayerThread(this,_ChunkRoot,_PackingThread);

		_ThreadPool.defaultPool().start(*_vPlayerThreads[x]);
	}
}

PlayerPool::~PlayerPool() {
	_ThreadPool.stopAll(); //Stop all threads

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) { //Release objects
		delete _vPlayerThreads[x];
	}
}


void PlayerPool::run() {
	std::string sData;
	PlayerPoolEvent Event;

	while (1) {
		if (_qEventQueue.empty()) {
			Thread::sleep(50);
			continue;
		}

		Event = _qEventQueue.front();
		_qEventQueue.pop();


		switch (Event.Job) {
		case FC_PPEVENT_CHAT:
			cout<<"Chat: "<<Event.Message<<endl; //Server console
			sendMessageToAll(Event.Message);
			break;
		case FC_PPEVENT_MOVE:
			cout<<"PP: move event"<<"\n";
			break;
		case FC_PPEVENT_JOIN:
			//Update PlayerLists
			for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
				if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
					_vPlayerThreads[x]->PlayerInfoList(true,Event.pThread->getUsername());
				}
			}
			break;
		case FC_PPEVENT_DISCONNECT:
			for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
				if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
					_vPlayerThreads[x]->PlayerInfoList(false,Event.pThread->getUsername());
				}
			}
			break;
		case FC_PPEVENT_ANIMATION:
			cout<<"PP: animation event"<<"\n";
			break;
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
		if (!_vPlayerThreads[x]->isAssigned()) {
			return x;
		}
	}
	return -1;
}

void PlayerPool::Event(PlayerPoolEvent& rEvent) {
	_qEventQueue.push(rEvent);
}

void PlayerPool::sendMessageToAll(string& rString) {
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
			_vPlayerThreads[x]->insertChat(rString);
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

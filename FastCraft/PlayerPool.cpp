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
#include "ChunkRoot.h"
#include "Structs.h"
#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include <iostream>

using Poco::Thread;
using std::cout;

PlayerPool::PlayerPool(SettingsHandler* pSettingsHandler):
_vPlayerThreads(0),
_ThreadPool("PlayerThreads",1,pSettingsHandler->getMaxClients()),
_EntityProvider(),
_ServerTime(),
_qChat(),
_PackingThread()
{
	//Create ChunkRoot obj
	try {
		_pChunkRoot = new ChunkRoot;
		_pChunkRoot->generateMap(0,0,20,20);
	} catch (Poco::RuntimeException) {
		throw Poco::RuntimeException("Chunk generation failed");
	}

	int iSlotCount = pSettingsHandler->getMaxClients();

	_vPlayerThreads.resize(iSlotCount); //Resize to slot count

	//Create Threads
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = new PlayerThread(pSettingsHandler,&_EntityProvider,&_ServerTime,this,_pChunkRoot,&_PackingThread);

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
	//Create Servertime thread
	Poco::Thread threadServerTime;
	ServerTime ServerTime;
	threadServerTime.start(ServerTime);

	//Create Packing Thread
	Poco::Thread threadPackingThread;
	threadPackingThread.start(_PackingThread);

	ChatEntry ChatEntry;
	std::string sData;

	while (1) {
		if (_qChat.size() == 0) {
			Thread::sleep(100);
			continue;
		}

		ChatEntry = _qChat.front();
		_qChat.pop();
		
		sendMessageToAll(ChatEntry.Message);
		cout<<"Chat: "<<ChatEntry.Message<<endl; //Server console
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

void PlayerPool::Chat(string String,PlayerThread* pPlayer,bool fAppendName) {
	ChatEntry Entry;

	Entry.Message.clear();

	if (fAppendName) {
		Entry.Message.assign("<");
		Entry.Message.append( pPlayer->getUsername());
		Entry.Message.append("> ");
	}
	Entry.Message.append(String);

	Entry.X = int(pPlayer->getCoordinates().X);
	Entry.Z = int(pPlayer->getCoordinates().Z);

	_qChat.push(Entry);
}

void PlayerPool::sendMessageToAll(string& rString) {
	QueueJob Job;

	Job.Data.clear();
	Job.Data.append<char>(1,0x3);

	NetworkIO::packString(Job.Data,rString);
	Job.Special = FC_JOB_NO;
		
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
			if( _vPlayerThreads[x]->isAssigned()) {
				if ( _vPlayerThreads[x]->isSpawned()) {
					_vPlayerThreads[x]->appendQueue(Job);
				}
			}

	}
}
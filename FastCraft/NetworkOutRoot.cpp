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
#include "NetworkOutRoot.h"
#include "Constants.h"
#include <iostream>
#include "FCRuntimeException.h"
#include "MinecraftServer.h"
#include "PlayerThread.h"
#include <cstring>
#include <utility>
using namespace CryptoPP;
using std::pair;

NetworkOutRoot::NetworkOutRoot(MinecraftServer* pMCServer,PlayerThread* pPlayer) {
	_pMCServer = pMCServer;
	_pPlayer = pPlayer;
}

//NetworkOutRoot::~NetworkOutRoot() {
//}

ThreadSafeQueue<string*> & NetworkOutRoot::getLowQueue() {
	return  _lowQueue;
}

ThreadSafeQueue<string*> & NetworkOutRoot::getHighQueue() {
	return _highQueue;
}

void NetworkOutRoot::Add(char iType,string* pData) {
	_pMCServer->_iWriteTraffic+=pData->length();

	switch(iType) {
	case FC_QUEUE_LOW:
		_lowQueue.push(pData);
		break;
	case FC_QUEUE_HIGH:
		_highQueue.push(pData);
		break;
	default:
		throw FCRuntimeException("Unknown queue type");
	}
}


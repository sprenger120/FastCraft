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
#include "AcceptThread.h" 
#include "NetworkOut.h"
#include "SettingsHandler.h"
#include "PlayerPool.h"
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>

using Poco::Net::StreamSocket;
using std::cout;
using Poco::Thread;

AcceptThread::AcceptThread(PlayerPool& rPlayerPool):
	_rPlayerPool(rPlayerPool),
	_ServerFullMsg("")
{
	_ServerFullMsg.append<unsigned char>(1,0xFF);
	NetworkOut::addString(_ServerFullMsg,"Server full!");
}

AcceptThread::~AcceptThread() {
}

void AcceptThread::run() {
	Poco::Net::ServerSocket ServerSock(SettingsHandler::getPort());
	Poco::Net::StreamSocket StrmSock;
	ServerSock.setBlocking(true);

	while(1) {
		ServerSock.listen(); //Wait
		StrmSock = ServerSock.acceptConnection(); //Ooh a new player

		if (!_rPlayerPool.isAnySlotFree()) { //There is no free slot
			StrmSock.sendBytes(_ServerFullMsg.c_str(),_ServerFullMsg.length());
			Thread::sleep(100); //Wait a moment 
			StrmSock.close();
			continue;
		}

		_rPlayerPool.Assign(StrmSock);
	}
}
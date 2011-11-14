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
#include "NetworkHandler.h" 
#include "SettingsHandler.h"
#include "TextHandler.h"
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>

using Poco::Net::StreamSocket;
using std::cout;
using Poco::Thread;

NetworkHandler::NetworkHandler(SettingsHandler* Settings):
_PlayerPool(Settings),
	_ServerFullMsg("Server full!")
{
	_iPort =  Settings->getPort();

	//Prepare Kick message
	string sTemp("");

	sTemp.append<unsigned char>(1,0xFF);
	TextHandler::packString16(sTemp,_ServerFullMsg);

	_ServerFullMsg.assign(sTemp);
}

NetworkHandler::~NetworkHandler() {
}

void NetworkHandler::run() {
	Poco::Net::ServerSocket ServerSock(_iPort);
	Poco::Net::StreamSocket StrmSock;

	ServerSock.setBlocking(true);


	while(1) {
		ServerSock.listen(); //Wait
		StrmSock = ServerSock.acceptConnection(); //Ooh a new player

		if (!_PlayerPool.isAnySlotFree()) { //There is no free slot
			StrmSock.sendBytes(_ServerFullMsg.c_str(),_ServerFullMsg.length());
			Thread::sleep(100); //Wait a moment 
			StrmSock.close();
			continue;
		}

		_PlayerPool.Assign(StrmSock);
	}
}
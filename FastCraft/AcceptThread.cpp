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
#include "AcceptThread.h" 
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/NetException.h>
#include <Poco/Thread.h>
#include "MinecraftServer.h"
#include "NetworkOut.h"
#include "PlayerPool.h"

using Poco::Net::StreamSocket;
using std::cout;
using Poco::Thread;


AcceptThread::AcceptThread(MinecraftServer* pServer) 
try :
ServerThreadBase("AcceptThread"),
_ServerSock(pServer->getPort()),
_preparedServerFullMsg("")
{
	_pMinecraftServer = pServer;
	_preparedServerFullMsg.append<unsigned char>(1,0xFF);
	NetworkOut::addString(_preparedServerFullMsg,pServer->getServerFullMessage());

	startThread(this);
}catch(Poco::IOException& ex) {
	cout<<"Unable to bind 0.0.0.0:"<<pServer->getPort()<<" ("<<ex.message()<<")\n"
		<<"Is there another Minecraft server, running on this port?\n";
	throw Poco::RuntimeException("Unable to start AcceptThread");
}


AcceptThread::~AcceptThread() {
	_ServerSock.close();
	while(_iThreadStatus == FC_THREADSTATUS_RUNNING) {
		Thread::sleep(100);
	}
}

void AcceptThread::run() {
	Poco::Net::StreamSocket StrmSock;

	_iThreadStatus = FC_THREADSTATUS_RUNNING;
	while(_iThreadStatus==FC_THREADSTATUS_RUNNING) {
		try {
			_ServerSock.listen();
			StrmSock = _ServerSock.acceptConnection(); //Ooh a new player


			if (!_pMinecraftServer->getPlayerPool()->isAnySlotFree()) { //There is no free slot
				StrmSock.sendBytes(_preparedServerFullMsg.c_str(),_preparedServerFullMsg.length());
				Thread::sleep(100); //Wait a moment 
				StrmSock.close();
				continue;
			}

			_pMinecraftServer->getPlayerPool()->Assign(StrmSock);
		}catch(...) {       /* Only happen if socket become closed */
			_iThreadStatus = FC_THREADSTATUS_TERMINATING;
			return;
		}
	}
}

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
#include <Poco/Net/ServerSocket.h>
#include <Poco/ThreadPool.h>


NetworkHandler::NetworkHandler(SettingsHandler* Settings):
	_pSettings(Settings),
	_sIP(""),
	_fReady(false),
	_iMaxClients(Settings->getMaxClients())
{
	Poco::Net::StreamSocket fakeStrm;

	//Create player threads
	_aPlayers = new PlayerThread[_iMaxClients];
	Poco::ThreadPool PlayerThreadPool(1,_iMaxClients);

	for (int x=0;x<=_iMaxClients-1;x++) {
		PlayerThreadPool.defaultPool().start(_aPlayers[x]);
		
		//Wait for thread become ready
		while (! _aPlayers[x].Ready() ) {}
		
		//give a settingshandler object pointer to thread
		_aPlayers[x].setSettingsHandler(Settings);
	}

}

NetworkHandler::~NetworkHandler() {
	delete [] _aPlayers; //Delete objects
}

bool NetworkHandler::Ready() {
	return _fReady;
}

void NetworkHandler::run() {
	int x;
	bool _fFull=true;

	Poco::Net::ServerSocket ServerSock(_pSettings->getPort());
	Poco::Net::StreamSocket StrmSock;

	ServerSock.setBlocking(true);
		
	_fReady=true;	

	while(1) {
		_fFull = true;

		ServerSock.listen(); //Wait
		StrmSock = ServerSock.acceptConnection(); //Ooh a new player
		_sIP = StrmSock.peerAddress().toString(); //get ip
		
		for (x=0;x<=_pSettings->getMaxClients()-1;x++) { //search free thread
			
			if (! _aPlayers[x].isAssigned() ) { 
				_aPlayers[x].Connect(StrmSock,_sIP); //Connect
				_fFull = false;
				break;
			}

		}
		
		if (_fFull) {
			cout<<"server full!"<<endl;
			StrmSock.close();
		}
	}

}
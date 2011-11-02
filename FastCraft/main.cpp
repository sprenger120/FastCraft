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

#include <iostream>
#include <string>

#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/NumberFormatter.h>

#include "NetworkHandler.h"
#include "SettingsHandler.h"
#include "PlayerThread.h"

using std::cout;
using std::string;
using std::endl;
using Poco::Thread;

int main() {
	int x;
	SettingsHandler Settings;

	//Informations
	cout<<"--- FAST CRAFT v. "<<Settings.getFastCraftVersion()<<" for Minecraft "<<Settings.getSupportedMCVersion()<<" ---"<<"\n";
	cout<<"Running on *:"<<Settings.getPort()<<" with "<<Settings.getMaxClients()<<" player slots"<<endl;

	//Player Threads
	PlayerThread* aPlayers = new PlayerThread[Settings.getMaxClients()];
	Poco::ThreadPool PlayerThreadPool(1,Settings.getMaxClients());

	for (x=0;x<=Settings.getMaxClients()-1;x++) {
		PlayerThreadPool.defaultPool().start(aPlayers[x]);
		
		//Wait for thread become ready
		while (! aPlayers[x].Ready() ) {}
	}


	//Start Network Thread
	NetworkHandler NetworkHandler(aPlayers,&Settings);
	Thread threadNetworkHandler("NetworkHandler");

	threadNetworkHandler.start(NetworkHandler);

	while(! NetworkHandler.Ready()) {}

	cout<<"Ready"<<endl;




	//Release Player Objects
	delete [] aPlayers;	

	return 1;
}
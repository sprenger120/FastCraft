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

#include "NetworkHandler.h"
#include "SettingsHandler.h"
#include "PlayerThread.h"
#include "EntityProvider.h"

using std::cout;
using std::string;
using std::endl;
using Poco::Thread;

int main() {
	SettingsHandler Settings;

	//Informations
	cout<<"--- FAST CRAFT v. "<<Settings.getFastCraftVersion()<<" for Minecraft "<<Settings.getSupportedMCVersion()<<" ---"<<"\n";
	cout<<"Running on *:"<<Settings.getPort()<<" with "<<Settings.getMaxClients()<<" player slots"<<endl;

	EntityProvider EntityProvider;


	//Start Network Thread
	NetworkHandler NetworkHandler(&Settings,&EntityProvider);
	Thread threadNetworkHandler("NetworkHandler");
	threadNetworkHandler.start(NetworkHandler);
	while(! NetworkHandler.Ready()) {}


	cout<<"Ready."<<"\n"<<endl;



	while (1) {
		Thread::sleep(100);
	}


	return 1;
}
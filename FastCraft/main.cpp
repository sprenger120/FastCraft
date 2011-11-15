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
#include <Poco/Exception.h>
#include "NetworkHandler.h"
#include "SettingsHandler.h"
#include "PlayerThread.h"
#include "EntityProvider.h"

using std::cout;
using std::string;
using std::endl;
using Poco::Thread;

int main() {
	SettingsHandler* pSettingHandler;

	cout<<"--- FAST CRAFT v. "<<SettingsHandler::getFastCraftVersion()<<" for Minecraft "<<SettingsHandler::getSupportedMCVersion()<<" ---"<<"\n";
	
	try {
		pSettingHandler = new SettingsHandler; //Load configuration
	} catch (Poco::RuntimeException) {
		Thread::sleep(3000);
		return 1;
	}
	cout<<"Running on *:"<<pSettingHandler->getPort()<<" with "<<pSettingHandler->getMaxClients()<<" player slots"<<endl;


	//Start Network Thread
	NetworkHandler NetworkHandler(pSettingHandler);
	Thread threadNetworkHandler("NetworkHandler");
	threadNetworkHandler.start(NetworkHandler);


	cout<<"Ready."<<"\n"<<endl;


	while (1) {
		Thread::sleep(100);
	}

	delete pSettingHandler;

	return 1;
}
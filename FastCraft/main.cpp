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
#define BUILD_VERSION ""
#include <iostream>
#include <sstream>
#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include "AcceptThread.h"
#include "SettingsHandler.h"
#include "PlayerPool.h"
#include "PackingThread.h"
#include "ServerTime.h"
#include "ItemInfoStorage.h"
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Data/SQLite/Connector.h>

using std::cout;
using std::stringstream;
using Poco::Thread;

int main(int argc, char *argv[]) {
	string BuildVer(BUILD_VERSION);
	Poco::Data::SQLite::Connector::registerConnector();

	cout<<"FastCraft Minecraft Server"<<"\n";
	cout<<"Version "<<BUILD_VERSION<<(BuildVer.compare("") == 0 ? "" : "-")<<SettingsHandler::getFastCraftVersion()<<" for Minecraft "<<SettingsHandler::getSupportedMCVersion()<<"\n"<<std::endl;

	/* 
	* Creating files / directorys
	*/
	Poco::Path FCPath(argv[0]); 
	FCPath.setFileName(""); //Remove filename

	//Main directory
	FCPath.pushDirectory("Settings");
	Poco::File settingsDirectory(FCPath.toString());

	//Item databases
	FCPath.pushDirectory("ItemDatabases");
	Poco::File itemDatabasesDirectory(FCPath.toString());

	if (!settingsDirectory.exists()) {
		cout<<"Settings directory not existing. Creating..."<<"\n";
		settingsDirectory.createDirectories();
	}

	if (!itemDatabasesDirectory.exists()) {
		cout<<"Item databases directory not existing. Creating..."<<"\n";
		itemDatabasesDirectory.createDirectories();
	}


	/*
	* Read item databases
	*/
	ItemInfoStorage::loadDatabases(FCPath);
	cout<<"\n";


	/*
	* Load settings
	*/
	cout<<"Loading settings... "<<"\n";
	try {
		SettingsHandler(); //Load configuration into static variables
	} catch (Poco::RuntimeException) {
		cout<<"Error while loading settings"<<"\n";
		Thread::sleep(3000);
		return 1;
	}
	cout<<"Done."<<"\n\n";
	

	/*
	* Create threads 
	*/

	//Threads
	Thread threadPackingThread("Chunk Packer");
	Thread threadPlayerPool("PlayerPool");
	Thread threadAcceptThread("Network Acceptor");


	//Create ChunkPacker Thread
	PackingThread Packer;
	threadPackingThread.start(Packer);


	//Create player pool thread
	PlayerPool PPool(Packer);
	threadPlayerPool.start(PPool);


	//Acceptor thread
	AcceptThread Acceptor(PPool);
	threadAcceptThread.start(Acceptor);


	/*
	* All right :)
	*/

	cout<<"Running on *:"<<SettingsHandler::getPort()<<" with "<<SettingsHandler::getPlayerSlotCount()<<" player slots"<<std::endl;
	cout<<"Ready."<<"\n"<<std::endl;
	while (1) {		
		Thread::sleep(1000);
		ServerTime::tick();
	}

	Poco::Data::SQLite::Connector::unregisterConnector();
	return 1;
}
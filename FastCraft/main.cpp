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
#include "Structs.h"
#include "WorldStorage.h"
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
	FCPath.pushDirectory("Data");
	Poco::File settingsDirectory(FCPath.toString());
	if (!settingsDirectory.exists()) {settingsDirectory.createDirectories();}

	//Config
	cout<<"Loading settings... "<<"\n";
	
	FCPath.setFileName("settings.xml");
	SettingsHandler::readConfiguration(FCPath);
	FCPath.setFileName("");
	
	cout<<"Done."<<"\n\n";


	//Item databases
	FCPath.pushDirectory("ItemDatabases");
	Poco::File itemDatabasesDirectory(FCPath.toString());
	if (!itemDatabasesDirectory.exists()) {itemDatabasesDirectory.createDirectories();}
	
	ItemInfoStorage::loadDatabases(FCPath);
	cout<<"\n";
	
	
	//Worlds directory
	FCPath.popDirectory();
	FCPath.pushDirectory("Worlds");
	Poco::File worldDirectory(FCPath.toString());
	if (!worldDirectory.exists()) {worldDirectory.createDirectories();}


	/*
	* Create threads 
	*/

	//Creating Threads & Classes
	Thread threadPackingThread("Chunk Packer");
	Thread threadPlayerPool("PlayerPool");
	Thread threadAcceptThread("Network Acceptor");

	PackingThread Packer;
	PlayerPool PPool(Packer);	
	AcceptThread Acceptor(PPool);
	
	
	/*
	* Loading world
	*/
	cout<<"Loading maps..."<<"\n";
	WorldStorage worldStorage(PPool,FCPath);
	worldStorage.loadAllWorlds();
	cout<<"Done.\n\n";


	/*
	* Start Threads
	*/
	threadPackingThread.start(Packer);
	threadPlayerPool.start(PPool);
	threadAcceptThread.start(Acceptor);
	

	//Init Done!
	cout<<"Running on *:"<<SettingsHandler::getPort()<<" with "<<SettingsHandler::getPlayerSlotCount()<<" player slots"<<std::endl;
	cout<<"Ready."<<"\n"<<std::endl;

	while(1) {
		Thread::sleep(1000);
		ServerTime::tick();
	}

	Poco::Data::SQLite::Connector::unregisterConnector();
	return 1;
}
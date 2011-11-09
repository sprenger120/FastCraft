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

#include "SettingsHandler.h"
#include "Constants.h"
#include <string>
#include <Poco/Path.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/AutoPtr.h>
#include <Poco/Foundation.h>
#include <Poco/File.h>
#include <Poco/Exception.h>

using std::cout;
using std::endl;
using Poco::Path;
using Poco::AutoPtr;
using Poco::Util::PropertyFileConfiguration;

SettingsHandler::SettingsHandler():
_sServerDescription("")
{
	//Set standard configuration
	_iPort = 25565;
	_iMaxClients = 16;
	_sServerDescription.assign("FastCraft "+getFastCraftVersion()+" Server"); 
	_iServerMode = FC_SERVMODE_SURVIVAL;
	_iDifficulty = FC_DIFFICULTY_EASY;
	
	//Build path
	Poco::Path ConfPath(true);
	ConfPath.assign(Path::current());
	ConfPath.setFileName("fastcraft.properties");

	//Already existing? 
	Poco::File File(ConfPath);
	if (! File.exists()) { //New Configuration file
		cout<<"Configuration file not found. Generating new one."<<std::endl;
		
		try {
			File.createFile();
		} catch (Poco::FileAccessDeniedException) {
			cout<<"Access denied!"<<std::endl;
			throw Poco::RuntimeException("Access denied");
			return;
		}

		AutoPtr<PropertyFileConfiguration> pConf;
		pConf = new PropertyFileConfiguration(ConfPath.toString());

		pConf->setInt("ServerPort",_iPort);
		pConf->setInt("Slots",_iMaxClients);
		pConf->setInt("ServerMode",_iServerMode);
		pConf->setInt("Difficulty",_iDifficulty);
		pConf->setString("Description",_sServerDescription);
		
		pConf->save(ConfPath.toString());
	}else{ //Load configuration
		AutoPtr<PropertyFileConfiguration> pConf;
		pConf = new PropertyFileConfiguration(ConfPath.toString());
		

		_iPort = pConf->getInt("ServerPort");
		_iMaxClients = pConf->getInt("Slots");
		_iServerMode = pConf->getInt("ServerMode");
		_iDifficulty = pConf->getInt("Difficulty");
		_sServerDescription = pConf->getString("Description");

		//Check
		//port
		if (_iPort < 1024 || _iPort > 65535) {
			cout<<"Config ERROR: Invalid port, falling back to 25565!"<<endl;
			_iPort = 25565;
		}

		//slots
		if (_iMaxClients < 0 || _iMaxClients > 255) {
			cout<<"Config ERROR: Invalid slot count, falling back to 16!"<<endl;
			_iMaxClients = 16;
		}

		//Server mode
		if (_iServerMode != FC_SERVMODE_SURVIVAL && _iServerMode != FC_SERVMODE_CREATIVE) {
			cout<<"Config ERROR: Invalid server mode, falling back to survival!"<<endl;
			_iServerMode = FC_SERVMODE_SURVIVAL;
		}

		//difficulty
		if (_iDifficulty < 0 || _iDifficulty > 3) {
			cout<<"Config ERROR: Invalid difficulty, falling back to easy!"<<endl;
			_iDifficulty = FC_DIFFICULTY_EASY;
		}

		//server desc
		if (_sServerDescription.length() > 34) {
			cout<<"Config WARNING: Server description too long. Description will be shortened!"<<endl;
			_sServerDescription.resize(34);
		}
	}
	

	//MapInfo
	_iMapSeed = 0;
	_iWorldHeight = 128;
}

SettingsHandler::~SettingsHandler() {

}


short SettingsHandler::getPort() {
	return short(_iPort); 
}

int SettingsHandler::getMaxClients() {
	return _iMaxClients;
}

string SettingsHandler::getSupportedMCVersion() {
	return "1.8.1";
}

string SettingsHandler::getFastCraftVersion() {
	return "0.0.1";
}

int SettingsHandler::getSupportedProtocolVersion() {
	return 17; //Minecraft 1.8.1
}

string SettingsHandler::getServerDescription() {
	return _sServerDescription;
}

int SettingsHandler::getServerMode() {
	return _iServerMode;
}

char SettingsHandler::getDifficulty() {
	return _iDifficulty;
}

long long SettingsHandler::getMapSeed() {
	return _iMapSeed;
}

unsigned char SettingsHandler::getWorldHeight() {
	return _iWorldHeight;
}
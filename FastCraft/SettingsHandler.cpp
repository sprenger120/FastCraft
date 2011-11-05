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

SettingsHandler::SettingsHandler():
_sSupportedMCVersion(""),
_sFastcraftVersion(""),
_sServerDescription("")
{
	//Network
	_iPort = 25565;
	_iMaxClients = 16;
	
	//Version
	_sSupportedMCVersion.assign("1.8.1");
	_sFastcraftVersion.assign("0.0.1");
	_iSupportedProtocolVersion = 17;


	//Server Info
	_sServerDescription.assign("FastCraft "+_sFastcraftVersion+" Server"); 

	if (_sServerDescription.length() > 34) {
		_sServerDescription.resize(34);
	}

	_iServerMode = FC_SERVMODE_SURVIVAL;
	_iDifficulty = 1;

	//MapInfo
	_iMapSeed = 0;
	_iWorldHeight = 128;
}

SettingsHandler::~SettingsHandler() {

}


short SettingsHandler::getPort() {
	return _iPort; 
}

int SettingsHandler::getMaxClients() {
	return _iMaxClients;
}

string SettingsHandler::getSupportedMCVersion() {
	return _sSupportedMCVersion;
}

string SettingsHandler::getFastCraftVersion() {
	return _sFastcraftVersion;
}

int SettingsHandler::getSupportedProtocolVersion() {
	return _iSupportedProtocolVersion; //Minecraft 1.8.1
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
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

#include "WorldStorage.h"
#include "PlayerPool.h"
#include "World.h"
#include "SettingsHandler.h"
#include <Poco/String.h>
#include <Poco/Exception.h>

vector<World*> WorldStorage::_vpWorlds(0);

WorldStorage::WorldStorage(PlayerPool& r,Poco::Path path) : 
_rPlayerPool(r),
_WorldPath(path)
{ 
}

WorldStorage::~WorldStorage() {
	forceSave();

	if (!_vpWorlds.empty()) {
		for(int x=0;x<=_vpWorlds.size()-1;x++) {
			delete _vpWorlds[x];
		}
	}
}

void WorldStorage::loadWorld(string Name,char iDimension) {
	World* pWorld;
	Poco::Path pathWorld(_WorldPath);

	Poco::trimLeftInPlace(Name);
	if (Name.compare("") == 0) {
		throw Poco::RuntimeException("Illegal name");
	}
	
	pathWorld.setFileName("");
	pathWorld.pushDirectory(Name);

	pWorld = new World(Name,iDimension,_rPlayerPool);
	pWorld->Load(pathWorld);

	_vpWorlds.push_back(pWorld);
}

void WorldStorage::forceSave() {
	return;
}

void WorldStorage::loadAllWorlds() {
	//Load main map
	try {
		loadWorld(SettingsHandler::getMainWorldName(), 1);
	}catch(Poco::RuntimeException& ex) { 
		std::cout<<"Unable to load world:"<<SettingsHandler::getMainWorldName()<<"! ("<<ex.message()<<")\n";
	}
}

World* WorldStorage::getWorldByName(string sName) {
	if (_vpWorlds.empty()) {
		std::cout<<"WorldStorage::getWorldByName no worlds loaded"<<"\n";
		return NULL;
	}

	for (int x = 0; x<= _vpWorlds.size()-1;x++) {
		if ( Poco::icompare(_vpWorlds[x]->getName(),sName) == 0) {
			return _vpWorlds[x];
		}
	}
	std::cout<<"WorldStorage::getWorldByName "<<sName<<" not found!"<<"\n";
	return NULL;
}

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
#include "EntityListEntry.h"
#include "ItemSlot.h"
#include "FCException.h"

EntityListEntry::EntityListEntry(Entity* pEntity) {
	_iEntityID = pEntity->getEntityID();
	Position = pEntity->Coordinates;


	if (pEntity->isAlive()) {
		EntityLiving* pLiving = dynamic_cast<EntityLiving*>(pEntity);
		if (pLiving == NULL) {throw FCException("Unable to cast entity instance");}
		Equipment.resize(5);
		
		_iType = pLiving->getType();
		_fAlive = true;

		for (char x=0;x<=4;x++) {
			Equipment[x] = new ItemSlot(pEntity->getMinecraftServer()->getItemInfoProvider(),pLiving->getEquipment(x),1);
		}
	}else{
		_fAlive = false;
	}
}

EntityListEntry::~EntityListEntry(){
	if (!Equipment.empty()){
		for (char x=0;x<=4;x++) {
			if (Equipment[x] != NULL){delete Equipment[x];}
		}
	}
}


bool EntityListEntry::isAlive(){
	return _fAlive;
}

char EntityListEntry::getType() {
	if (!_fAlive) {throw FCException("Not alive");}
	return _iType;
}

int EntityListEntry::getEntiyID(){
	return _iEntityID;
}

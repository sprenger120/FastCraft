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
#include "EntityLiving.h"
#include "Constants.h"
#include "NetworkOut.h"
#include "ItemSlot.h"

using std::vector;

EntityLiving::EntityLiving(char iType,MinecraftServer* pServer,World* pWorld) try :
Entity			(pServer,pWorld),
	_vpHeld			(5)
{
	if (!Constants::isDefined(iType,"/Entity/Alive/TypeID")) {throw Poco::RuntimeException("Entity type not defined!");}

	_iHealth = 0;
	_iType = iType;

	for(char x=0;x<=4;x++) {_vpHeld[x] = NULL;}
}catch(Poco::RuntimeException& ex) {
	ex.rethrow();
}

EntityLiving::~EntityLiving() {
}

bool EntityLiving::isAlive() {
	return true;
}

float EntityLiving::getHeight() {
	return 0.0F;
}

float EntityLiving::getWidth() {
	return 0.0F;
}

string EntityLiving::getName() {
	return string("");
}

void EntityLiving::spawn(NetworkOut& rOut) {
	rOut.addByte(0x18);
	rOut.addInt(_iEntityID);
	rOut.addByte(_iType);
	rOut.addInt(int(floor(Coordinates.X * 32.0)));
	rOut.addInt(int(floor(Coordinates.Y * 32.0)));
	rOut.addInt(int(floor(Coordinates.Z * 32.0)));
	rOut.addByte(char((Coordinates.Yaw * 256.0F) / 360.0F));
	rOut.addByte(char((Coordinates.Pitch * 256.0F) / 360.0F));
	//rOut.addByte(char((Coordinates.HeadYaw * 256.0F) / 360.0F));
	appendMetadata(rOut);
	rOut.Finalize(FC_QUEUE_HIGH);

	sendEquipment(rOut);
}


void EntityLiving::appendMetadata(NetworkOut& rOut) { 
	rOut.addByte(127);
}

short EntityLiving::getHealth() {
	return _iHealth;
} 

short EntityLiving::getMaxHealth() {
	return 0;
}

void EntityLiving::sendEquipment(NetworkOut& rOut) {
	for (char x=0;x<=4;x++) {
		if (_vpHeld[x] == NULL) {continue;}
		rOut.addByte(0x5);
		rOut.addInt(_iEntityID);
		rOut.addShort(x);
		if(_vpHeld[x]->isEmpty()) { //empty slot
			rOut.addShort(-1);
			rOut.addShort(0);
		}else{
			rOut.addShort(_vpHeld[x]->getItem().first);
			rOut.addShort(_vpHeld[x]->getItem().second);
		}
		rOut.Finalize(FC_QUEUE_HIGH);
	}
}

void EntityLiving::updateEquipment(NetworkOut& rOut,EquipmentArray& rOldEquip) {
	if (rOldEquip.size() != 5) {throw Poco::RuntimeException("Illegal size");}

	for (char x=0;x<=4;x++) {
		if (_vpHeld[x] == NULL && rOldEquip[x] == NULL) {continue;}
		if (_vpHeld[x] == NULL && rOldEquip[x] != NULL) {
			rOut.addByte(0x5);
			rOut.addInt(_iEntityID);
			rOut.addShort(x);
			rOut.addShort(-1);
			rOut.addShort(0);
			rOut.Finalize(FC_QUEUE_HIGH);
			continue;
		}

		if ( (_vpHeld[x] != NULL && rOldEquip[x] == NULL) || (_vpHeld[x]->getItem().first != rOldEquip[x]->getItem().first)) {
			rOut.addByte(0x5);
			rOut.addInt(_iEntityID);
			rOut.addShort(x);
			if(_vpHeld[x]->isEmpty()) { //empty slot
				rOut.addShort(-1);
				rOut.addShort(0);
			}else{
				rOut.addShort(_vpHeld[x]->getItem().first);
				rOut.addShort(_vpHeld[x]->getItem().second);
			}
			rOut.Finalize(FC_QUEUE_HIGH);
			continue;
		}
	}
}


void EntityLiving::setEquipment(char index,ItemID id) {
	if (index < 0 || index > 4) {throw Poco::RuntimeException("Illegal index");}

	if (id.first == -1 && id.second == -1) {
		if (_vpHeld[index] == NULL) {return;}

		delete _vpHeld[index];
		_vpHeld[index] = NULL;

		return;
	}

	try {
		if (_vpHeld[index] == NULL) {
			_vpHeld[index] = new ItemSlot(_pMCServer->getItemInfoProvider(),id,1);
		}else{
			_vpHeld[index]->setItem(id);
			_vpHeld[index]->setStackSize(1);
		}
	}catch(Poco::RuntimeException & ex) {
		ex.rethrow();
	}
}

ItemID EntityLiving::getEquipment(char index) {
	if (index < 0 || index > 4) {throw Poco::RuntimeException("Illegal index");}
	return (_vpHeld[index] == NULL ? FC_EMPTYITEMID : _vpHeld[index]->getItem());
}
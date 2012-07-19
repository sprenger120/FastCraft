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
#include <cmath>
#include "FCRuntimeException.h"

using std::vector;

EntityLiving::EntityLiving(char iType,MinecraftServer* pServer,World* pWorld,bool fGrabNew) try :
Entity			(pServer,pWorld,fGrabNew),
	_vpHeld			(5)
{
	if (!Constants::isDefined(iType,"/Entity/Alive/TypeID")) {throw FCRuntimeException("Entity type not defined!");}

	_iHealth = 0;
	_iType = iType;

	for(char x=0;x<=4;x++) {_vpHeld[x] = new ItemSlot(pServer->getItemInfoProvider());}
}catch(FCRuntimeException& ex) {
	ex.rethrow();
}

EntityLiving::~EntityLiving() {
	for(char x=0;x<=4;x++) {delete _vpHeld[x];}
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

void EntityLiving::sendMetadata(NetworkOut& rOut) { 
	rOut.addByte(0x28);
	rOut.addInt(_iEntityID);
	appendMetadata(rOut);
	rOut.Finalize(FC_QUEUE_HIGH);
}

short EntityLiving::getHealth() {
	return _iHealth;
} 

short EntityLiving::getMaxHealth() {
	return 0;
}

void EntityLiving::sendEquipment(NetworkOut& rOut) {
	for (char x=0;x<=4;x++) {
		if (_vpHeld[x]->isEmpty()) {continue;}
		rOut.addByte(0x5);
		rOut.addInt(_iEntityID);
		rOut.addShort( x==0 ? 0 : 4-(x-1) );
		rOut.addShort(_vpHeld[x]->getItem().first);
		rOut.addShort(_vpHeld[x]->getItem().second);
		rOut.Finalize(FC_QUEUE_HIGH);
	}
}

void EntityLiving::updateEquipment(NetworkOut& rOut,EquipmentArray& rOldEquip) {
	if (rOldEquip.size() != 5) {throw FCRuntimeException("Illegal size");}

	for (char x=0;x<=4;x++) {
		if (_vpHeld[x]->isEmpty() && rOldEquip[x]->isEmpty()) {continue;} //Both are empty - nothing changed

		if (_vpHeld[x]->isEmpty() && !rOldEquip[x]->isEmpty()) { //A slot was cleared
			rOldEquip[x]->clear();
			
			rOut.addByte(0x5);
			rOut.addInt(_iEntityID);
			rOut.addShort(x==0 ? 0 : 4-(x-1));
			rOut.addShort(-1);
			rOut.addShort(0);
			rOut.Finalize(FC_QUEUE_HIGH);
			continue;
		}
		 

		if ((*_vpHeld[x]) != (*rOldEquip[x])) { //ItemID change
			rOldEquip[x]->setItem(_vpHeld[x]->getItem());

			rOut.addByte(0x5);
			rOut.addInt(_iEntityID);
			rOut.addShort(x==0 ? 0 : 4-(x-1));
			rOut.addShort(_vpHeld[x]->getItem().first);
			rOut.addShort(_vpHeld[x]->getItem().second);
			rOut.Finalize(FC_QUEUE_HIGH);
			continue;
		}
	}
}


void EntityLiving::setEquipment(char index,ItemID id) {
	if (index < 0 || index > 4) {throw FCRuntimeException("Illegal index");}

	if (id.isEmpty()) {
		_vpHeld[index]->clear();
		return;
	}

	try {
		_vpHeld[index]->setItem(id);
		_vpHeld[index]->setStackSize(1);
	}catch(FCRuntimeException & ex) {
		ex.rethrow();
	}
}

ItemID EntityLiving::getEquipment(char index) {
	if (index < 0 || index > 4) {throw FCRuntimeException("Illegal index");}
	return _vpHeld[index]->getItem();
}


char EntityLiving::getBaseType() {
	return Constants::get("/Entity/BaseType/Living");
}

char EntityLiving::getType(){
	return _iType;
}
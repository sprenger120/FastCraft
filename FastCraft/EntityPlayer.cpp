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
#include "EntityPlayer.h"
#include "Constants.h"
#include "World.h"
#include "NetworkOut.h"
#include "ItemSlot.h"
#include <bitset>


EntityPlayer::EntityPlayer(int iEID,MinecraftServer* pServer,World* pWorld,EntityFlags Flags,string sName,vector<ItemSlot*> vItems) try :
EntityLiving		(Constants::get("/Entity/Alive/TypeID/Player"),iEID,pServer,pWorld),
_sName				(""),
_vpHeld				(vItems),
_Flags				(Flags)
{
	if (sName.compare("") == 0) {throw Poco::RuntimeException("Illegal name");}
	if (_vpHeld.size() != 5) {throw Poco::RuntimeException("Illegal vector size");}
	for(int x=0;x<=4;x++) {
		if (_vpHeld[x] == NULL) {throw Poco::RuntimeException("Nullpointer exception");}
	}

	_sName.assign(sName);
}catch(Poco::RuntimeException & ex) {
	ex.rethrow();
}

EntityPlayer::~EntityPlayer() {
	for(int x=0;x<=4;x++) {
		delete _vpHeld[x];
	}
}

float EntityPlayer::getHeight() {
	return 1.8F;
}

float EntityPlayer::getWidth() {
	return 0.6F;
}

string EntityPlayer::getName() {
	return _sName;
}

void EntityPlayer::spawn(NetworkOut& rOut) {
	/* Spawn packet */
	rOut.addByte(0x14);
	rOut.addInt(_iEntityID);
	rOut.addString(_sName);
	rOut.addInt(int(floor(Coordinates.X * 32.0)));
	rOut.addInt(int(floor(Coordinates.Y * 32.0)));
	rOut.addInt(int(floor(Coordinates.Z * 32.0)));
	rOut.addByte(char((Coordinates.Yaw * 256.0F) / 360.0F));
	rOut.addByte(char((Coordinates.Pitch * 256.0F) / 360.0F));
	rOut.addShort(_vpHeld[0]->isEmpty() ? 0 : _vpHeld[0]->getItem().first);
	appendMetadata(rOut);
	rOut.Finalize(FC_QUEUE_HIGH);


	/* Equipment */
	for (int x=0;x<=4;x++) {
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


void EntityPlayer::appendMetadata(NetworkOut& rOut) {
	//Flags
	//            DataID      Data Type
	rOut.addByte((0&0x1f) | ((0<<5)&0xe0)); 
	
	std::bitset<5> Flags;
	Flags[0] = _Flags.isOnFire();
	Flags[1] = _Flags.isCrouched();
	Flags[2] = _Flags.isRiding();
	Flags[3] = _Flags.isSprinting();
	Flags[4] = _Flags.isRightClicking();

	rOut.addByte((unsigned char)Flags.to_ulong());
	rOut.addByte(127);
}

short EntityPlayer::getMaxHealth() {
	return 20;
}
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
#include <cmath>
#include "FCException.h"
#include "PlayerThread.h"

EntityPlayer::EntityPlayer(MinecraftServer* pServer,World* pWorld,string sName) try :
	EntityLiving		(Constants::get("/Entity/Alive/TypeID/Player"),pServer,pWorld),
	_sName				("")
{
	if (sName.compare("") == 0) {throw FCException("Illegal name");}
	_sName.assign(sName);
	_iHealth = 20;
}catch(FCException & ex) {
	ex.rethrow();
}

EntityPlayer::EntityPlayer(PlayerThread* pPlayer) try :
	EntityLiving		(Constants::get("/Entity/Alive/TypeID/Player"),pPlayer->getMinecraftServer(),pPlayer->getWorld(),false),
	_sName				(pPlayer->getUsername())
{

	Flags = pPlayer->getFlags();
	Coordinates = pPlayer->getCoordinates();
	_iHealth = 20;
	_iEntityID = pPlayer->getEntityID();

	setEquipment(0,pPlayer->getInventory().getSelectedSlot()->getItem());
	for (char x=1;x<=4;x++) {
		setEquipment(x, pPlayer->getInventory()[4+x]->getItem());
	}
}catch(FCException & ex) {
	ex.rethrow();
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
	try{
		rOut.addByte(0x14);
		rOut.addInt(_iEntityID);
		rOut.addString(_sName);
		rOut.addInt(int(floor(Coordinates.X * 32.0)));
		rOut.addInt(int(floor(Coordinates.Y * 32.0)));
		rOut.addInt(int(floor(Coordinates.Z * 32.0)));
		rOut.addByte(char((Coordinates.Yaw * 256.0F) / 360.0F));
		rOut.addByte(char((Coordinates.Pitch * 256.0F) / 360.0F));

		rOut.addShort(_vpHeld[0] == NULL || _vpHeld[0]->isEmpty() ? 0 : _vpHeld[0]->getItem().first);
		appendMetadata(rOut);
		rOut.Finalize(FC_QUEUE_HIGH);

		sendEquipment(rOut);
		
	}catch(FCException& ex) {
		ex.rethrow();
	}
}


void EntityPlayer::appendMetadata(NetworkOut& rOut) {

	try{
		//Flags
		//            DataID      Data Type);
		rOut.addByte((0&0x1f) | ((0<<5)&0xe0)); 

		std::bitset<5> bitFlags;
		bitFlags[0] = Flags.isOnFire();
		bitFlags[1] = Flags.isCrouched();
		bitFlags[2] = Flags.isRiding();
		bitFlags[3] = Flags.isSprinting();
		bitFlags[4] = Flags.isRightClicking();

		rOut.addByte((unsigned char)bitFlags.to_ulong());
		rOut.addByte(127);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

short EntityPlayer::getMaxHealth() {
	return 20;
}

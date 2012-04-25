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

EntityLiving::EntityLiving(char iType,int iEID,MinecraftServer* pServer,World* pWorld) try :
Entity			(iEID,pServer,pWorld)
{
	if (!Constants::isDefined(iType,"/Entity/Alive/TypeID")) {throw Poco::RuntimeException("Entity type not defined!");}

	_iHealth = 0;
	_iMaxHealth = 0;
	_iType = iType;
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
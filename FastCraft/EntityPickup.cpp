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
#include "EntityPickup.h"
#include "FCException.h"
#include "NetworkOut.h"

EntityPickup::EntityPickup(MinecraftServer* pMCServer,World* pWorld,ItemID ID) try : 
	Entity(pMCServer,pWorld)
{
	if (!_pMCServer->getItemInfoProvider()->isRegistered(ID)) {
		ID.first = ID.second = 0;
		FCException("Item not registered");
	}
	_itemID = ID;
}catch(FCException& ex) { 
	ex.rethrow();
}

bool EntityPickup::isAlive() {
	return false;
}

float EntityPickup::getHeight() {
	return 0.25F;
}

float EntityPickup::getWidth() {
	return 0.25F;
}


string EntityPickup::getName() {
	return string("Pickup");
}

void EntityPickup::spawn(NetworkOut& rOut){
	try{
		rOut.addByte(0x15);
		rOut.addInt(_iEntityID);
		rOut.addShort(_itemID.first);
		rOut.addByte(1);

		if(_pMCServer->getItemInfoProvider()->isBlock(_itemID) || !_pMCServer->getItemInfoProvider()->getItem(_itemID)->Damageable) {
			rOut.addShort(_itemID.second);
		}else{
			rOut.addShort(0);
		}
	
		rOut.addInt( (int) (Coordinates.X * 32.0));
		rOut.addInt( (int) (Coordinates.Y * 32.0));
		rOut.addInt( (int) (Coordinates.Z * 32.0));
		rOut.addByte(0);
		rOut.addByte(0);
		rOut.addByte(0);
	
		rOut.Finalize(FC_QUEUE_HIGH);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

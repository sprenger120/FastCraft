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

#ifndef _FASTCRAFTHEADER_ENTITYPICKUP	
#define _FASTCRAFTHEADER_ENTITYPICKUP
#include <string>
#include <vector>
#include "ItemInformationProvider.h"
#include "Entity.h"

using std::string;

class EntityPickup : public Entity {
	ItemID _itemID;
public:
	/*
	* Constructor
	* Throws FCRuntimeException if item doesn't exists

	Parameter:
	@1 : a valid MinecraftServer instance
	@2 : pointer to a World class that the entity is in
	@3 : Item ID
	*/
	EntityPickup(MinecraftServer*,World*,ItemID);


	/*
	* Returns true if entity has health points (class is derived from EntityLiving)
	*/ 
	bool isAlive();

	
	/*
	* Returns height
	*/
	float getHeight();


	/*
	* Returns width
	*/
	float getWidth();


	/*
	* Returns name of entiy
	*/
	string getName();

	
	/*
	* Sends spawn packet to given client
	*/
	void spawn(NetworkOut&);
};
#endif
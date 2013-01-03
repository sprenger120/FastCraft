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

#ifndef _FASTCRAFTHEADER_ENTITYLISTENTRY
#define _FASTCRAFTHEADER_ENTITYLISTENTRY
#include "EntityLiving.h"

/*
* This is a storage class for all spawned entities in
* PlayerThreads
*/
class MinecraftServer;

class EntityListEntry {
	int _iEntityID;
	bool _fAlive;
	char _iType; /* TypeID of EntityLiving instances */
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Entity instance
	*/ 
	EntityListEntry(Entity*);


	/*
	* Destructor
	*/
	~EntityListEntry();


	/*
	* Returns true if entity is alive
	*/
	bool isAlive();


	/*
	* Returns type of liviing entity
	* Throws FCException if entity isn't alive
	*/
	char getType();


	/*
	* Returns EntityID
	*/
	int getEntiyID();


	/*
	* Position & Equipment
	*/
	EntityCoordinates Position;
	EquipmentArray Equipment; 
};
#endif

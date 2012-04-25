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

#ifndef _FASTCRAFTHEADER_ENTITYLIVING
#define _FASTCRAFTHEADER_ENTITYLIVING
#include "Entity.h"


class EntityLiving : public Entity {
protected:
	/* Health */
	short _iHealth;
	short _iMaxHealth;

	char _iType;
public:
	/*
	* Constructor
	* Throws Poco::RuntimeException if EntityID is invalid
	* Throws Poco::RuntimeException if typeID is invalid
	
	Parameter:
	@1 : Type of entity (/Entity/Alive/TypeID/)
	@2 : EntityID
	@3 : a valid MinecraftServer instance
	@4 : pointer to a World class that the entity is in
	*/
	EntityLiving(char,int,MinecraftServer*,World*);


	/*
	* Destructor
	*/
	virtual ~EntityLiving();


	/*
	* Returns true if entity has health points (class is derived from EntityLiving)
	*/
	bool isAlive();


	/*
	* Returns height
	*/
	virtual float getHeight();


	/*
	* Returns width
	*/
	virtual float getWidth();


	/*
	* Returns name
	*/
	virtual string getName();


	/*
	* Sends spawn packet to given client
	*/
	virtual void spawn(NetworkOut&);


	/*
	* Adds the mob specific metadata to NetworkOut buffer
	*/
	virtual void appendMetadata(NetworkOut&);


	/*
	* Returns actual health points
	*/
	short getHealth();


	/*
	* Returns maximal health points
	*/
	virtual short getMaxHealth();
};
#endif
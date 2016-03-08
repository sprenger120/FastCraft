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

#ifndef _FASTCRAFTHEADER_ENTITY
#define _FASTCRAFTHEADER_ENTITY
#include "EntityCoordinates.h"
#include "MinecraftServer.h"
#include <string>

//Forward definitions
class World;
class NetworkOut;

using std::string;

class Entity {
	Tick _iCreation;
	World* _pWorld;
protected:
	MinecraftServer* _pMCServer;
	int _iEntityID;
public:
	/*
	* Constructor
	* Throws FCException if EntityID/pointers/entity type is invalid

	Parameter:
	@1 : a valid MinecraftServer instance
	@2 : pointer to a World class that the entity is in
	@3 : set this to true if you want to grab a new EID, _iEntityID will be set to -1 otherwise
	*/
	Entity(MinecraftServer*,World*,bool = true);


	/*
	* Destructor
	*/
	virtual ~Entity();


	/*
	* Returns ID of this entity
	*/
	int getEntityID();


	/*
	* Coordintes
	*/
	EntityCoordinates Coordinates;


	/*
	* Syncs coordinates with client

	Parameter:
	@1 : NetworkOut instance
	@2 : old Coordiantes
	*/
	void syncCoordinates(NetworkOut&,EntityCoordinates&);


	/*
	* Returns true if entity has health points (class is derived from EntityLiving)
	*/
	virtual bool isAlive();


	/*
	* Returns pointer to world that the entity is in
	*/
	World* getWorld();


	/*
	* Returns time since creation in ticks
	*/
	Tick getLiveTime();


	/*
	* Returns height
	*/
	virtual float getHeight();


	/*
	* Returns width
	*/
	virtual float getWidth();


	/*
	* Returns name of entiy
	*/
	virtual string getName();


	/*
	* Sends spawn packet to given client
	* Throws FCException if client isn't connected
	*/
	virtual void spawn(NetworkOut&);


	/* 
	* Returns entitys base type
	* Values are taken from /Entity/BaseType/...
	*/
	virtual char getBaseType();


	/*
	* Returns pointer to MinecraftServer that entity is in
	*/
	MinecraftServer* getMinecraftServer();
};
#endif

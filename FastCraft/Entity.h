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
	EntityCoordinates Coordinates;
	EntityCoordinates lastCoordinates;
public:
	/*
	* Constructor
	* Throws Poco::RuntimeException if EntityID/pointers/entity type is invalid
	
	Parameter:
	@1 : EntityID
	@2 : a valid MinecraftServer instance
	@3 : pointer to a World class that the entity is in
	*/
	Entity(int,MinecraftServer*,World*);


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
	double getX();
	double getY();
	double getZ();
	float getYaw();
	float getPitch();
	float getHeadYaw();
	EntityCoordinates getCoordinates();

	void setX(double);
	void setY(double);
	void setZ(double);
	void setYaw(float);
	void setPitch(float);
	void setHeadYaw(float);
	void setCoordinates(EntityCoordinates&);
	
	EntityCoordinates getLastCoordinates();


	/*
	* Syncs coordinates with client
	*/
	void syncCoordinates(NetworkOut&);


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
	*/
	virtual void spawn(NetworkOut&);
};
#endif
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

#ifndef _FASTCRAFTHEADER_ENTITYPLAYER
#define _FASTCRAFTHEADER_ENTITYPLAYER
#include <string>
#include <vector>
#include "EntityLiving.h"
#include "EntityFlags.h"

//Forward definitions
class ItemSlot;
class PlayerThread;

using std::string;
using std::vector;

class EntityPlayer : public EntityLiving {
	string _sName;
public:
	/*
	* Constructor
	* Throws FCRuntimeException if held array size is invalid
	* Throws FCRuntimeException if username is empty

	Parameter:
	@1 : a valid MinecraftServer instance
	@2 : pointer to a World class that the entity is in
	@3 : Username

	Parameter:
	* Will take the EID of the player
	@1 : a PlayerThread instance 
	*/
	EntityPlayer(MinecraftServer*,World*,string);
	EntityPlayer(PlayerThread*);			

	/*
	* Destructor
	* Will delete pHeld from heap
	*/
	~EntityPlayer();


	/*
	* Returns height
	*/
	float getHeight();


	/*
	* Returns width
	*/
	float getWidth();


	/*
	* Returns username
	*/
	string getName();


	/*
	* Sends spawn packet to given client
	*/
	void spawn(NetworkOut&);


	/*
	* Adds the mob specific metadata to NetworkOut buffer
	*/
	void appendMetadata(NetworkOut&);


	/*
	* Returns maximal health points
	*/
	short getMaxHealth();
};
#endif

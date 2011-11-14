/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011  Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_ENTITYPROVIDER
#define _FASTCRAFTHEADER_ENTITYPROVIDER

#include <vector>
#include "Structs.h"

using std::vector;

class EntityProvider {
private:
	vector<EntityType> _vEntitys;
	int _iCount;
public:
	//De- / constructor
	EntityProvider();
	~EntityProvider();

	int Add(char); //Adds a new entity to the list and returns its EID
	void Remove(int); //Delete entity from list

	char getType(int); //returns type of EID
};


#endif
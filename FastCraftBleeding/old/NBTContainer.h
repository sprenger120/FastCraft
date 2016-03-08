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
#ifndef _FASTCRAFTHEADER_NBT_CONTAINER
#define _FASTCRAFTHEADER_NBT_CONTAINER
#include <map>
using std::map;
using std::string;
class NBTTag;

class NBTContainer {
protected:
	map<string,NBTTag*> _tagMap;
public:
	/*
	* Destructor
	*/
	virtual ~NBTContainer();


	/*
	* Element access operator

	Parameter:
	@1 : Type name
	*/
	NBTTag* operator[] (string);


	/*
	* Returns true if given element exists
	*/
	bool has(string); 
};

#endif
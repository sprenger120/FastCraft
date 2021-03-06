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
#ifndef _FASTCRAFTHEADER_NBTTAGINT64
#define _FASTCRAFTHEADER_NBTTAGINT64
#include "NBTBase.h"

class NBTTagInt64 : public NBTBase {
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Name
	*/
	NBTTagInt64(ConstString&);


	/*
	* Returns the elements type
	*/ 
	char getType();
	
	
	long long Data;


	/* 
	* Internal use only
	*/
	void save(ostream&,bool = true);
	void load(istream&,bool = true);
};
#endif
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


#ifndef _FASTCRAFTHEADER_NBTTAGLIST
#define _FASTCRAFTHEADER_NBTTAGLIST
#include "NBTBase.h"
#include <vector>
using std::vector;

class NBTTagList : public NBTBase {
private:
	char _iType;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Name
	@2 : Target type
	*/
	NBTTagList(string);


	/*
	* Destructor
	*/
	~NBTTagList();


	/*
	* Returns the elements type
	*/ 
	char getType();
	

	vector<NBTBase*> Data;


	/* 
	* Internal use only
	*/
	void save(ostream&,bool = true);
	void load(istream&,bool = true);
};
#endif
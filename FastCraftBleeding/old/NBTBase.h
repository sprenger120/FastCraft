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
#ifndef _FASTCRAFTHEADER_NBT_TAGBASE
#define _FASTCRAFTHEADER_NBT_TAGBASE
#include <string>
#include "NBTContainer.h"
#include "NBTTag.h"

class NBTBase : public NBTContainer {
public:
	/*
	* Reads element data

	Parameter:
	@1 : Source stream
	@2 : Set to false to force reading tag header
	*/
	void read(istream&,bool = false);


	/*
	* Writes element data to given stream

	Parameter:
	@1 : Destination stream
	@2 : Set to false to force writing tag header
	*/
	void write(ostream&,bool = false);


	/*
	* Helper functions for easier stream data handling
	*/
	static void writeString(ostream&,string&);
	static bool readString(istream&,string&);

	static void writeInt(ostream&,int);
	static bool readInt(istream&,int&);
};
#endif
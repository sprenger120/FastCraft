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
#ifndef _FASTCRAFTHEADER_NBT_SHORT
#define _FASTCRAFTHEADER_NBT_SHORT
#include <string>
#include "NBTTagBase.h"
using std::string;

class NBTTagShort : public  NBTTagBase {
private:
	short iData;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Name of element
	*/
	NBTTagShort(string);


	/*
	* Destructor
	*/
	~NBTTagShort();


	/*
	* Writes content to target string

	Parameter:
	@1 : Target string
	@2 : Output Type: (FC_NBT_IO_RAW, FC_NBT_IO_GZIP,FC_NBT_IO_ZLIB)
	@3 : Nameless flag (won't write name field if is set to true)
	*/
	void write(string&,char,bool = false);


	/*
	* Returns a reference to internal data
	*/
	short& getDataRef();
};
#endif
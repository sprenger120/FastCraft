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
#ifndef _FASTCRAFTHEADER_NBT_FLOAT
#define _FASTCRAFTHEADER_NBT_FLOAT
#include <string>
#include "NBTTagBase.h"
using std::string;

class NBTTagFloat: public  NBTTagBase {
private:
	float nData;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Name of element
	*/
	NBTTagFloat(string);


	/*
	* Destructor
	*/
	~NBTTagFloat();


	/*
	* Writes own content to string
	* FC_NBT_OUTPUT_GZIP will be ignored 

	Parameter:
	@1 : Reference to target string
	@2 : Output Type: FC_NBT_OUTPUT_RAW or FC_NBT_OUTPUT_GZIP
	@3 : Headerless flag (won't write name and type field if is set to true)
	*/
	void write(string&,bool,bool = false);


	/*
	* Returns a reference to internal data
	*/
	float& getDataRef();
};
#endif
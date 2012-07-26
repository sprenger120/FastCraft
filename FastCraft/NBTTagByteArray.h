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
#ifndef _FASTCRAFTHEADER_NBT_BYTEARRAY
#define _FASTCRAFTHEADER_NBT_BYTEARRAY
#include <string>
#include "NBTTagBase.h"
using std::string;

class NBTTagByteArray : public  NBTTagBase {
private:
	char* _pData;
	int _iLen;
public:
	/*
	* Constructor
	* Throws FCRuntimeException if 
	  - Data pointer is NULL
	  - Lenght is <= 0

	Parameter:
	@1 : Name of element
	@2 : Data (allocated with new)
	@3 : Lenght
	*/
	NBTTagByteArray(string,char*,int);


	/*
	* Destructor
	* Deletes pData 
	*/
	~NBTTagByteArray();


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
	char* getDataRef();


	/*
	* [ ] Operator
	* Use it to get a chat at specific position
	* Will throw FCRuntimeException if index is out of bound or invalid
	*/
	char operator[] (int);
};
#endif

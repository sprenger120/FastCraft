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
#ifndef _FASTCRAFTHEADER_NBT_BINARYPARSER
#define _FASTCRAFTHEADER_NBT_BINARYPARSER
#include <string>
#include <stack>


/* This class is not thread-safe! */ 
union NBTU_Short {
	char sData[2];
	short iData;
};

union NBTU_Int {
	char sData[4];
	int iData;
	float nData;
};

union NBTU_Int64 {
	char sData[8];
	long long iData;
	double nData;
};

using std::stack;
using std::string;
class NBTTagCompound;
class NBTTagBase;

class NBTBinaryParser {
private:
	int _iSize;
	int _byteIndex;
	char* _pStr;
	
	stack<NBTTagBase*> _storageStack;

	NBTU_Short _unionShort;
	NBTU_Int _unionInt;
	NBTU_Int64 _unionInt64;
public:
	/*
	* Parses string and returns a pointer to a new NBTCompound
	* Don't forget to release the memory, allocated by this function
	* Throws FCRuntimeException if data is invalid

	Parameter:
	@1 : Data source string
	@2 : Input type (FC_NBT_IO_RAW, FC_NBT_IO_GZIP,FC_NBT_IO_ZLIB)
	*/
	NBTTagCompound* parse(char*,char,int);
	NBTTagCompound* parse(string&,char);
private:
	void nextElement(char,bool = true);
	string readName();
	int readInt();

	/*
	Parameter
	14 : True if function has to parse it's own header
	*/ 
	void handleByte(bool = true);
	void handleShort(bool = true);
	void handleInt(bool = true);
	void handleInt64(bool = true);
	void handleDouble(bool = true);
	void handleFloat(bool = true);
	void handleList(bool = true);
	void handleString(bool = true);
	void handleByteArray(bool = true);
	void handleIntArray (bool = true);
	NBTTagCompound* handleCompound(bool = true);
};
#endif
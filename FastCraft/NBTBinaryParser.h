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

class NBTBinaryParser {
private:
	NBTBinaryParser();
	~NBTBinaryParser();
public:
	/*
	* Parses a binary or gzipped string and returns a pointer to a new NBTCompound
	* Don't forget to release the memory, allocated by this function
	* Throws Poco::RuntimeException if data is invalid

	Parameter:
	@1 : Data source string
	@2 : Input type (FC_NBT_INPUT_RAW or FC_NBT_INPUT_GZIP)
	*/
	static NBTTagCompound* parse(string&,bool);
private:
	static void nextElement(string&,stack<NBTTagBase*>&,int&,char,bool = true);
	static void readName(string&,int&,string&);

	static void handleByte(		string&,	int&,	NBTTagBase*,	bool = true);
	static void handleShort(	string&,	int&,	NBTTagBase*,	bool = true);
	static void handleInt(		string&,	int&,	NBTTagBase*,	bool = true);
	static void handleInt64(	string&,	int&,	NBTTagBase*,	bool = true);
	static void handleDouble(	string&,	int&,	NBTTagBase*,	bool = true);
	static void handleFloat(	string&,	int&,	NBTTagBase*,	bool = true);
	static void handleList(		string&,	int&,	NBTTagBase*,	stack<NBTTagBase*>&,	bool = true);
	static void handleString(	string&,	int&,	NBTTagBase*,	bool = true);
	static void handleByteArray(string&,	int&,	NBTTagBase*,	bool = true);
	static NBTTagCompound* handleCompound(string&,int&,NBTTagBase*,	bool = true);
};
#endif
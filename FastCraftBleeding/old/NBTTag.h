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
#ifndef _FASTCRAFTHEADER_NBT_TAG
#define _FASTCRAFTHEADER_NBT_TAG
#include <istream>
#include <ostream>
#include <string>
using std::istream;
using std::ostream;
using std::string;

class NBTTag {
private:
	char _iTagType;
	string _sName;
protected:
	/*
	* Constructor
	
	Parameter:
	@1 : Tag type
	@2 : Tag name
	*/
	NBTTag(char,string);


	/*
	* Destructor
	*/
	virtual ~NBTTag();
public:
	/*
	* Returns element name
	*/
	string getName();


	/*
	* Returns Tag type
	*/
	char getTagType();
};
#endif
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
using std::string;

//Base class for all NBT Elements
class NBTTagBase {
protected:
	string _sName; //Contains name of the element
	char _iElementType;
public:
	/*
	* Constructor

	Paramter:
	@1 : Name of element
	@2 : Type of element (read NBTConstants.h)
	*/
	NBTTagBase(string,char);


	/*
	* Destructor
	*/
	virtual ~NBTTagBase();

	/*
	* Writes content to target string

	Parameter:
	@1 : Target string
	@2 : Output Type: FC_NBT_OUTPUT_RAW or FC_NBT_OUTPUT_GZIP
	@3 : Nameless flag (won't write name field if is set to true)
	*/
	virtual void write(string&,bool,bool = false) = 0;


	/*
	* Returns name of tag
	*/
	string getName();
	

	/*
	* Returns type of tag
	* The tag types a
	*/
	char getTagType();


	/*
	* Compresses given string, using GZIP

	Parameter:
	@1 : Reference to target string
	*/
	static void compress(string&);


	/*
	* Writes a NBT string element (2 byte lenght + string UTF8)

	Parameter:
	@1 : Target string
	@2 : string to write
	*/
	static void addHeaderlessString(string&,string&);
};
#endif
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


#ifndef _FASTCRAFTHEADER_NBTBASE
#define _FASTCRAFTHEADER_NBTBASE
#include <string>
#include <istream>
#include <ostream>
using std::string;
using std::istream;
using std::ostream;

class NBTBase {
protected: 
	string _sName;
	char _iTagID;


	/*
	* Constructor
	* Throws FCException if ID is unknown

	Parameter:
	@1 : Name of tag
	@2 : Tag ID
	*/
	NBTBase(string,int);
public:
	/*
	* Destructor
	*/
	virtual ~NBTBase();


	/*
	* sets/getsName
	*/
	void setName(string);
	string getName();


	/*
	* Creates a new tag from given id
	* Returns NULL if given tag is unknown

	Parameter:
	@1 : One of the FC_NBT_TYPE_ constants
	@2 : Name
	*/ 
	static NBTBase* newTag(int,string&);


	/*
	* Returns the elements type
	*/ 
	virtual char getType() = 0;

	
	/*
	* Writes the tag to the stream
	@1 : Target stream
	@2 : Set to true to force tag name writing
	*/
	virtual void save(ostream&,bool = true);


	/*
	* Reads custom payload from stream and initializes the tag

	Parameter:
	@1 : Source stream
	@2 : Set to true to force tag name writing
	*/
	virtual void load(istream&,bool = true);
};
#endif

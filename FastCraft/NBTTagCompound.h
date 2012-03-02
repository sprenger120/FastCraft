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
#ifndef _FASTCRAFTHEADER_NBT_COMPOUND
#define _FASTCRAFTHEADER_NBT_COMPOUND
#include <string>
#include <vector>
#include "NBTTagBase.h"
#include "NBTTagList.h"
using std::vector;


//A compound is a list of named tags
class NBTTagCompound : public NBTTagBase {
private:
	vector<NBTTagBase*> _vpElements;
public:
	/*
	* Constructor

	Parameter:
	@1 : Name of element
	*/
	NBTTagCompound(string);


	/*
	* Destructor
	* Will destroy all sub elements
	*/
	~NBTTagCompound();


	/*
	* Writes own content to string and calls write() of all sub elements

	Parameter:
	@1 : Reference to target string
	@2 : Output Type: FC_NBT_OUTPUT_RAW or FC_NBT_OUTPUT_GZIP
	@3 : Headerless flag (won't write name and type field if is set to true)
	*/
	void write(string&,bool,bool = false);


	/*
	* Adds a sub element
	* Throws Poco::RuntimeException if element name is already choosen (case sensitive search)
	* Throws Poco::RuntimeException if name is empty
	*/
	void addSubElement(NBTTagBase*);


	/*
	* Removes element by given name
	* Removing of an element will cause a destructing of all sub elements
	* Throws Poco::RuntimeException if element wasn't found

	Parameter:
	@1 : Name of element to remove
	*/
	void removeSubElement(string);


	/*
	* Searches a element by given path (e.g.  /players/ExamplePlayer/Health)
	* Will throw Poco::RuntimeEception if item wasn't found
	* Will throw Poco::RuntimeEception if given type doesn't fits with found item

	Parameter:
	@1 : Path to element
	@2 : Type of element
	*/
	NBTTagBase*	search(string,char);
private:
	int getElementIndex(string); //-1 if not found
};
#endif
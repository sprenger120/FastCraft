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


#ifndef _FASTCRAFTHEADER_ITEMID
#define _FASTCRAFTHEADER_ITEMID
#include <utility>


class ItemID : public std::pair<short,char> { 
public:
	/*
	* Constructor
	* Constructs with FC_EMPTYITEMID if no parameters were given

	Paramter:
	@1 : item ID
	@2 : metadata
	*/
	ItemID(short,char);


	/*
	* Empty constructor
	* Sets data to -1
	*/
	ItemID();


	/*
	* Comparison operators 
	*/
	bool operator == (ItemID&);
	bool operator != (ItemID&);

	
	/*
	* Returns true if id and metadata are -1
	*/
	bool isEmpty();


	/*
	* Sets data to -1
	*/
	void clear();
};
#endif
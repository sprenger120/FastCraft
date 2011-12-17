/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011  Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_ITEMIDSTORAGE
#define _FASTCRAFTHEADER_ITEMIDSTORAGE
#include <vector>
#include <string>
using std::vector;
using std::string;
/*
* This class stores informations/avoid id conflics of blocks/items
*/

struct ItemInformation{
	short ID;
	string Name;
	bool Damageable;
	bool Enchantable;
	char MaxStackSize;
	short MaxUsages;
};

class ItemInfoStorage {
private:
	static bool _fBasicItemsAdded;
	static vector<ItemInformation> _vItems;
	ItemInfoStorage();
	~ItemInfoStorage();
public:
	/*
	* Adds vanilla minecraft items/blocks
	*/
	static void addBasicIDSet();

	/*
	* Returns true if item has a damage/usage bar

	Parameter:
	@1 : Item/Block ID
	*/
	static bool isDamageable(short);


	/*
	* Returns true if item is enchantable

	Parameter:
	@1 : Item/Block ID
	*/
	static bool isEnchantable(short);


	/*
	* Returns the vanilla maximal stack size of item

	ParameteR:
	@1 : Item/Block ID
	*/
	static char getMaxStackSize();

	/*
	* Returns the vanilla maximal item usage
	* It will return 0 if item hasn't a damage/usage bar

	Parameter:
	@1 : Item ID
	*/
	static short getMaximalUsages();

	
	/*
	* Adds a item to list
	* Will throw a Poco::RuntimeException if id is already used or informations are invalid

	Parameter:
	@1 : ItemID
	@2 : Item name
	@3 : Damageable
	@4 : Enchantable
	@5 : Maxiamal stack size
	@6 : Maximal usages    *Note: set to 0 if item hasn't a damage/usage bar
	*/
	static void addItem(short,string,bool,bool,char,short);
};
#endif
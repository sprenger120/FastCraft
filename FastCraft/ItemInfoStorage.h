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
	short MaxUsage;
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
	* Will throw Poco::RuntimeException, if item wasn't found

	Parameter:
	@1 : Item/Block ID
	*/
	static bool isDamageable(short);


	/*
	* Returns true if item is enchantable
	* Will throw Poco::RuntimeException, if item wasn't found

	Parameter:
	@1 : Item/Block ID
	*/
	static bool isEnchantable(short);


	/*
	* Returns the vanilla maximal stack size of item
	* Will throw Poco::RuntimeException, if item wasn't found

	ParameteR:
	@1 : Item/Block ID
	*/
	static char getMaxStackSize(short);


	/*
	* Returns the vanilla maximal item usage
	* It will return 0 if item hasn't a damage/usage bar
	* Will throw Poco::RuntimeException, if item wasn't found

	Parameter:
	@1 : Item ID
	*/
	static short getMaxUsage(short);


	/*
	* Returns item name from given id
	* Will throw Poco::RuntimeException, if item wasn't found

	Parameter:
	@1 : Item/Block ID
	*/
	static string getName(short);


	/*
	* Returns true if item id is alreay in use, otherwise false

	Parameter:
	@1 : Item/Block ID
	*/
	static bool isRegistered(short);

	/*
	* Returns item id from given name
	* When item wasn't found, it will return 0
	* Search is case insensitive

	Parameter:
	@1 : Item/Block name
	*/
	static short getIDbyName(string);


	/*
	* Adds a item to list
	* Will throw a Poco::RuntimeException if id is already used or informations are invalid

	Parameter:
	@1 : ItemID
	@2 : Item name
	@3 : Damageable
	@4 : Enchantable
	@5 : Maxiamal stack size
	@6 : Maximal usage    *Note: set to 0 if item hasn't a damage/usage bar
	*/
	static void addItem(short,string,bool,bool,char,short);


	/*
	* Returns true if given Block/Item ID is a block

	Parameter:
	@1 : A Block/Item ID
	*/
	static bool isBlock(short);
};
#endif
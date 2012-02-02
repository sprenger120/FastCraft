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

#ifndef _FASTCRAFTHEADER_ITEMINFOSTORAGE
#define _FASTCRAFTHEADER_ITEMINFOSTORAGE
#include <vector>
#include <string>
#include <utility>
#include <Poco/Path.h>
#include <Poco/Data/Common.h>
#include <iostream>

using std::vector;
using std::string;
using std::pair;

typedef std::pair<short,char> ItemID; //Used for explicit item identification

struct ItemEntry {
	short ID;
	char SubID;
	string Name;
	bool Damageable;
	bool Enchantable;
	short Durability;
	char MaxStackSize;
	bool Eatable;
	char ConnectedBlock;
	bool Weapon;
};

struct BlockEntry {
	unsigned char ID;
	char SubID;
	string Name;
	float BlastResistance;
	char MaxStackSize;
	char SelfLightLevel;
	bool Flammable;
	bool Solid;
	char NeededTool;
	char ToolLevel;
	float Thickness;
	float Height;
	bool Stackable;
	bool NeedWallOrFloorConnection;
};

class ItemInfoStorage {
private:
	static vector<ItemEntry> _vItems;
	static vector<BlockEntry> _vBlocks;
	static vector<string> _vDatabases;

	static bool _fDatabasesLoaded;
	static Poco::Path _workingDirectory;

	ItemInfoStorage();
	~ItemInfoStorage();
public:
	/*
	* Loads iteminfo database
	
	Parameter:
	@1 : Path to FastCrafts item database directory
	*/
	static void loadDatabases(Poco::Path);


	/*
	* Refreshes item/block cache
	* It use the path, passed to loadDatabases
	* Will throw Poco::RuntimeException if no path is specified
	*/
	static void refreshCache();


	/*
	* Returns item name from given id
	* Will throw Poco::RuntimeException, if item wasn't found
	* SubID will be handle

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static string getName(ItemID);
	static string getName(short);


	/*
	* Returns true if item id is alreay in use, otherwise false

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static bool isRegistered(ItemID);
	static bool isRegistered(short);


	/*
	* Returns item id from given name
	* Will throw Poco::RuntimeException if item wasn't found
	* Search is case insensitive

	Parameter:
	@1 : Item/Block name
	*/
	static ItemID getIDbyName(string);


	/*
	* Returns true if given Block/Item ID is a block
	* Will not check item existance

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static bool isBlock(ItemID);
	static bool isBlock(short);

	/*
	* Returs block by given ID
	* Will throw Poco::RuntimeException if block not exists
	* Will throw Poco::RuntimeException if ID isn't a block

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static BlockEntry getBlock(ItemID);
	static BlockEntry getBlock(short);

	/*
	* Returns Item by given ID
	* Will throw Poco::RuntimeException if item not exists
	* Will throw Poco::RuntimeException if ID isn't a item

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static ItemEntry getItem(ItemID);
	static ItemEntry getItem(short);


	/*
	* Returns true if item is damageable
	* Will return false if ID is a block
	* Will throw Poco::RuntimeException if ID not exists

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static bool isDamageable(ItemID);
	static bool isDamageable(short);


	/*
	* Returns true if item is enchantable
	* Will return false if ID is a block
	* Will throw Poco::RuntimeException if ID not exists

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static bool isEnchantable(ItemID);
	static bool isEnchantable(short);
	
	
	/*
	* Returns true if item is flammable
	* Will return false if ID is a item
	* Will throw Poco::RuntimeException if ID not exists

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static bool isFlammable(ItemID);
	static bool isFlammable(short);


	/*
	* Returns true if block is solid
	* Will return false if ID is a item
	* Will throw Poco::RuntimeException if ID not exists

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static bool isSolid(ItemID);
	static bool isSolid(short);


	/*
	* Returns maximal stack size of item
	* Will throw Poco::RuntimeException if ID not exists

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static char getMaxStackSize(ItemID);
	static char getMaxStackSize(short);


	/*
	* Returns durability of item
	* Will returns -1 if item is a block
	* Will throw Poco::RuntimeException if ID not exists

	Parameter:
	@1 : a pair of item ID and sub ID (ItemID) or  item ID (SubID will be zero)
	*/
	static short getDurability(ItemID);
	static short getDurability(short);

	/*
	* Returns item/block count in cache
	*/
	static int getItemsInCache();
	static int getBlocksInCache();
private:
	static void isValid(ItemEntry);
	static void isValid(BlockEntry);

	static void loadSingleDatabase(Poco::Data::Session&);
};
#endif
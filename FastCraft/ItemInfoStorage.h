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
	* Adds a item to list
	* Will throw a Poco::RuntimeException if id is already used or parameters are invalid
	* Invalid parameters are:
	 - Enchantable == true AND Damageable == false
	 - Durability == 0 OR < -1
	 - Damageable == true AND Durability != -1
	 - Damageable == true AND biggest StackSize > 1 
	 - biggest StackSize <= 0
	 - Damageable == true AND Eatable == true
	 - Name is empty
	 - SubID > 16 OR SubID < 0
	 - ItemID < 256 

	Parameter:
	@1 : ItemID
	@2 : SubID (set to 0 if there isn't a item with same ItemID)
	@3 : Item name
	@4 : Damageable
	@5 : Enchantable
	@6 : Durability (set to -1 if there isn't a damage bar)
	@7 : biggest StackSize
	@8 : Eatable
	*/
	static void addItem(short,char,string,bool,bool,short,char,bool);


	/*
	* Adds a block to list
	* Will throw a Poco::RuntimeException if id is already used or parameters are invalid
	* Invalid parameters are:
	 - ItemID > 255 OR ItemID < 0
	 - biggest StackSize <= 0
	 - Unknown tool level / needed tool 
	 - BlastResistance <= 0.0F
	 - SelfLightLevel < 0 OR > 17
	 - Thickness == 0.0F OR Height == 0.0F AND Solid == true
	 - Thickness > 1.0F OR Thickness < 0.0F
	 - Height > 1.0F OR Height < 0.0F

	Parameter:
	@1 : ItemID
	@2 : SubID (set to 0 if there isn't a item with same ItemID)
	@3 : Item name 
	@4 : BlastResistance (set to 255.0F if block is unbreakable)
	@5 : biggest StackSize
	@6 : SelfLightLevel (If the block emits light, set to a value n > 0 AND  n < 17)
	@7 : Flammable
	@8 : Solid
	@9 : Needed Tool (Constants.h FC_TOOL_)
	@10: Tool Level  (Constants.h FC_TOOLLEVEL_)
	@11: Thickness
	@12: Height
	*/
	static void addBlock(unsigned char,char,string,float,char,char,bool,bool,char,char,float,float);


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
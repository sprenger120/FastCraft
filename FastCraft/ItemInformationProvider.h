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

#ifndef _FASTCRAFTHEADER_ITEMINFORMATIONPROVIDER
#define _FASTCRAFTHEADER_ITEMINFORMATIONPROVIDER
#include <vector>
#include <string>
#include <utility>
#include <Poco/Path.h>
#include <Poco/Data/Common.h>
#include <iostream>

using std::vector;
using std::string;
using std::pair;

typedef std::pair<short,char> ItemID;

struct ItemEntry {
	short ID;
	char SubID;
	string Name;
	char MaxStackSize;

	bool Damageable;
	bool Enchantable;
	short Durability;
	
	bool Eatable;
	char FoodValue;

	bool Weapon;
	char Damage;

	bool hasSubItems;

	ItemID ConnectedBlock;
};

struct BlockEntry {
	short ID;
	char SubID;
	string Name;
	char MaxStackSize;

	bool Flammable;
	bool Solid;
	float BlastResistance;
	char SelfLightLevel;
	
	char NeededTool;
	char ToolLevel;

	float Thickness;
	float Height;

	bool Stackable;
	bool CanFloat;
	bool Placeable;
	bool hasSubBlocks;
	bool noLoot;

	bool Fluid;
	char Spread;
	char Speed;

	ItemID ConnectedItem;
};


class ItemInformationProvider {
private:
	vector<ItemEntry> _vItems;
	vector<BlockEntry> _vBlocks;
public:
	/*
	* Constructor

	Parmater:
	@1 : Path the items.db 
	*/
	ItemInformationProvider(Poco::Path&);


	/*
	* Destructor
	*/
	~ItemInformationProvider();


	/*	
	* Returns item name from given ID
	* Will throw Poco::RuntimeException, if item wasn't found

	Parameter:
	@1 : ItemID
	*/
	string getName(ItemID);
	string getName(short);


	/*
	* Returns true if item ID is alreay in use, false otherwise

	Parameter:
	@1 : ItemID
	*/
	bool isRegistered(ItemID);
	bool isRegistered(short);


	/*
	* Returns item ID from given name
	* Will return FC_EMPTYITEMID if item wasn't found
	* Search is case insensitive

	Parameter:
	@1 : Item/Block name
	*/
	ItemID getIDbyName(string);


	/*
	* Returns true if given Block/Item ID is a block
	* Will not check item existance

	Parameter:
	@1 : ItemID
	*/
	bool isBlock(ItemID);
	bool isBlock(short);


	/*
	* Returs block by given ID
	* Will throw Poco::RuntimeException if block not exists
	* Will throw Poco::RuntimeException if ID isn't a block

	Parameter:
	@1 : ItemID
	*/
	BlockEntry* getBlock(ItemID);
	BlockEntry* getBlock(short);


	/*
	* Returns Item by given ID
	* Will throw Poco::RuntimeException if item not exists
	* Will throw Poco::RuntimeException if ID isn't a item

	Parameter:
	@1 : ItemID
	*/
	ItemEntry* getItem(ItemID);
	ItemEntry* getItem(short);


	/*
	* Returns item/block count in cache
	*/
	int getItemsInCache();
	int getBlocksInCache();
private:
	void isValid(ItemEntry);
	void isValid(BlockEntry);
	int search(vector<BlockEntry>&,ItemID);
	int search(vector<ItemEntry>&,ItemID);
};
#endif
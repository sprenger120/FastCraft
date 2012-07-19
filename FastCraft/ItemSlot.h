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


#ifndef _FASTCRAFTHEADER_ITEMSLOT
#define _FASTCRAFTHEADER_ITEMSLOT	
#include <vector>
#include "Structs.h"
#include "ItemInformationProvider.h"
#include <Poco/Mutex.h>

using std::vector;
class NetworkIn;
class NetworkOut;

/*
Class Note: If you change something with set... functions, it won't be sync with client
call synchronizeInventory from PlayerInventory for this
Note2: Enchantments are not supported right now
*/

class ItemSlot {
private:
	ItemID _Item;
	char _iStackSize;
	short _iUsage;
	BlockEntry* _pItemCache_Block;
	ItemEntry*  _pItemCache_Item;

	vector<Enchantment> _vEnchantments;
	ItemInformationProvider* _pItemInfoProvider;
	/*Poco::Mutex _Mutex;*/
public:
	/*
	* Construct as a free slot

	Parameter:
	@1 : A ItemInformationProvider instance
	*/
	ItemSlot(ItemInformationProvider*);


	/*
	* Constructs with a ItemID and stack size
	* Throws FCRuntimeException if something is incorrect 

	Parameter:
	@1 : A ItemInformationProvider instance
	@2 : ItemID
	@3 : stacksize
	*/
	ItemSlot(ItemInformationProvider*,ItemID,char);


	/*
	* Destructor
	*/
	~ItemSlot();


	/*
	* Adds an enchantment to item slot
	* Will throw a FCRuntimeException if slot contains more than one item (itemstack) or an unenchantable item

	Parameter:
	@1 : Reference to a Enchantment struct
	*/
	//void addEnchantment(Enchantment&);


	/*
	* Removes a enchantment
	* Will throw a FCRuntimeException if array index is out of bound

	Parameter:
	@1 : ID of enchantment
	*/
	//void removeEnchantment(int);


	/*
	* Returns a vector of all enchantments
	* Will return a blank vector if there a no enchantments on item or item is not enchantable
	*/
	//vector<Enchantment> getEnchantments();


	/*
	* Returns item data
	*/
	ItemID getItem();


	/*
	* Sets item data
	* will throw FCRuntimeException, if item doesn't exists

	Parameter:
	@1 : new item data
	*/
	void setItem(ItemID);


	/*
	* Returns acutal stack size
	*/
	char getStackSize();


	/*
	* Sets size of item stack
	* Will set stack size to orginal maximal stack size if out of bound
	*/
	void setStackSize(char);


	/* 
	* Returns actual tool usage
	* Will return -1 if item hasn't have a usagebar
	*/
	short getUsage();


	/*
	* Increments usage by one
	* Will clear item slot/decrease stack if usage of this item is exceeded
	* throws FCRuntimeException if item hasn't have a usagebar 
	*/
	void IncrementUsage();


	/*
	* Sets actual usage
	* If usage is bigger than maximal orginal item usage or lower than 0, the slot will be cleared / stack decreased
	* throws FCRuntimeException if item hasn't have a usagebar 

	Parameter:
	@1 : Usage count
	*/
	void setUsage(short);


	/*
	* Clears actual item slot
	*/
	void clear();


	/*
	* Returns true if slot is empty, false otherwise
	*/
	bool isEmpty();


	/*
	* Reads slot information from NetworkIO
	* Will throw FCRuntimeException, if connection gets aborded
	* Will overwrite existing data

	Parameter:
	@1 : Reference to a vaild NetworkIn instance
	*/
	void readFromNetwork(NetworkIn&);


	/*
	* Writes it's item information to network
	* Will throw FCRuntimeException, if connection gets aborded

	Parameter:
	@1 : Reference to a vaild NetworkOut instance
	*/
	void writeToNetwork(NetworkOut&);


	/* 
	* Operators
	*/
	bool operator == (ItemSlot&);
	bool operator != (ItemSlot&);


	/*
	* Returns true if stored item is a block
	* Returns true if slot is empty
	*/
	bool isBlock();


	/*
	* Returns the pointer to the item information cache 
	*/
	ItemEntry* getItemEntryCache();


	/*
	* Returns the pointer to the block information cache
	*/
	BlockEntry* getBlockEntryCache();

	/*
	* Decrements/Increments the stack size by one
	*/
	void DecrementStackSize();
	void IncrementStackSize();
};
#endif

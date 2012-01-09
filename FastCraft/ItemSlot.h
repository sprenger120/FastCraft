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

using std::vector;
class NetworkIn;
class NetworkOut;

/*
Class Note: If you change something with set... functions, it won't be sync with client
			call synchronizeInventory from PlayerInventory class for this
Note2: Enchantments are not supported right now
*/

class ItemSlot {
private:
	short _iItemID;
	char _iStackSize;
	short _iUsage;
	bool _isTool;

	vector<Enchantment> _vEnchantments;
public:
	/*
	* Construct as a free slot
	*/
	ItemSlot();


	/*
	* Constructs as a used slot
	* Items with a usagebar will be used 

	Parameter:
	@1 : ItemID
	@2 : Stack size
	*/
	ItemSlot(short,char);

	/*
	* Construct as a used slot with extra informations

	Parameter: 
	@1 : ItemID
	@2 : Stack size
	@3 : Usage
	*/
	ItemSlot(short,char,short);

	/*
	* Construct with a NetworkIO reference and reads slot data from given TCP stream
	* Will throw Poco::RuntimeException, if connection gets aborded

	Parameter:
	@1 : NetworkIO reference
	*/
	ItemSlot(NetworkIn&);


	/*
	* Destructor
	*/
	~ItemSlot();


	/*
	* Adds an enchantment to item slot
	* Will throw a Poco::RuntimeException if slot contains more than one item (itemstack) or an unenchantable item

	Parameter:
	@1 : Reference to a Enchantment struct
	*/
	//void addEnchantment(Enchantment&);

	
	/*
	* Removes a enchantment
	* Will throw a Poco::RuntimeException if array index is out of bound

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
	* Returns item ID
	*/
	short getItemID();


	/*
	* Sets item ID
	* will throw Poco::RuntimeException, if item doesn't exists

	Parameter:
	@1 : new item ID
	*/
	void setItemID(short);


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
	* throws Poco::RuntimeException if item hasn't have a usagebar 
	*/
	void IncrementUsage();


	/*
	* Sets actual usage
	* If usage is bigger than maximal orginal item usage or lower than 0, the slot will be cleared / stack decreased
	* throws Poco::RuntimeException if item hasn't have a usagebar 

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
	* Will throw Poco::RuntimeException, if connection gets aborded
	* Will overwrite existing data

	Parameter:
	@1 : Reference to a vaild NetworkIn instance
	*/
	void readFromNetwork(NetworkIn&);


	/*
	* Writes it's item information to network
	* Will throw Poco::RuntimeException, if connection gets aborded

	Parameter:
	@1 : Reference to a vaild NetworkOut instance
	*/
	void writeToNetwork(NetworkOut&);

	
	/* 
	* Operators
	*/
	bool operator == (ItemSlot&);
	bool operator != (ItemSlot&);
};
#endif
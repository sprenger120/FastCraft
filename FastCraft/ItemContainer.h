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

#ifndef _FASTCRAFTHEADER_ITEMCONTAINER
#define _FASTCRAFTHEADER_ITEMCONTAINER
#include <vector>
#include <string>
#include <utility>
#include "ItemInformationProvider.h"

using std::vector;
using std::string;
class ItemSlot;
class NetworkIn;
class MinecraftServer;
class PlayerThread;
class NetworkOut;

typedef std::pair<short,short> SlotRange;

class ItemContainer {
protected:
	vector<ItemSlot*> _vSlots;
	ItemSlot* _pInHand;
	MinecraftServer* _pMCServer;
	char _iType;

	/* 
	* Needed to simulate the behavior while pressing shift
	* ItemStacks that are in listed ranges (Low,High Range) 
	* will move from low -> high range / high -> low range.
	* If a stack moves from a not listed slot, it will
	* move to the preferred Range.
	* The preferred range can only be a pointer to high or low range.
	* If the preferredRange is full, it will move to the other.
	* Set preferredRange to NULL if there is none.
	*/

	SlotRange _lowerRange; 
	SlotRange _higherRange;
	SlotRange* _pPreferredRange; 

	/*
	* Constructor
	* Throws FCRuntimeException if slot count is out of range (>0)
	* Throws FCRuntimeException if MinecraftServer instance is null
	* Throws FCRuntimeException if inventory type is invalid 

	@1 : Slot count
	@2 : Valid MinecraftServer instance
	@3 : Inventory type (Constants:  /InventoryType/)
	*/
	ItemContainer(char,MinecraftServer*,char);


	/*
	* Destructor
	*/
	virtual ~ItemContainer();
public:
	/*
	* Returns Inventory Name
	*/
	virtual string getName();


	/*
	* Returns slot count 
	*/
	char getSlotCount(void);

	
	/*
	* Gets slot at given position
	* Throws FCRuntimeException if given index is out of bound
	* Doesn't synchronizes with player
	*/
	ItemSlot* operator[](int);


	/*
	* Reads the Click Window packet and appends the changes to the ItemSlot 
	*/
	void readClickWindow(PlayerThread*,NetworkIn&);

	/*
	* Merges two stacks
	* Does nothing if ItemIDs don't match
	* Throws FCRuntimeException if a pointer is null
	* Returns true if stacks were merged successfully

	Parameter:
	@1 : pointer to the pointer of Item 1
	@2 : pointer to the pointer of Item 2
	@3 : Set this to false, if you want to avoid using an empty slot for the stack
	*/
	bool mergeStacks(ItemSlot**,ItemSlot**,bool = true);


	/*
	* Synchronizes inventory with player
	
	Parameter:
	@1 : NetworkOut instance
	*/
	void syncInventory(NetworkOut&);


	/*
	* Clears all slots
	*/
	void clear();


	/*
	* Synchronizes given slot with client
	* Throws FCRuntimeException if slotID is out of bound

	Parameter:
	@1 : NetworkOut instance
	@2 : SlotID 
	*/
	void syncSlot(NetworkOut&,short);


	/*
	* Checks if you are allowed to place the given item 
	* in the given slot

	Parameter:
	@1 : slotID
	@2 : Item
	*/
	virtual bool isAllowedToPlace(short,ItemID);
private:
	/*
	* Checks if given slotID is in given SlotRange

	Parameter:
	@1 : SlotRange
	@2 : slotID
	*/
	bool isInSlotRange(SlotRange&,short);


	/*
	* Inserts/merges stack at given slotID into a free field / appropriate stack
	* Returns true if stack could be merged/inserted into a free slot
	* False otherwise
	* Throws FCRuntimeException if SlotRange pointer is NULL 
	* Throws FCRuntimeException if slotID is out of range


	Parameter:
	@1 : SlotRange pointer
	@2 : slotID
	*/
	bool insertItem(SlotRange*,short);
};
#endif
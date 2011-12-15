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

#ifndef _FASTCRAFTHEADER_PLAYERINVENTORY
#define _FASTCRAFTHEADER_PLAYERINVENTORY
#include <vector>
#include "Structs.h"

using std::vector;
class NetworkIO;

class PlayerInventory {
private:
	vector<Item> _vItemStack;
	short _iSlotSelection;
	NetworkIO& _rNetwork;

	static const short _EnchantableItems[40];
	static const short _DestructibleItems[49];
public:
	/*
	* Constructor

	Parameter:
	@1 : Refernce to a high level NetworkIO
	*/
	PlayerInventory(NetworkIO&);


	/*
	* Destructor
	*/
	~PlayerInventory();


	/*
	* synchronize server and player 
	*/
	void synchronizeInventory(); 


	/*
	* Sets slot 
	* Doesn't synchronize with player!
	* Throws Poco::RuntimeException

	Parameter:
	@1 : Slot ID
	@2 : Item
	*/
	void setSlot(short,Item&);


	/*
	* Returns Item struct of slot

	Parameter:
	@1 : SlotID
	*/
	Item getSlot(short);


	/*
	* Sets actual selected slot. 
	* Only purpose of this function: Container for this information
	* Range: 0-8

	Parameters:
	@1 : Slot ID 
	*/
	void setSlotSelection(short);


	/*
	* Returns actual selected slot
	* Range: 0-8
	*/
	short getSlotSelection();


	/*
	* Clears inventory 
	* Doesn't synchronize with player!
	*/ 
	void clear();


	/*
	* Returns true if user can enchant given item

	Parameter:
	@1 : ItemID
	*/
	static bool isEnchantable(short);


	/*
	* Returns true if given item has a usage bar

	Parameter:
	@1 : ItemID
	*/
	static bool isDestructible(short);
};

#endif
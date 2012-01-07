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
#include "ItemSlot.h"

using std::vector;
class NetworkOutRoot;
class NetworkIn;
class PlayerThread;
class PlayerPool;

class PlayerInventory {
private:
	vector<ItemSlot> _vItemStack;
	ItemSlot _ItemInHand;
	short _iSlotSelection;
	NetworkOutRoot& _rNetwork;
	NetworkIn& _rNetworkIn;
public:
	/*
	* Constructor

	Parameter:
	@1 : Refernce to player's NetworkOutRoot
	@2 : Reference to player's NetworkIn
	*/
	PlayerInventory(NetworkOutRoot&,NetworkIn&);


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
	* Will throw Poco::RuntimeException if slot id is invalid

	Parameter:
	@1 : Slot ID
	@2 : Item
	*/
	void setSlot(short,ItemSlot&);


	/*
	* Returns Item struct of slot
	* Will throw Poco::RuntimeException if slot id is invalid

	Parameter:
	@1 : SlotID
	*/
	ItemSlot getSlot(short);


	/*
	* Clears slot at given position
	* Doesn't synchronize with player!
	* Will throw Poco::RuntimeException if slot id is invalid

	Parameter:
	@1: Slot ID
	*/
	void clearSlot(short);


	/*
	* Returns actual selected slot id (0-8)
	*/
	short getSlotSelection();


	/*
	* Clears inventory 
	* Doesn't synchronize with player!
	*/ 
	void clear();


	/*
	* Handles Window click
	* throws Poco::RuntimeException if connection gets aborded 

	Parameter:
	@1 : this Pointer of class that calls this function
	*/
	void HandleWindowClick(PlayerThread*);


	/*
	* Event function for disconnect
	*/
	void HandleDisconnect();


	/*
	* Event function for closing inventory

	Parameter:
	@1 : this pointer to class that called this function
	*/
	void HandleWindowClose(PlayerPool*);


	/*
	* Event function for selection change
	* Throws Poco::RuntimeException if slot id is invalid

	Parameter:
	@1 : New selected slot id
	*/
	void HandleSelectionChange(short);


	/*
	* Returns ItemSlot object of actual selected slot
	*/
	ItemSlot& getSelectedSlot();
};

#endif
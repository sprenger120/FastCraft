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

#ifndef _FASTCRAFTHEADER_INVENTORYPLAYER
#define _FASTCRAFTHEADER_INVENTORYPLAYER
#include "ItemContainer.h"

class NetworkOutRoot;

class InventoryPlayer : public ItemContainer {
private:
	char _iActionBarSelection;
	PlayerThread* _pPlayer;
	NetworkIn& _rNetworkIn;
public:
	/*
	* Constructor
	* Rethrows all InventoryPlayer exceptions

	Parameter:
	@1 : this pointer of player
	@2 : Minecraft Server instance
	@2 : his NetworkIn instance
	*/
	InventoryPlayer(PlayerThread*,MinecraftServer*,NetworkIn&);

	
	/*
	* Returns Inventory Name
	*/
	string getName();


	/*
	* Clears slots and resets variables
	*/
	void reset();


	/*
	* Parses the HeldItemChange packet
	*/
	void readHeldItemChange();


	/*
	* Call this when the player disconnects
	*/
	void onDisconnect();


	/*
	* Call this when the player logs in
	*/
	void onLogin();


	/*
	* Decreases stack count of actual held item
	* Synchronizes with player
	*/
	void DecreaseInHandStack();


	/*
	* Returns the actual selected item in action bar
	*/
	ItemSlot* getSelectedSlot();

	
	/*
	* Checks if you are allowed to place the given item 
	* in the given slot

	Parameter:
	@1 : slotID
	@2 : Item
	*/
	bool isAllowedToPlace(short,ItemID);


	/*
	* Synchronizes the actual hold stack with client
	*/
	void syncInHandStack();
};
#endif
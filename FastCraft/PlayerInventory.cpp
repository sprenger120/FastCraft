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
#include "PlayerInventory.h"
#include "NetworkIO.h"
#include <iostream>

PlayerInventory::PlayerInventory(NetworkIO& rNtwk) :
_vItemStack(45),
	_rNetwork(rNtwk)
{
	_iSlotSelection = 0;
	clear();
}

const short PlayerInventory::_EnchantableItems[40] = {
	256,257,258,267,268,269,270,271,272,273,
	274,275,276,277,278,279,283,284,285,286,
	298,299,300,301,302,303,304,305,306,307,
	308,309,310,311,312,313,314,315,316,317
};

const short PlayerInventory::_DestructibleItems[49] = {
	256,257,258,267,268,269,270,271,272,273,
	274,275,276,277,278,279,283,284,285,286,
	298,299,300,301,302,303,304,305,306,307,
	308,309,310,311,312,313,314,315,316,317,
	290,291,292,293,294, //Hoes
	259,261,346,359 //Flint&Stone,Bow,Fishing Rod,Shears
};

PlayerInventory::~PlayerInventory() {
	_vItemStack.clear();
}

void PlayerInventory::synchronizeInventory() {
	_rNetwork.Lock();
	_rNetwork.addByte(0x68);
	_rNetwork.addByte(0);
	_rNetwork.addShort(45); //Slots in inventory

	//Build payload
	short iItemID= 0;
	for (int x=0;x<=_vItemStack.size()-1;x++){
		iItemID = _vItemStack[x].ItemID;
		if (iItemID == 0) { //Slot is empty
			_rNetwork.addShort(-1);
		}else{
			_rNetwork.addShort(iItemID);
			_rNetwork.addByte(_vItemStack[x].Count);
			_rNetwork.addShort(_vItemStack[x].Usage);

			if (isDestructible(iItemID)) { //Item has a usage bar
				//Add nbt tag
				std::cout<<"NBT TAGS ARE NOT SUPPORTED YET!!"<<"\n";
			}
		}
	}



	_rNetwork.Flush();
	_rNetwork.UnLock();
}

void PlayerInventory::clear() {
	if (_vItemStack.size()==0 || _vItemStack.size() != 45) {
		std::cout<<"Invalid inventory array!"<<"\n";
	}
	for(int x=0;x<=_vItemStack.size()-1;x++) {
		_vItemStack[x].Count = 0;
		_vItemStack[x].Enchantment = 0;
		_vItemStack[x].ItemID = 0;
		_vItemStack[x].Usage = 0;
	}
}

void PlayerInventory::setSlot(short iSlot,Item& rItem) {
	if (iSlot < 0 || iSlot > 44) {
		std::cout<<"PlayerInventory::setSlot: illegal slot id"<<"\n";
		return;
	}
	//ToDo: Add Item verfication

	_vItemStack[iSlot] = rItem;
}


bool PlayerInventory::isDestructible(short iID) {
	for (int x = 0;x<=48;x++) {
		if (_DestructibleItems[x]==iID) {
			return true;
		}
	}
	return false;
}

bool PlayerInventory::isEnchantable(short iID) {
	for (int x = 0;x<=39;x++) {
		if (_DestructibleItems[x]==iID) {
			return true;
		}
	}
	return false;
}

void PlayerInventory::setSlotSelection(short iSel) {
	if (iSel < 0 || iSel > 8) {
		std::cout<<"PlayerInventory::setSlotSelection: illegal slot id"<<"\n";
		return;
	}
	_iSlotSelection = iSel;
}

short PlayerInventory::getSlotSelection() {
	return _iSlotSelection;
}
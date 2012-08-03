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
#include "InventoryPlayer.h"
#include "MinecraftServer.h"
#include "NetworkIn.h"
#include "NetworkOutRoot.h"
#include "PlayerThread.h"
#include <utility>

InventoryPlayer::InventoryPlayer(PlayerThread* pPlayer,MinecraftServer* pMCServer,NetworkIn& rIn) try : 
	ItemContainer(45,pMCServer,0),
	_rNetworkIn(rIn)
{
	_lowerRange = std::pair<short,short>(36,44);
	_higherRange = std::pair<short,short>(9,35);
	_pPreferredRange = &_higherRange;
	_pPlayer = pPlayer;
	_iActionBarSelection = 0;
}catch(FCRuntimeException& ex){
	ex.rethrow();
}

string InventoryPlayer::getName() {
	return string("Player Inventory");
}

void InventoryPlayer::reset() {
	clear();
	_iActionBarSelection = 0;
}

void InventoryPlayer::readHeldItemChange() {
	try {
		_iActionBarSelection = (char)_rNetworkIn.readShort();
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	if (_iActionBarSelection < 0 || _iActionBarSelection > 8) {_pPlayer->Disconnect("Illegal ActionBarSelection ID");}
}

void InventoryPlayer::onDisconnect() {
	reset();
}

void InventoryPlayer::onLogin() {
	try{
	NetworkOut Out(_pPlayer->getNetworkOutRoot());
	syncInventory(Out);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}

ItemSlot* InventoryPlayer::getSelectedSlot() {
	return _vSlots[36+_iActionBarSelection];
}

void InventoryPlayer::DecreaseInHandStack() {
	char iSize = getSelectedSlot()->getStackSize();
	if (iSize == 1) {
		getSelectedSlot()->clear();
	}else{
		getSelectedSlot()->setStackSize(iSize-1);
	}
	syncInHandStack();
}

bool InventoryPlayer::isAllowedToPlace(short iSlot,ItemID itemID) {
	if (iSlot >= 5 && iSlot <= 8){ 
		if (!_vSlots[iSlot]->isEmpty()) {return false;}
		if (_vSlots[iSlot]->getStackSize() + 1 > 1){return false;}
	}
	switch(iSlot) {
	case 0:
		return false;
	case 5:
		if (itemID.first != 298 && itemID.first != 302 && 
			itemID.first != 306 && itemID.first != 310 && 
			itemID.first != 314)
		{
			return false;
		}
		return true;
	case 6:
		if (itemID.first != 299 && itemID.first != 303 && 
			itemID.first != 307 && itemID.first != 311 && 
			itemID.first != 315)
		{
			return false;
		}
		return true;
	case 7:
		if (itemID.first != 300 && itemID.first != 304 && 
			itemID.first != 308 && itemID.first != 312 && 
			itemID.first != 316)
		{
			return false;
		}
		return true;
	case 8:
		if (itemID.first != 301 && itemID.first != 305 && 
			itemID.first != 309 && itemID.first != 313 && 
			itemID.first != 317)
		{
			return false;
		}
		return true;
	default:
		return true;
	}
}

void InventoryPlayer::syncInHandStack() {
	try{
		NetworkOut Out(_pPlayer->getNetworkOutRoot());
		syncSlot(Out,36+_iActionBarSelection);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}
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
#include "PlayerThread.h"
#include "ItemInfoStorage.h"
#include "ItemSlot.h"
//#include "NBT.h"
#include <iostream>

using std::cout;

PlayerInventory::PlayerInventory(NetworkIO& rNtwk) :
_vItemStack(45),
	_rNetwork(rNtwk),
	_ItemInHand()
{
	_iSlotSelection = 0;
	clear();
}

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
		iItemID = _vItemStack[x].getItemID();
		if (iItemID == 0) { //Slot is empty
			_rNetwork.addShort(-1);
		}else{
			_rNetwork.addShort(iItemID);
			_rNetwork.addByte(_vItemStack[x].getStackSize());
			_rNetwork.addShort(_vItemStack[x].getUsage());

			if (ItemInfoStorage::isDamageable(iItemID)) { //Item has a usage bar
				_rNetwork.addShort(-1); //enchantments are not supported yet - a java nullpointer exception blocks the enchantment phrasing in client

				/*
				//Add nbt tag
				Enchantment Ench;
				Ench.EnchID = 19;
				Ench.Level = 1;
				NBT::writeEnchantment(Ench,_rNetwork);
				//	std::cout<<"NBT TAGS ARE NOT SUPPORTED YET!!"<<"\n";
				*/
			}
		}
	}



	_rNetwork.Flush();
	_rNetwork.UnLock();
}

void PlayerInventory::clear() {
	if (_vItemStack.size() != 45) {
		std::cout<<"Invalid inventory array!"<<"\n";
		throw Poco::RuntimeException("Invalid inventory array!");
	}
	for(int x=0;x<=_vItemStack.size()-1;x++) {
		_vItemStack[x].clear();
	}
}

void PlayerInventory::setSlot(short iSlot,ItemSlot& rItem) {
	if (iSlot < 0 || iSlot > 44) {
		std::cout<<"PlayerInventory::setSlot illegal slot id"<<"\n";
		throw Poco::RuntimeException("Invalid slot ID");
	}

	_vItemStack[iSlot] = rItem;
}

ItemSlot PlayerInventory::getSlot(short iSlot) {
	if (iSlot < 0 || iSlot > 44) {
		std::cout<<"PlayerInventory::getSlot illegal slot id"<<"\n";
		throw Poco::RuntimeException("Invalid slot ID");
	}
	return _vItemStack[iSlot];
}

void PlayerInventory::clearSlot(short iSlot) {
	if (iSlot < 0 || iSlot > 44) {
		std::cout<<"PlayerInventory::clearSlot: illegal slot id"<<"\n";
		throw Poco::RuntimeException("Invalid slot ID");
	}
	_vItemStack[iSlot].clear();
}

short PlayerInventory::getSlotSelection() {
	return _iSlotSelection;
}


void PlayerInventory::HandleWindowClick(PlayerThread* pPlayer) {
	char iWindowID;
	short iSlot,iActionNumber;
	bool fShift,fRightClick;
	ItemSlot Item;

	try{
		iWindowID = _rNetwork.readByte();
		iSlot = _rNetwork.readShort();
		fRightClick = _rNetwork.readBool();
		iActionNumber = _rNetwork.readShort();
		fShift = _rNetwork.readBool();

		Item.readFromNetwork(_rNetwork);

		//Check data
		if (iWindowID != 0) {
			pPlayer->Kick("Unsupported window type");
			return;
		}
		if (iSlot < 0 || iSlot > 45) { //Invalid slot id, caused by clicking out of inventory field
			return;
		}

		switch(fRightClick) {
		case false: //Left klick
			if (_ItemInHand.isEmpty()) { //Player took item to cursor
				_ItemInHand = Item;
				clearSlot(iSlot);
			}else{ //Player put it into a new slot
				if (_vItemStack[iSlot].isEmpty()) { //Slot is empty
					_vItemStack[iSlot] = _ItemInHand;
					_ItemInHand.clear();
				}else{ //Not empty 
					if (_vItemStack[iSlot].getItemID() ==  _ItemInHand.getItemID()) { // Merge stacks
						short iItemMaxStackSize = ItemInfoStorage::getMaxStackSize(_ItemInHand.getItemID());
						short iItemSum = _vItemStack[iSlot].getStackSize() +  _ItemInHand.getStackSize();

						if (iItemSum <= iItemMaxStackSize) {//Merge stacks into one slot
							_vItemStack[iSlot].setStackSize(iItemSum);
							_ItemInHand.clear(); //Hand is empty now
						}else{//Slot size is excessed
							iItemSum -= iItemMaxStackSize;
							_vItemStack[iSlot].setStackSize(iItemMaxStackSize); //Stack in inventory is full
							_ItemInHand.setStackSize(iItemSum); //Subtract items from in hand stack
						}
					}else{ //Switch stacks
						ItemSlot Temp = _ItemInHand;
						_ItemInHand = _vItemStack[iSlot];
						_vItemStack[iSlot] = Temp;
					}
				}
			}
			break;
		case true: //right klick
			if (_ItemInHand.isEmpty()) { //Hand is empty -> stack split action
				if(_vItemStack[iSlot].getStackSize() == 1) { //Single item stacks can not splited -> items in hand get transfered to slot
					return;
				}

				char iFullStackSize = _vItemStack[iSlot].getStackSize();
				_ItemInHand = _vItemStack[iSlot]; //Copy informations

				_vItemStack[iSlot].setStackSize(iFullStackSize/2);//Interger division - without decimal places
				_ItemInHand.setStackSize(iFullStackSize/2  +  ( iFullStackSize%2 ? 1 : 0 ) ); //if stack size odd: bigger part is in players hand
			}else{//item in hand -> put one item into other stack
				if (_vItemStack[iSlot].getItemID() == 0) { //Put one item into an empty slot
					
					_vItemStack[iSlot].setItemID(_ItemInHand.getItemID());
					_vItemStack[iSlot].setStackSize(1);

					_ItemInHand.setStackSize(_ItemInHand.getStackSize()-1);
					return;
				}
				if (_vItemStack[iSlot].getItemID() ==  _ItemInHand.getItemID()) { //If stack types are equal
					if(ItemInfoStorage::getMaxStackSize(_vItemStack[iSlot].getItemID()) ==  _vItemStack[iSlot].getStackSize() ) {
						return; //max stack size reached
					}

					_vItemStack[iSlot].setStackSize(_vItemStack[iSlot].getStackSize()+1);
					_ItemInHand.setStackSize(_ItemInHand.getStackSize()-1);
				}else{//Switch stacks
					ItemSlot Temp = _ItemInHand;
					_ItemInHand = _vItemStack[iSlot];
					_vItemStack[iSlot] = Temp;
				}

			}
			break;
		}
	} catch(Poco::RuntimeException& ex) {
		std::cout<<"PlayerInventory::HandleWindowClick exception:"<<ex.message()<<"\n";
		pPlayer->Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerInventory::HandleDisconnect() {
	_ItemInHand.clear();
}

void PlayerInventory::HandleWindowClose(PlayerPool* pPool) {
	_ItemInHand.clear();
	//ToDo: Add pickup spawn
}

void PlayerInventory::HandleSelectionChange(short iSel) {
	if (iSel < 0 || iSel > 8) {
		throw Poco::RuntimeException("Illegal holding slotID");
		return;
	}
	_iSlotSelection = iSel;
}
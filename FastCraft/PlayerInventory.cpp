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
#include "NetworkOutRoot.h"
#include "NetworkOut.h"
#include "NetworkIn.h"
#include "PlayerThread.h"
#include "ItemInfoStorage.h"
#include "ItemSlot.h"
//#include "NBT.h"
#include <iostream>

using std::cout;

PlayerInventory::PlayerInventory(NetworkOutRoot& rNtwk,NetworkIn& rIn) :
_vItemStack(45),
	_rNetwork(rNtwk),
	_ItemInHand(),
	_rNetworkIn(rIn)
{
	_iSlotSelection = 0;
	clear();
}

PlayerInventory::~PlayerInventory() {
	_vItemStack.clear();
}

void PlayerInventory::synchronizeInventory() {
	NetworkOut Out = _rNetwork.New();

	Out.addByte(0x68);
	Out.addByte(0);
	Out.addShort(45); //Slots in inventory

	//Build payload
	short iItemID= 0;
	for (int x=0;x<=_vItemStack.size()-1;x++){
		iItemID = _vItemStack[x].getItemID();
		if (iItemID == 0) { //Slot is empty
			Out.addShort(-1);
		}else{
			Out.addShort(iItemID);
			Out.addByte(_vItemStack[x].getStackSize());
			Out.addShort(_vItemStack[x].getUsage());

			if (ItemInfoStorage::isDamageable(iItemID)) { //Item has a usage bar
				Out.addShort(-1); //enchantments are not supported yet - a java nullpointer exception blocks the enchantment phrasing in client

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

	Out.Finalize(FC_QUEUE_HIGH);
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
		iWindowID = _rNetworkIn.readByte();
		iSlot = _rNetworkIn.readShort();
		fRightClick = _rNetworkIn.readBool();
		iActionNumber = _rNetworkIn.readShort();
		fShift = _rNetworkIn.readBool();

		Item.readFromNetwork(_rNetworkIn);

		//Check data
		if (iWindowID != 0) {
			pPlayer->Kick("Unsupported window type");
			return;
		}
		if (iSlot < 0 || iSlot > 45) { //Invalid slot id, caused by clicking out of inventory field
			pPlayer->Kick("Invalid slot ID");
			return;
		}

		if (fShift) { //Shift was pressed
			if (_vItemStack[iSlot].getItemID() == 0) { //Affected slot is empty
				return; 
			}

			char iMaxStackSize = ItemInfoStorage::getMaxStackSize(_vItemStack[iSlot].getItemID());
			short iFrom=0;
			short iTo=0;

			if (iSlot < 9) {//Crafting and amor slots -> Main inventory
				iFrom = 9;
				iTo = 35;
			}
			if (iSlot >= 9 && iSlot <= 35) { //Main inventory -> action bar
				iFrom = 36;
				iTo = 44;
			}
			if (iSlot >= 36 && iSlot <= 44) {//Action bar -> main inventory
				iFrom = 9;
				iTo = 35;
			}

			if (iTo==0 && iFrom==0) {
				std::cout<<"logical error"<<"\n";
			}

			//Merge same item stacks
			for (int x=iFrom;x<=iTo;x++) { 
				if(_vItemStack[x].getItemID()==_vItemStack[iSlot].getItemID()) {
					short iSum = _vItemStack[x].getStackSize() + _vItemStack[iSlot].getStackSize();

					if (iSum < iMaxStackSize) { //Stacks fit in each other
						_vItemStack[x].setStackSize((char)iSum);
						_vItemStack[iSlot].clear();
					}else{ //Stack size reached
						_vItemStack[x].setStackSize(iMaxStackSize); //set to maximal
						_vItemStack[iSlot].setStackSize(iSum-iMaxStackSize); //leave rest in slot
					}
				}
			}
			
			//If there are still some items in the affected slot
			if (_vItemStack[iSlot].getStackSize() > 0) { //Look for a free slot
				for (int x=iFrom;x<=iTo;x++) {
					if(_vItemStack[x].getItemID()==0) {
						_vItemStack[x] = _vItemStack[iSlot];
						_vItemStack[iSlot].clear();
						break;
					}
				}
			}
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
							_vItemStack[iSlot].setStackSize((char)iItemSum);
							_ItemInHand.clear(); //Hand is empty now
						}else{//Slot size is excessed
							iItemSum -= iItemMaxStackSize;
							_vItemStack[iSlot].setStackSize((char)iItemMaxStackSize); //Stack in inventory is full
							_ItemInHand.setStackSize((char)iItemSum); //Subtract items from in hand stack
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

ItemSlot PlayerInventory::getSelectedSlot() {
	return _vItemStack[36 + _iSlotSelection];
}
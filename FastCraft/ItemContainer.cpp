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
#include "ItemContainer.h"
#include "NetworkIn.h"
#include "FCRuntimeException.h"
#include "MinecraftServer.h"
#include "ItemSlot.h"
#include <iostream>
#include "PlayerThread.h"
#include "Constants.h"
using std::cout;

ItemContainer::ItemContainer(char iCount,MinecraftServer* pMCServer,char iType) {
	if (pMCServer == NULL){throw FCRuntimeException("Nullpointer");}
	if (iCount < 0) {throw FCRuntimeException("Slot count is below zero");}
	if (!Constants::isDefined(iType,"/InventoryType/")) {throw FCRuntimeException("Illegal inventory type");}

	_pMCServer = pMCServer;
	_iType = iType;

	_pInHand = new ItemSlot(_pMCServer->getItemInfoProvider());
	for (char x = 0;x<=iCount-1;x++) {
		_vSlots.push_back(new ItemSlot(pMCServer->getItemInfoProvider()));
	}
}


ItemContainer::~ItemContainer() {
	for (char x=0;x<=_vSlots.size()-1;x++) {
		delete _vSlots[x];
	}
	delete _pInHand;
}

string ItemContainer::getName() {
	return string("");
}

char ItemContainer::getSlotCount() {
	return _vSlots.size();
}

ItemSlot* ItemContainer::operator[](int ID) {
	if (ID < 0 || ID > _vSlots.size()-1) {throw FCRuntimeException("Invalid ID");}
	return _vSlots[ID];
}

void ItemContainer::readClickWindow(PlayerThread* pPlayer,NetworkIn& rIn) {
	ItemSlot Item(_pMCServer->getItemInfoProvider());

	try{
		char iWindowID = rIn.readByte();
		short iSlot = rIn.readShort();
		bool fRightClick = rIn.readBool();
		rIn.readShort(); //TransactionID
		bool fShift = rIn.readBool();

		Item.readFromNetwork(rIn);


		/* Check data */
		if (iWindowID != 0) {
			cout<<"Unsupported inventory type. Sorry\n";
			return;
		}

		if (iSlot < 0 || iSlot > _vSlots.size()) {    //Invalid slot id, caused by clicking out of inventory field
			cout<<pPlayer->getUsername()<<" tried to crash the server with an invalid inventory slot ID\n";
			return;
		}

		if (fShift) {
			if (_vSlots[iSlot]->isEmpty()) {return;} //Kicked on an empty slot

			/* source is in a range */
			if (isInSlotRange(_lowerRange,iSlot)) { //In low Range -> move to high range
				insertItem(&_higherRange,iSlot);
				return;
			}
			if(isInSlotRange(_higherRange,iSlot)) { //High -> low range
				insertItem(&_lowerRange,iSlot);
				return;
			}


			/* source is not in a range */
			if(_pPreferredRange == NULL) {return;} //No preferred range defined, do nothing

			/* Preferred range is defined */
			if (_pPreferredRange == &_lowerRange) { //preferred range is low range
				if (insertItem(&_lowerRange,iSlot)) {return;} //stack was moved successfully 
				insertItem(&_higherRange,iSlot); // low range is full, try high range
				return;
			}

			if (_pPreferredRange == &_higherRange) { 
				if (insertItem(&_higherRange,iSlot)) {return;} 
				insertItem(&_lowerRange,iSlot); 
				return;
			}

			throw FCRuntimeException("Illegal preferred range");
		}

		if (fRightClick)  {
			if (isAllowedToPlace(iSlot,_pInHand->getItem())) {
				if (!_pInHand->isEmpty()) {
					/* Put one item into this empty slot */
					if (_vSlots[iSlot]->isEmpty()) { 
						_vSlots[iSlot]->setItem(_pInHand->getItem());
						_vSlots[iSlot]->setStackSize(1);
						_pInHand->DecrementStackSize();
						return;
					}

					/* Add one item to same stack */
					if ((*_vSlots[iSlot]) == (*_pInHand)) { 
						char iMaxStackSize = 0;
						if (_vSlots[iSlot]->isBlock()) {
							iMaxStackSize = _vSlots[iSlot]->getBlockEntryCache()->MaxStackSize;
						}else{
							iMaxStackSize = _vSlots[iSlot]->getItemEntryCache()->MaxStackSize;
						}
						if (_vSlots[iSlot]->getStackSize() + 1 > iMaxStackSize) {return;}

						_vSlots[iSlot]->IncrementStackSize();
						_pInHand->DecrementStackSize();
						return;
					}

					/* swap stacks */
					std::swap(_vSlots[iSlot],_pInHand);
					return;
				}
			}else{
				NetworkOut Out(pPlayer->getNetworkOutRoot());
				syncSlot(Out,iSlot);
				return;
			}
			char iStackSize = _vSlots[iSlot]->getStackSize();

			if (iStackSize%2) {
				iStackSize /= 2;
				_vSlots[iSlot]->setStackSize(iStackSize);
				_pInHand->setItem(_vSlots[iSlot]->getItem());
				_pInHand->setStackSize(iStackSize+1);
				return;
			}

			iStackSize /= 2;
			_vSlots[iSlot]->setStackSize(iStackSize);
			_pInHand->setItem(_vSlots[iSlot]->getItem());
			_pInHand->setStackSize(iStackSize);
			return;
		}

		if (!_pInHand->isEmpty()) {
			if (!isAllowedToPlace(iSlot,_pInHand->getItem())) {
				NetworkOut Out(pPlayer->getNetworkOutRoot());
				syncSlot(Out,iSlot);
				return;
			}
		}
		mergeStacks(&_vSlots[iSlot],&_pInHand);
	} catch(FCRuntimeException& ex) {
		std::cout<<"PlayerInventory::HandleWindowClick exception:"<<ex.getMessage()<<"\n";
		pPlayer->Disconnect(FC_LEAVE_OTHER);
	}
}

bool ItemContainer::isInSlotRange(SlotRange& rRange,short iSlot) {
	if (iSlot >= rRange.first && iSlot <= rRange.second) {return true;}
	return false;
}

bool ItemContainer::insertItem(SlotRange* pRange,short iSlot) {
	if (pRange == NULL) {throw FCRuntimeException("Nullpointer");}
	if (iSlot < 0 || iSlot > _vSlots.size()-1) {throw FCRuntimeException("Illegal slotID");}

	try {
		short i;
		//Search a stack to merge
		for(i = pRange->first; i<= pRange->second;i++) {
			if (mergeStacks(&_vSlots[i],&_vSlots[iSlot],false)) {return true;}
		}

		//Nothing found, try to use an empty one
		for(i = pRange->first; i<= pRange->second;i++) {
			if (mergeStacks(&_vSlots[i],&_vSlots[iSlot])) {return true;}
		}
	}catch(FCRuntimeException& ex){
		ex.rethrow();
	}
	return false;
}

bool ItemContainer::mergeStacks(ItemSlot** item1,ItemSlot** item2,bool fUseEmptySlot) {
	if (item1 == NULL || item2 == NULL) {throw FCRuntimeException("Nullpointer");}
	if ((*item1)->isEmpty() && (*item2)->isEmpty()){
		cout<<"ItemContainer::mergeStacks logic error. Both are empty\n";
		return false;
	}

	if (fUseEmptySlot) {
		if (((*item1)->isEmpty() && !((*item2)->isEmpty())) ||      
			((*item2)->isEmpty() && !((*item1)->isEmpty())))
		{//Swap stacks
			std::swap(*item1,*item2);
			return true;
		}
	}

	if ((**item1) != (**item2)) {return false;}

	char iMaxStackSize = 0;
	if ((*item1)->isBlock()) {
		iMaxStackSize = (*item1)->getBlockEntryCache()->MaxStackSize;
	}else{
		iMaxStackSize = (*item1)->getItemEntryCache()->MaxStackSize;
	}

	if ((*item1)->getStackSize() >= iMaxStackSize) {return false;} //Stack if full
	short iSum = (*item1)->getStackSize() + (*item2)->getStackSize();

	if (iSum < iMaxStackSize) { //Merge stacks
		(*item1)->setStackSize((char)iSum);
		(*item2)->clear();
	}else{
		(*item1)->setStackSize(iMaxStackSize);
		(*item2)->setStackSize(iSum - iMaxStackSize);
	}
	return true;
}

void ItemContainer::syncInventory(NetworkOut& rOut) {
	rOut.addByte(0x68);
	rOut.addByte(_iType);
	rOut.addShort(_vSlots.size()); //Slots in inventory

	for (short x=0;x<=_vSlots.size()-1;x++){
		_vSlots[x]->writeToNetwork(rOut);
	}

	rOut.Finalize(FC_QUEUE_HIGH);
}

void ItemContainer::clear() {
	for (short x=0;x<=_vSlots.size()-1;x++){
		_vSlots[x]->clear();
	}
}

void ItemContainer::syncSlot(NetworkOut& rOut,short iSlot) {
	if (iSlot < 0 || iSlot > _vSlots.size() -1) {throw FCRuntimeException("Invalid slotID");}

	rOut.addByte(0x67);
	rOut.addByte(_iType);
	rOut.addShort(iSlot); 
	_vSlots[iSlot]->writeToNetwork(rOut);
}

bool ItemContainer::isAllowedToPlace(short,ItemID) {
	return true;
}
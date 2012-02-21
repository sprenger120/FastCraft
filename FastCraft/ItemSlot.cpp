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

#include "ItemSlot.h"
#include "ItemInfoStorage.h"
#include <Poco/Exception.h>
#include <iostream>
#include "NetworkIn.h"
#include "NetworkOut.h"

ItemSlot::ItemSlot(): 
_vEnchantments(0)
{
	_Item.first = _Item.second = 0;
	_iStackSize = 0;
	_iUsage = 0;
	_isTool = false;
}

ItemSlot::ItemSlot(ItemID id,char stacksize):
_vEnchantments(0)
{
	id = removeUnnecessarySubID(id);
	char iInfo;
	try{
		iInfo = ItemInfoStorage::getMaxStackSize(id);
	}catch (Poco::RuntimeException) {
		std::cout<<"ItemSlot::ItemSlot Item wasn't found. Affected:"<<id.first<<":"<<id.second<<"\n";
		clear();
		return;
	}

	_Item = id;
	_iUsage = 0;

	//Check stack size
	if (stacksize<0){
		_iStackSize=0;
		std::cout<<"ItemSlot::ItemSlot Stacksize invalid. Affected:"<<id.first<<":"<<id.second<<"\n";
	}
	if (stacksize > iInfo) {
		_iStackSize = iInfo;
		std::cout<<"ItemSlot::ItemSlot Stacksize exceeded. Affected:"<<id.first<<":"<<id.second<<"\n";
	}else{
		_iStackSize = stacksize;
	}
	if (stacksize==0) {
		clear();
		return;
	}

	//Set isTool
	if (ItemInfoStorage::isDamageable(id)) {
		_isTool=true;
	}else{
		_isTool=false;
	}
}

ItemSlot::ItemSlot(ItemID id,char stacksize,short usage) :
_vEnchantments(0)
{
	short iUsageInfo;
	char iStackInfo;
	id = removeUnnecessarySubID(id);

	try	{
		iStackInfo = ItemInfoStorage::getMaxStackSize(id);
		iUsageInfo = ItemInfoStorage::getDurability(id);
	}catch (Poco::RuntimeException) {
		std::cout<<"ItemSlot::ItemSlot(ex) Item wasn't found. Affected:"<<id.first<<":"<<id.second<<"\n";
		clear();
		return;
	}

	_Item = id;

	//Check stack size
	if (stacksize<0){
		_iStackSize=0;
		std::cout<<"ItemSlot::ItemSlot(ex) Stacksize invalid. Affected:"<<id.first<<":"<<id.second<<"\n";
	}
	if (stacksize > iStackInfo) {
		_iStackSize = iStackInfo;
		std::cout<<"ItemSlot::ItemSlot(ex) Stacksize exceeded. Affected:"<<id.first<<":"<<id.second<<"\n";
	}else{
		_iStackSize = stacksize;
	}
	if (stacksize==0) {
		clear();
		return;
	}

	if (ItemInfoStorage::isDamageable(_Item)) {
		_isTool=true;
		//Check usage
		if (usage > iUsageInfo || usage < 0) {
			std::cout<<"ItemSlot::ItemSlot(ex) Usage exceeded/invalid. Affected:"<<id.first<<":"<<id.second<<"\n";
			_iUsage = 0;
		}else{
			_iUsage = usage;
		}
	}else{
		_isTool=false;
		if (_Item.second < 0 || _Item.second > 15) {
			std::cout<<"ItemSlot::ItemSlot(ex) invalid metadata. Affected:"<<id.first<<":"<<id.second<<"\n";
			_Item.second = 0;
		}
	}
}


ItemSlot::ItemSlot(NetworkIn& rNetwork) {
	try {
		readFromNetwork(rNetwork);
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	removeUnnecessarySubID();
}

ItemSlot::~ItemSlot() {
	_vEnchantments.clear();
}

ItemID ItemSlot::getItem() {
	return _Item;
}

void ItemSlot::setItem(ItemID id) {
	try{
		id = removeUnnecessarySubID(id);
		if (ItemInfoStorage::isDamageable(id)) {
			_isTool=true;
		}else{
			_isTool=false;
		}
	}catch(Poco::RuntimeException) {
		std::cout<<"ItemSlot::setItemID Item wasn't found. Affected:"<<id.first<<":"<<id.second<<"\n";
		throw Poco::RuntimeException("ID not registered");
	}
	if (id.first==0) {
		clear();
		return;
	}else{
		_Item = id;
	}
}

char ItemSlot::getStackSize() {
	return _iStackSize;
}

void ItemSlot::setStackSize(char size) {
	char SizeInfo;

	if (size<0){
		_iStackSize=0;
		std::cout<<"ItemSlot::setStackSize Stacksize invalid."<<"\n";
		return;
	}
	if (size==0) {
		clear();
		return;
	}

	SizeInfo = ItemInfoStorage::getMaxStackSize(_Item);

	if (size > SizeInfo) {
		_iStackSize = SizeInfo;
		std::cout<<"ItemSlot::setStackSize Stacksize exceeded."<<"\n";
	}else{
		_iStackSize = size;
	}
}

short ItemSlot::getUsage() {
	return _iUsage;
}


void ItemSlot::IncrementUsage() {
	if (!_isTool) {
		std::cout<<"ItemSlot::IncrementUsage Not a tool!"<<"\n";
		throw Poco::RuntimeException("Not a tool");
	}
	short iUsageInfo = ItemInfoStorage::getDurability(_Item);

	if(_iUsage > iUsageInfo || _iUsage < 0) {
		if (_iStackSize == 1) {//Last item in stack 
			clear();
		}else{//Decrement stack size if there is more than one
			_iStackSize--;
			_iUsage = 0;
		}
		return;
	}
	_iUsage++;
}

void ItemSlot::setUsage(short iUsage){
	short iUsageInfo;	
	if (!_isTool) {
		std::cout<<"ItemSlot::IncrementUsage Not a tool!"<<"\n";
		throw Poco::RuntimeException("Not a tool");
	}

	iUsageInfo = ItemInfoStorage::getDurability(_Item);

	if(iUsage > iUsageInfo || iUsage < 0) {
		//Decrement stack size if more than one
		if (_iStackSize == 1) {
			clear(); //Stack will is empty
		}else{
			_iStackSize--;
			_iUsage = 0;
		}
		return;
	}
	_iUsage = iUsage;
}

void ItemSlot::clear() {
	_Item.first = _Item.second = 0;
	_iStackSize = 0;
	_iUsage = 0;
	_isTool=false;
}

bool ItemSlot::isEmpty() {
	if (_iStackSize == 0 || _Item.first == 0) {
		return true;
	}else{
		return false;
	}
}

void ItemSlot::readFromNetwork(NetworkIn& rNetwork) {
	_isTool=false;
	try	{
		short iItemID,Usage=0;
		char StackSize=0;
		bool _fDamageable = false;

		iItemID = rNetwork.readShort();
		if (iItemID != -1) {
			StackSize = rNetwork.readByte();
			Usage = rNetwork.readShort();
			
			_fDamageable = ItemInfoStorage::isDamageable(iItemID);

			if (_fDamageable) {
				short iEnchPayload = rNetwork.readShort();
				if (iEnchPayload !=-1) {
					std::cout<<"Enchantment payload:"<<iEnchPayload<<"\n";
					throw Poco::RuntimeException("Enchantments not supported yet");
				}
			}
		}
		if (iItemID == -1) { //Slot is empty
			clear();
			return;
		}

		//Get information about maximal stack size and durability
		char iStackInfo = ItemInfoStorage::getMaxStackSize(iItemID);
		short iUsageInfo = ItemInfoStorage::getDurability(iItemID);

		//check stack size and write
		if (StackSize > iStackInfo) { //Stack too big
			_iStackSize = iStackInfo;
			std::cout<<"ItemSlot::readFromNetwork Stacksize exceeded. Affected:"<<iItemID<<"\n";
		}else{ //all right
			_iStackSize = StackSize;
		}

		//if item is a tool... damagebar instead of metadata
		if (iItemID > 255 && _fDamageable) { //Item has a damage bar
			_isTool=true;
			if (Usage >= iUsageInfo) {
				clear();
				return;
			}
			_Item = std::make_pair(iItemID,0);
		}else{
			if (Usage < 0 || Usage > 15) {
				std::cout<<"ItemSlot::readFromNetwork invalid metadata. Affected:"<<iItemID<<"\n";
				_iUsage = 0;
			}else{
				_iUsage=Usage;
			}
			_Item = std::make_pair(iItemID,(char)_iUsage);
		}

		removeUnnecessarySubID();
	}catch (Poco::RuntimeException) {
		std::cout<<"ItemSlot::ItemSlot(fetch) Item wasn't found.\n";
		clear();
		return;
	}
}

void ItemSlot::writeToNetwork(NetworkOut& Out) {
	if (isEmpty()) {
		Out.addShort(-1);
		return;
	}else{
		Out.addShort(_Item.first);
		Out.addByte(_iStackSize);

		if ( ItemInfoStorage::isDamageable(_Item)) {
			Out.addShort(_iUsage);
			Out.addShort(-1);
		}else{
			Out.addShort(_Item.second);
		}
	}
}

bool ItemSlot::operator == (ItemSlot& other) {
	if (_Item.first == other.getItem().first && _Item.second == other.getItem().second && _iStackSize == other.getStackSize() && _iUsage == other.getUsage()) {
		return true;
	}else{
		return false;
	}
}

bool ItemSlot::operator != (ItemSlot& other) {
	if (_Item.first == other.getItem().first && _Item.second == other.getItem().second && _iStackSize == other.getStackSize() && _iUsage == other.getUsage()) {
		return false;
	}else{
		return true;
	}
}

ItemID ItemSlot::removeUnnecessarySubID(ItemID ID) {
	try {
		switch (ItemInfoStorage::isBlock(ID)) {
		case true:
			return std::make_pair(ID.first,  
				(ItemInfoStorage::getBlock(ID.first).hasSubBlocks ? ID.second : 0)
				);
		case false:
			return std::make_pair(ID.first,  
				(ItemInfoStorage::getItem(ID.first).hasSubItems ? ID.second : 0)
				);
		}
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return std::make_pair(0,0);
}

void ItemSlot::removeUnnecessarySubID() {
	_Item = removeUnnecessarySubID(_Item);
}


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

#include "ItemSlot.h"
#include "ItemInfoStorage.h"
#include <Poco/Exception.h>
#include <iostream>
#include "NetworkIn.h"

ItemSlot::ItemSlot(): 
_vEnchantments(0)
{
	_iItemID = 0;
	_iStackSize = 0;
	_iUsage = 0;
	_isTool = false;
}

ItemSlot::ItemSlot(short id,char stacksize):
_vEnchantments(0)
{
	char iInfo;
	try{
		iInfo = ItemInfoStorage::getMaxStackSize(id);
	}catch (Poco::RuntimeException) {
		std::cout<<"ItemSlot::ItemSlot Item wasn't found. Affected:"<<id<<"\n";
		clear();
		return;
	}

	_iItemID = id;
	_iUsage = 0;

	//Check stack size
	if (stacksize<0){
		_iStackSize=0;
		std::cout<<"ItemSlot::ItemSlot Stacksize invalid. Affected:"<<id<<"\n";
	}
	if (stacksize > iInfo) {
		_iStackSize = iInfo;
		std::cout<<"ItemSlot::ItemSlot Stacksize exceeded. Affected:"<<id<<"\n";
	}else{
		_iStackSize = stacksize;
	}
	
	//Set isTool
	if (ItemInfoStorage::isDamageable(id)) {
		_isTool=true;
	}else{
		_isTool=false;
	}
}

ItemSlot::ItemSlot(short id,char stacksize,short usage) :
_vEnchantments(0)
{
	short iUsageInfo;
	char iStackInfo;

	try	{
		iStackInfo = ItemInfoStorage::getMaxStackSize(id);
		iUsageInfo = ItemInfoStorage::getMaxUsage(id);
	}catch (Poco::RuntimeException) {
		std::cout<<"ItemSlot::ItemSlot(ex) Item wasn't found. Affected:"<<id<<"\n";
		clear();
		return;
	}

	_iItemID = id;

	//Check stack size
	if (stacksize<0){
		_iStackSize=0;
		std::cout<<"ItemSlot::ItemSlot(ex) Stacksize invalid. Affected:"<<id<<"\n";
	}
	if (stacksize > iStackInfo) {
		_iStackSize = iStackInfo;
		std::cout<<"ItemSlot::ItemSlot(ex) Stacksize exceeded. Affected:"<<id<<"\n";
	}else{
		_iStackSize = stacksize;
	}

	//Check usage
	if (usage > iUsageInfo || usage < 0) {
		std::cout<<"ItemSlot::ItemSlot(ex) Usage exceeded/invalid. Affected:"<<id<<"\n";
		_iUsage = 0;
	}else{
		_iUsage = usage;
	}
	
	//Set isTool
	if (ItemInfoStorage::isDamageable(id)) {
		_isTool=true;
	}else{
		_isTool=false;
	}
}

ItemSlot::ItemSlot(NetworkIn& rNetwork) {
	try {
		readFromNetwork(rNetwork);
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
}

ItemSlot::~ItemSlot() {
	_vEnchantments.clear();
}

short ItemSlot::getItemID() {
	return _iItemID;
}

void ItemSlot::setItemID(short id) {
	try{
		if (ItemInfoStorage::isDamageable(id)) {
			_isTool=true;
		}else{
			_isTool=false;
		}
	}catch(Poco::RuntimeException) {
		std::cout<<"ItemSlot::setItemID Item wasn't found. Affected:"<<id<<"\n";
		throw Poco::RuntimeException("ID not registered");
	}
	_iItemID = id;
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

	SizeInfo = ItemInfoStorage::getMaxStackSize(_iItemID);

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
	short iUsageInfo = ItemInfoStorage::getMaxUsage(_iItemID);

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

	iUsageInfo = ItemInfoStorage::getMaxUsage(_iItemID);

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
	_iItemID = 0;
	_iStackSize = 0;
	_iUsage = 0;
	_isTool=false;
}

bool ItemSlot::isEmpty() {
	if (_iStackSize == 0) {
		return true;
	}else{
		return false;
	}
}

void ItemSlot::readFromNetwork(NetworkIn& rNetwork) {
	try {
	_iItemID = rNetwork.readShort();
	if (_iItemID == -1) { //Slot is empty
		clear();
		return;
	}

	_isTool=false;
	_iStackSize = rNetwork.readByte();
	_iUsage = rNetwork.readShort();

	short iUsageInfo;
	char iStackInfo;
	try	{
		iStackInfo = ItemInfoStorage::getMaxStackSize(_iItemID);
		iUsageInfo = ItemInfoStorage::getMaxUsage(_iItemID);
	}catch (Poco::RuntimeException) {
		std::cout<<"ItemSlot::ItemSlot(fetch) Item wasn't found. Affected:"<<_iItemID<<"\n";
		clear();
		return;
	}

	//Check values
	if (_iStackSize > iStackInfo) {
		_iStackSize = iStackInfo;
		std::cout<<"ItemSlot::ItemSlot(fetch) Stacksize exceeded. Affected:"<<_iItemID<<"\n";
	}
	if(_iUsage > iUsageInfo || _iUsage < 0) {
		_iUsage = 0;
	}

	//Check if item has a damage bar
	if (ItemInfoStorage::isDamageable(_iItemID)) {
		_isTool=true;
		rNetwork.readShort();
	}
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
}
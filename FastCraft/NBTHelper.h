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
#ifndef _FASTCRAFTHEADER_NBTHELPER
#define _FASTCRAFTHEADER_NBTHELPER
#include "NBTTagCompound.h"
#include "FCRuntimeException.h"

/* 
NBTType  = NBTag type of element
realType = Raw data type (NBTTagByte = char, NBTTagInt = int)
*/

template <typename NBTType,typename realType>
class NBTHelper {
public:
	/* 
	* Tries to find the element and sets the elements value to the given one 
	* If the element doesn't exist it will create a new one and set the value to the given one

	Parameter:
	@1 : Compound that contains the element
	@2 : Elements name
	@3 : Data source
	*/
	static void set(NBTTagCompound*,string,realType&);
	

	/* 
	* Tries to find the element and writes the elements contentent to the given variable
	* If the element doesn't exist it will create a new one and set the value to the given one

	Parameter:
	@1 : Compound that contains the element
	@2 : Elements name
	@3 : Target variable
	*/
	static void read(NBTTagCompound*,string,realType&);
};


template <typename NBTType,typename realType>
void NBTHelper<NBTType,realType>::read(NBTTagCompound* pCompound,string sName,realType& rData) {	
	if (!pCompound->has(sName)) {
		NBTType* pTag;
		pCompound->addSubElement(pTag = new NBTType(sName));
		pTag->getDataRef() = rData;
	}else{
		NBTType* pTag = dynamic_cast<NBTType*>(pCompound->getElementByName(sName));
		if (pTag == NULL) {throw FCRuntimeException("Illegal cast");}
		rData = pTag->getDataRef();
	}
}

template <typename NBTType,typename realType>
void NBTHelper<NBTType,realType>::set(NBTTagCompound* pCompound,string sName,realType& rData) {	
	NBTType* pTag;
	
	if (!pCompound->has(sName)) {
		pCompound->addSubElement(pTag = new NBTType(sName));
	}else{
		pTag = dynamic_cast<NBTType*>(pCompound->getElementByName(sName));
		if (pTag == NULL) {throw FCRuntimeException("Illegal cast");}
	}


	pTag->getDataRef() = rData;
}
#endif
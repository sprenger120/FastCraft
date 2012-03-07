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
#include "NBTAll.h"
#include <sstream>
#include <Poco/InflatingStream.h>
#include <Poco/Exception.h>
#include <Poco/ByteOrder.h>
#include <stack>
#include <cstring>
#include <iostream>
using std::cout;

NBTBinaryParser::NBTBinaryParser(){
}

NBTBinaryParser::~NBTBinaryParser(){
}


NBTTagCompound* NBTBinaryParser::parse(string& rStr, bool fType) {
	std::stringstream ssOutput;
	if (fType == FC_NBT_INPUT_GZIP) {
		Poco::InflatingOutputStream inflator(ssOutput,Poco::InflatingStreamBuf::STREAM_GZIP);
		inflator.write(rStr.c_str(),rStr.length());
		inflator.flush();
		inflator.clear();
		inflator.close();
	}
	string& rTarget = (fType == FC_NBT_INPUT_GZIP ? ssOutput.str() : rStr);
	if (rTarget[0] != 0xA) { throw Poco::RuntimeException("Start compound not found!");}

	NBTTagCompound *pRootCompound;
	stack<NBTTagBase*> storageStack;
	string sName("");
	int byteIndex = 1;

	try {
		readName(rTarget,byteIndex,sName);
		pRootCompound = new NBTTagCompound(sName);
		storageStack.push(pRootCompound);

		while (1) {
			if (byteIndex+1 > rTarget.length()-1) {throw Poco::RuntimeException("End of file!");}
			nextElement(rTarget,storageStack,byteIndex,rTarget[byteIndex+1]);
			if (storageStack.empty()) {return pRootCompound;}
		}
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return NULL;
}


void NBTBinaryParser::nextElement(string& rTarget,stack<NBTTagBase*>& StorageStack,int& byteIndex,char iType,bool fParseHeader) {
	NBTTagBase* pLastStorage = StorageStack.top();


	//cout<<"stack size:"<<StorageStack.size()<<"\n";
	/*for (int x=1;x<=StorageStack.size()-1;x++) {
		cout<<"\t";
	}*/
	try {
		switch(iType) {
		case FC_NBT_TYPE_BYTE:
			handleByte(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_SHORT:
			handleShort(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_INT:
			handleInt(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_INT64:
			handleInt64(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_FLOAT:
			handleFloat(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_DOUBLE:
			handleDouble(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_BYTEARRAY:
			handleByteArray(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_STRING:
			handleString(rTarget,byteIndex,pLastStorage,fParseHeader);
			break;
		case FC_NBT_TYPE_LIST:
			handleList(rTarget,byteIndex,pLastStorage,StorageStack,fParseHeader);
			break;
		case FC_NBT_TYPE_COMPOUND:
			pLastStorage = handleCompound(rTarget,byteIndex,pLastStorage,fParseHeader);
			StorageStack.push(pLastStorage);
			break;
		case 0: //Compound end tag
			//cout<<"compound end "<<StorageStack.top()->getName()<<"\n";
			if (StorageStack.top()->getTagType() != FC_NBT_TYPE_COMPOUND) {throw Poco::RuntimeException("Not a compound");}
			StorageStack.pop();
			byteIndex++;
			if (StorageStack.empty()) {return;  /*End reached */ } 
			break;
		default:
			throw Poco::RuntimeException("Unknown TAG");
		}
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
}


void NBTBinaryParser::readName(string& rSource,int& iByteIndex,string& rTarget) {
	if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	short iLen =  ((short(rSource[iByteIndex]) & 0x00FF)<<8) |
		(short(rSource[iByteIndex+1]) & 0x00FF);

	if (iLen < 0) {throw Poco::RuntimeException("Illegal lengh field");}

	iByteIndex += 2;
	if (iByteIndex + iLen - 1 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	rTarget.assign(rSource,iByteIndex,iLen);
	iByteIndex += iLen-1; //Move index to last name char
}

void NBTBinaryParser::handleByte(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagByte* pElement;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagByte(sName);
		//cout<<"byte "<<sName<<"\n";
	}else{
		pElement = new NBTTagByte("");
		//cout<<"byte (nameless)\n";
	}


	if (iByteIndex + 1 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++; //Move to data start
	pElement->getDataRef() = rSource[iByteIndex];


	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

void NBTBinaryParser::handleShort(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagShort* pElement;
	NBTU_Short unionShort;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagShort(sName);
		//cout<<"short "<<sName<<"\n";
	}else{
		pElement = new NBTTagShort("");
		//cout<<"short (nameless)\n";
	}

	
	if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++; //Move to data start
	memcpy(unionShort.sData,&rSource[iByteIndex],2);
	pElement->getDataRef() = Poco::ByteOrder::flipBytes(unionShort.iData);
	iByteIndex++; //Move to data end


	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

void NBTBinaryParser::handleInt(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagInt* pElement;
	NBTU_Int unionInt;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagInt(sName);
		//cout<<"int "<<sName<<"\n";
	}else{
		pElement = new NBTTagInt("");
		//cout<<"int (nameless)\n";
	}


	if (iByteIndex + 4 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++;//Move to data start
	memcpy(unionInt.sData,&rSource[iByteIndex],4);
	pElement->getDataRef() = Poco::ByteOrder::flipBytes(unionInt.iData);
	iByteIndex += 3;//Move to data end


	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

void NBTBinaryParser::handleInt64(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagInt64* pElement;
	NBTU_Int64 unionInt64;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagInt64(sName);
		//cout<<"int64 "<<sName<<"\n";
	}else{
		pElement = new NBTTagInt64("");
		//cout<<"int64 (nameless)\n";
	}


	if (iByteIndex + 8 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++; //Move to data start
	memcpy(unionInt64.sData,&rSource[iByteIndex],8);
	pElement->getDataRef() = Poco::ByteOrder::flipBytes(unionInt64.iData);
	iByteIndex += 7;//Move to data end


	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

void NBTBinaryParser::handleFloat(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagFloat* pElement;
	NBTU_Int unionInt;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagFloat(sName);
		//cout<<"float "<<sName<<"\n";
	}else{
		pElement = new NBTTagFloat("");
		//cout<<"float (nameless)\n";
	}


	if (iByteIndex + 4 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++; //Move to data start
	memcpy(unionInt.sData,&rSource[iByteIndex],4);
	unionInt.iData = Poco::ByteOrder::flipBytes(unionInt.iData);
	pElement->getDataRef() = unionInt.nData;
	iByteIndex += 3;//Move to data end


	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

void NBTBinaryParser::handleDouble(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagDouble* pElement;
	NBTU_Int64 unionInt64;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagDouble(sName);
		//cout<<"double "<<sName<<"\n";
	}else{
		pElement = new NBTTagDouble("");
		//cout<<"double (nameless)\n";
	}

	if (iByteIndex + 8 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++;
	memcpy(unionInt64.sData,&rSource[iByteIndex],8);
	unionInt64.iData = Poco::ByteOrder::flipBytes(unionInt64.iData);
	pElement->getDataRef() = unionInt64.nData;

	iByteIndex += 7;//Move to data end

	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

void NBTBinaryParser::handleString(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagString* pElement;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagString(sName);
		//cout<<"string "<<sName<<"\n";
	}else{
		pElement = new NBTTagString("");
		//cout<<"string (nameless)\n";
	}
	iByteIndex++; //Move to data start
	readName(rSource,iByteIndex,pElement->getDataRef());

	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

void NBTBinaryParser::handleByteArray(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagByteArray* pElement;
	NBTU_Int unionInt;
	char* pData;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);

		pElement = new NBTTagByteArray(sName);
		//cout<<"byteArray "<<sName<<"\n";
	}else{
		pElement = new NBTTagByteArray("");
		//cout<<"byteArray (nameless)\n";
	}
	if (iByteIndex + 4 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++; //Move to data start
	memcpy(unionInt.sData,&rSource[iByteIndex],4);
	unionInt.iData = Poco::ByteOrder::flipBytes(unionInt.iData);
	iByteIndex += 3;//Move to data end

	if (unionInt.iData < 0) {
		throw Poco::RuntimeException("Illegal ByteArray length field");
	}

	if (unionInt.iData > 0) {
		if (iByteIndex + unionInt.iData-1 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		pData = new char[unionInt.iData];
		iByteIndex++; //Move to data start
		memcpy(pData,&rSource[iByteIndex],unionInt.iData);
		pElement->getDataRef().assign(pData,unionInt.iData);
		delete [] pData;
		iByteIndex += unionInt.iData-1;
	}


	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

NBTTagCompound* NBTBinaryParser::handleCompound(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,bool fParseHeader) {
	string sName("");
	NBTTagCompound* pElement;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);
		pElement = new NBTTagCompound(sName);
		//cout<<"compound "<<sName<<"\n";
	}else{
		pElement = new NBTTagCompound("");
		//cout<<"compound (nameless)\n";
	}
	

	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
	return pElement;
}

void NBTBinaryParser::handleList(string& rSource,int& iByteIndex,NBTTagBase* pLastStorage,stack<NBTTagBase*>& storageStack,bool fParseHeader) {
	string sName("");
	char iType;
	NBTU_Int unionInt;
	NBTTagList* pElement;

	if (fParseHeader) {
		if (iByteIndex + 2 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
		iByteIndex+=2; //Move to tag type byte & skip it
		readName(rSource,iByteIndex,sName);
	}

	if (iByteIndex + 1 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++; 
	iType = rSource[iByteIndex];
	
	if (iByteIndex + 4 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
	iByteIndex++;
	 memcpy(unionInt.sData,&rSource[iByteIndex],4);
	unionInt.iData = Poco::ByteOrder::flipBytes(unionInt.iData);
	iByteIndex += 3;

	if (iType < 1 || iType > 10) {throw Poco::RuntimeException("Unknown tag type");}
	if (unionInt.iData < 0) {throw Poco::RuntimeException("Invalid count field");}

	if (fParseHeader) {
		pElement = new NBTTagList(sName,iType);
		//cout<<"list "<<sName<<"\n";
	}else{
		pElement = new NBTTagList("",iType);
		//cout<<"list (nameless)\n";
	}

	if (unionInt.iData > 0) {
		storageStack.push(pElement);
		while(unionInt.iData != 0){
			if (iType == FC_NBT_TYPE_COMPOUND) {
				NBTTagCompound* pComp = new NBTTagCompound("");
				storageStack.push(pComp);
				pElement->addSubElement(pComp);
			}else {
				nextElement(rSource,storageStack,iByteIndex,iType,false);
			}

			while (storageStack.top() != pElement){
				if (iByteIndex+1 > rSource.length()-1) {throw Poco::RuntimeException("End of file!");}
				nextElement(rSource,storageStack,iByteIndex,rSource[iByteIndex+1]);
			}
			
			unionInt.iData--;
		}
		storageStack.pop();
	}


	switch(pLastStorage->getTagType()) {
	case FC_NBT_TYPE_LIST:
		{
			NBTTagList* p = (NBTTagList*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	case FC_NBT_TYPE_COMPOUND:
		{
			NBTTagCompound* p = (NBTTagCompound*)pLastStorage;
			p->addSubElement(pElement);
		}
		break;
	default:
		throw Poco::RuntimeException("Illegal storage");
	}
}

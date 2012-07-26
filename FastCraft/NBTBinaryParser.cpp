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
#include "FCRuntimeException.h"
#include <Poco/ByteOrder.h>
#include <stack>
#include <cstring>
#include <string>
#include <iostream>
using std::cout;


NBTTagCompound* NBTBinaryParser::parse(string& rString, char iInputType) {
	try {
		return parse((char*)rString.c_str(),iInputType,rString.length());
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	return NULL; /* Compiler warning fix */
}

NBTTagCompound* NBTBinaryParser::parse(char* pStr, char iInputType,int iLen) {
	Poco::InflatingStreamBuf::StreamType type;

	switch (iInputType) {
	case FC_NBT_IO_ZLIB:
		type = Poco::InflatingStreamBuf::STREAM_ZLIB;
		break;
	case FC_NBT_IO_GZIP:
		type = Poco::InflatingStreamBuf::STREAM_GZIP;
		break;
	case FC_NBT_IO_RAW:
		break;
	default:
		throw FCRuntimeException("Unknown compression type");
	}

	/* Decompress */
	if (iInputType != FC_NBT_IO_RAW) {
		std::stringstream ssOutput;
		Poco::InflatingOutputStream inflator(ssOutput,type);
		inflator.write(pStr,iLen);
		inflator.flush();
		inflator.clear();
		inflator.close();

		_iSize = ssOutput.str().length();
		if (_iSize <= 0) {FCRuntimeException("Unable to extract data");}

		pStr = new char[_iSize];
		memcpy(pStr,ssOutput.str().c_str(),_iSize);
	}else{
		if (iLen <= 0) {FCRuntimeException("Illegal lenght");}
		_iSize = iLen;
	}

	/* First check */
	if (pStr[0] != 0xA) { throw FCRuntimeException("Start compound not found!");}


	/* Init local variables */
	_pStr = pStr;
	_byteIndex = 0;
	while(!_storageStack.empty()){_storageStack.pop();}

	NBTTagCompound *pRootCompound;
	try {
		pRootCompound = new NBTTagCompound(readName());
		_storageStack.push(pRootCompound);

		while (1) {
			if (_storageStack.empty()) {
				if (iInputType != FC_NBT_IO_RAW) {delete [] pStr;}
				return pRootCompound;
			}

			if (_byteIndex+1 > _iSize-1) {throw FCRuntimeException("End of file!");}
			_byteIndex++;
			nextElement(pStr[_byteIndex]);
		}
	}catch(FCRuntimeException& ex) {
		if (iInputType != FC_NBT_IO_RAW) {delete [] pStr;}
		delete pRootCompound;
		ex.rethrow();
	}
	return NULL;
}


void NBTBinaryParser::nextElement(char iType,bool fParseHeader) {
	if (_storageStack.empty()) {return;}
	NBTTagBase* pLastStorage = _storageStack.top();

	/*cout<<"stack size:"<<StorageStack.size()<<"\n";
	for (int x=1;x<=StorageStack.size()-1;x++) {
	cout<<"\t";
	}*/

	/*cout<<"nextElement:"<<int(iType)<<"\n";*/

	try {
		switch(iType) {
		case FC_NBT_TYPE_BYTE:
			handleByte(fParseHeader);
			break;
		case FC_NBT_TYPE_SHORT:
			handleShort(fParseHeader);
			break;
		case FC_NBT_TYPE_INT:
			handleInt(fParseHeader);
			break;
		case FC_NBT_TYPE_INT64:
			handleInt64(fParseHeader);
			break;
		case FC_NBT_TYPE_FLOAT:
			handleFloat(fParseHeader);
			break;
		case FC_NBT_TYPE_DOUBLE:
			handleDouble(fParseHeader);
			break;
		case FC_NBT_TYPE_BYTEARRAY:
			handleByteArray(fParseHeader);
			break;
		case FC_NBT_TYPE_STRING:
			handleString(fParseHeader);
			break;
		case FC_NBT_TYPE_LIST:
			handleList(fParseHeader);
			break;
		case FC_NBT_TYPE_COMPOUND:
			handleCompound(fParseHeader);
			break;
		case FC_NBT_TYPE_INTARRAY:
			handleIntArray(fParseHeader);
			break;
		case 0: //Compound end tag
			if (_storageStack.top()->getTagType() != FC_NBT_TYPE_COMPOUND) {throw FCRuntimeException("Not a compound");}
			_storageStack.pop();

			//for(int x=0;x<=3;x++) {
			//	cout<<(int(_pStr[_byteIndex+x])&0xf)<<"\n";
			//}

			break;
		default:
			cout<<"Unknown NBT Tag:"<<(int(iType)&0xFF);
			for(int x=-10;x<=10;x++) {
				cout<<"dump ["<<x<<"/"<<_byteIndex+x<<"] :"<<(int(_pStr[_byteIndex+x])&0xff)<<"\n";
			}
			throw FCRuntimeException("Unknown TAG");
		}
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}


string NBTBinaryParser::readName() {
	if (_byteIndex + 2 > _iSize-1) {throw FCRuntimeException("End of file!");}
	_byteIndex++;
	short iLen =  ((short(_pStr[_byteIndex]) & 0x00FF)<<8) |
		(short(_pStr[_byteIndex+1]) & 0x00FF);
	
	if (iLen < 0) {throw FCRuntimeException("Illegal lengh field");}
	_byteIndex++;
	if (iLen == 0) {return string("");}
	

	if (_byteIndex + iLen > _iSize-1) {throw FCRuntimeException("End of file!");}
	_byteIndex++;

	string sData;
	sData.assign(&_pStr[_byteIndex],iLen); 

	_byteIndex += iLen -1; //Move index to last name char
	return sData;
}

int NBTBinaryParser::readInt() {
	if (_byteIndex + 4 > _iSize-1) {throw FCRuntimeException("End of file!");}
	
	_byteIndex++;
	memcpy(_unionInt.sData,&_pStr[_byteIndex],4);
	_byteIndex += 3;//Move to data end

	return Poco::ByteOrder::flipBytes(Poco::Int32(_unionInt.iData));
}

void NBTBinaryParser::handleByte(bool fParseHeader) {
	NBTTagByte* pElement = NULL;
	try {
		if (fParseHeader) {
			pElement = new NBTTagByte(readName());
		}else{
			pElement = new NBTTagByte("");
		}

		if (_byteIndex + 1 > _iSize-1) {throw FCRuntimeException("End of file!");}
		_byteIndex++; //Move to data start
		pElement->getDataRef() = _pStr[_byteIndex];


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

void NBTBinaryParser::handleShort(bool fParseHeader) {
	NBTTagShort* pElement = NULL;
	try {
		if (fParseHeader) {
			pElement = new NBTTagShort(readName());
		}else{
			pElement = new NBTTagShort("");
		}


		if (_byteIndex + 2 > _iSize-1) {throw FCRuntimeException("End of file!");}
		_byteIndex++; //Move to data start
		memcpy(_unionShort.sData,&_pStr[_byteIndex],2);
		pElement->getDataRef() = Poco::ByteOrder::flipBytes(Poco::Int16(_unionShort.iData));
		_byteIndex++; //Move to data end


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

void NBTBinaryParser::handleInt(bool fParseHeader) {
	NBTTagInt* pElement = NULL;
	try {
		if (fParseHeader) {
			pElement = new NBTTagInt(readName());
		}else{
			pElement = new NBTTagInt("");
		}
		
		pElement->getDataRef() = readInt();
		
		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

void NBTBinaryParser::handleInt64(bool fParseHeader) {
	NBTTagInt64* pElement = NULL;
	try {
		if (fParseHeader) {
			pElement = new NBTTagInt64(readName());
		}else{
			pElement = new NBTTagInt64("");
		}


		if (_byteIndex + 8 > _iSize-1) {throw FCRuntimeException("End of file!");}
		_byteIndex++; //Move to data start
		memcpy(_unionInt64.sData,&_pStr[_byteIndex],8);
		pElement->getDataRef() = Poco::ByteOrder::flipBytes(Poco::Int64(_unionInt64.iData));
		_byteIndex += 7;//Move to data end


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

void NBTBinaryParser::handleFloat(bool fParseHeader) {
	NBTTagFloat* pElement = NULL;
	try {
		if (fParseHeader) {
			pElement = new NBTTagFloat(readName());
		}else{
			pElement = new NBTTagFloat("");
		}


		if (_byteIndex + 4 > _iSize-1) {throw FCRuntimeException("End of file!");}
		_byteIndex++; //Move to data start

		memcpy(_unionInt.sData,&_pStr[_byteIndex],4);
		_unionInt.iData = Poco::ByteOrder::flipBytes(Poco::Int32(_unionInt.iData));
		pElement->getDataRef() = _unionInt.nData;

		_byteIndex += 3;//Move to data end


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

void NBTBinaryParser::handleDouble(bool fParseHeader) {
	NBTTagDouble* pElement = NULL;
	try {
		if (fParseHeader) {
			pElement = new NBTTagDouble(readName());
		}else{
			pElement = new NBTTagDouble("");
		}

		if (_byteIndex + 8 > _iSize-1) {throw FCRuntimeException("End of file!");}
		_byteIndex++;
		memcpy(_unionInt64.sData,&_pStr[_byteIndex],8);
		_unionInt64.iData = Poco::ByteOrder::flipBytes(Poco::Int64(_unionInt64.iData));
		pElement->getDataRef() = _unionInt64.nData;

		_byteIndex += 7;//Move to data end


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

void NBTBinaryParser::handleString(bool fParseHeader) {
	NBTTagString* pElement = NULL;
	try {
		if (fParseHeader) {
			pElement = new NBTTagString(readName());
		}else{
			pElement = new NBTTagString("");
		}

		pElement->getDataRef() = readName();

		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

void NBTBinaryParser::handleByteArray(bool fParseHeader) {
	NBTTagByteArray* pElement = NULL;
	try {
		char* pData;
		string sName("");

		if (fParseHeader) {
			sName = readName();
		}else{
			_byteIndex++;
		}

		int iLen = readInt();
		if (iLen <= 0) {throw FCRuntimeException("Illegal ByteArray length field");}
		if (_byteIndex + iLen > _iSize-1) {throw FCRuntimeException("End of file!");}


		pData = new char[iLen];
		_byteIndex++; //Move to data start

		memcpy(pData,&_pStr[_byteIndex],iLen);
		pElement = new NBTTagByteArray(sName,pData,iLen);

		_byteIndex += iLen-1;


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}

NBTTagCompound* NBTBinaryParser::handleCompound(bool fParseHeader) {
	NBTTagCompound* pElement = NULL;
	try {
		string sName("");


		if (fParseHeader) {
			pElement = new NBTTagCompound(readName());
		}else{
			pElement = new NBTTagCompound("");
		}
		

		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}

		_storageStack.push(pElement);

		return pElement;
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
	return NULL; /* Compiler warning fix */
}

void NBTBinaryParser::handleList(bool fParseHeader) {
	NBTTagList* pElement = NULL;
	try {
		string sName("");

		if (fParseHeader) {sName = readName();}

		if (_byteIndex + 5 > _iSize-1) {throw FCRuntimeException("End of file!");}

		_byteIndex++; 
		char iType = _pStr[_byteIndex];
		if (iType < 1 || iType > 11) {throw FCRuntimeException("Unknown tag type");}

		int iSize = readInt();
		if (iSize < 0) {throw FCRuntimeException("Invalid count field");}

		if (fParseHeader) {
			pElement = new NBTTagList(sName,iType);
		}else{
			pElement = new NBTTagList("",iType);
		}
		_storageStack.push(pElement);

		/*cout<<"Listy list: "<<int(iType)<<"  "<<sName<<"    "<<iSize<<"\n";*/

		while(iSize > 0){
			nextElement(iType,false);	

			while (_storageStack.top() != pElement){
				if (_byteIndex + 1 > _iSize-1) {throw FCRuntimeException("End of file!");}
				_byteIndex++;

				/*cout<<"other topstack element: "<<int(_pStr[_byteIndex+1])<<"\n";*/

				nextElement(_pStr[_byteIndex]);
			}

			if (_storageStack.empty()) {throw FCRuntimeException("Unexpected end");}
			iSize--;
		}
		_storageStack.pop();


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}


void NBTBinaryParser::handleIntArray(bool fParseHeader) {
	NBTTagIntArray* pElement = NULL; 
	try {
		if (fParseHeader) {
			pElement = new NBTTagIntArray(readName());
		}else{
			pElement = new NBTTagIntArray("");
		}

		int iArraySize = readInt();

		while (iArraySize > 0) {
			pElement->addSubElement(readInt());
			iArraySize--;
		}


		NBTTagBase* pLastStorage = _storageStack.top();
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
			throw FCRuntimeException("Illegal storage");
		}
	}catch(FCRuntimeException& ex) {
		if (pElement) {delete pElement;}
		ex.rethrow();
	}
}
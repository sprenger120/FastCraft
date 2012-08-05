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
#include "IOFile.h"
#include <Poco\File.h>
#include "FCRuntimeException.h"
#include <Poco\ScopedLock.h>

IOFile::IOFile(Poco::File& rFile){
	if(rFile.exists()) {
		if (!rFile.canRead()) {throw FCRuntimeException("Permission denied (read)");}
		if (!rFile.canWrite()) {throw FCRuntimeException("Permission denied (write)");}
	}
	_stream.open(rFile.path(),fstream::in | fstream::out | fstream::ate | fstream::binary);
	if (_stream.fail()) {
		_stream.open(rFile.path(),fstream::in | fstream::out | fstream::trunc | fstream::binary);
		if (_stream.fail()) {throw FCRuntimeException("Unable to open file");}
	}

	_iSize = (int)_stream.tellg();
	if (_iSize == -1) {_iSize = 0;}

	setPosition(0);
}

IOFile::~IOFile() {
	_stream.close();
}

void IOFile::write(char* pData,int iSize) {
	if (iSize <= 0) {throw FCRuntimeException("Illegal size");}
	if (pData == NULL) {throw FCRuntimeException("Nullpointer");}

	_iSize += iSize;
	_stream.write(pData,iSize);
	_stream.flush();
}

bool IOFile::read(char* pData,int iSize) {
	if (iSize <= 0) {throw FCRuntimeException("Illegal size");}
	if (pData == NULL) {throw FCRuntimeException("Nullpointer");}
	
	_stream.read(pData,iSize);

	if (_stream.eof()) {return false;}
	return true;
}

bool IOFile::setPosition(int iPos) {
	if (iPos > _iSize) {return false;}

	_stream.seekg(iPos,std::ios::beg);
	_stream.seekp(iPos,std::ios::beg);

	return true;
}

int IOFile::getSize() {
	return _iSize;
}

void IOFile::createDir(Poco::Path& path) {
	try {
		Poco::File fileInstance(path);
		if (!fileInstance.exists()) {fileInstance.createDirectories();}
	}catch(...) {
		throw FCRuntimeException("Unable to create directory");
	}
}
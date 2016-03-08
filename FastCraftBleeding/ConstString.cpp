#include "ConstString.h"
#include <cstring>
#include "FCException.h"

ConstString::ConstString(const char* pData) {
	if (pData == NULL) {throw NullpointerException();}

	_pAllocString = NULL;
	_pConstantString = pData;

	_iLen = strlen(pData);
}

ConstString::ConstString(char* pData) {
	if (pData == NULL) {throw NullpointerException();}

	_pAllocString = pData;
	_pConstantString = NULL;

	_iLen = strlen(pData);
}

ConstString::ConstString(std::string& rStr) {
	_pConstantString = NULL;
	if (rStr.empty()) {
		_iLen = 0;
		_pAllocString = NULL;
	}else{
		_iLen = rStr.length();
		_pAllocString = new char[_iLen+1];
		_pAllocString[_iLen] = 0;
		memcpy(_pAllocString,rStr.c_str(),_iLen);
	}
}

ConstString::ConstString(std::istream& rStrm,size_t iLen) {
	_pConstantString = NULL;
	_iLen = 0;
	_pAllocString = NULL;

	if (rStrm.eof()) {throw EndOfFileException();}
	if (iLen <= 0) {throw IllegalLengthException();}

	_iLen = iLen;
	_pAllocString = new char[iLen+1];
	_pAllocString[iLen] = 0;
	
	
	size_t pos,strmLen;
	pos = rStrm.tellg();
	rStrm.seekg(0,std::ios::end);
	strmLen = size_t(rStrm.tellg()) - pos;
	rStrm.seekg(pos);

	rStrm.read(_pAllocString,iLen);

	if (rStrm.eof()) {
		_iLen = strmLen;
		_pAllocString[_iLen] = 0;
	}
}


ConstString::~ConstString() {
	if (_pAllocString != NULL) {
		delete [] _pAllocString;
	}
}

void ConstString::operator=(ConstString& r) {
	_iLen = r._iLen;
	_pConstantString = r._pConstantString;


	if (r._pAllocString == NULL && _pAllocString != NULL)  {
		delete [] _pAllocString;
		_pAllocString = NULL;
		return;
	}

	if (_pAllocString != NULL) {delete [] _pAllocString;}
	_pAllocString = new char[_iLen+1];
	memcpy(_pAllocString,r._pAllocString,_iLen+1);
}

bool ConstString::operator<(const ConstString& r) const {
	if (_pConstantString == NULL && r._pConstantString == NULL &&
		_pAllocString == NULL    && r._pAllocString  == NULL) 
	{
		return false;
	}

	if (_pConstantString == NULL && r._pConstantString == NULL) {
		return strcmp(_pAllocString,r._pAllocString) > 0;
	}

	if(_pAllocString == NULL && r._pAllocString == NULL) {
		return strcmp(_pConstantString,r._pConstantString) > 0;
	}

	if (_pAllocString == NULL && r._pConstantString == NULL) {
		return strcmp(_pConstantString,r._pAllocString) > 0;
	}

	if (_pConstantString == NULL && r._pAllocString == NULL) {
		return strcmp(_pAllocString,r._pConstantString) > 0;
	}
	return false;
}

bool ConstString::operator==(const ConstString& r) const{
	if (_iLen != r._iLen) {
		return false;
	}else{
		if (_pConstantString == NULL && r._pConstantString == NULL &&
			_pAllocString == NULL    && r._pAllocString  == NULL) 
		{
			return true;
		}

		if (_pConstantString == NULL && r._pConstantString == NULL) {
			return strcmp(_pAllocString,r._pAllocString) == 0;
		}

		if(_pAllocString == NULL && r._pAllocString == NULL) {
			return strcmp(_pConstantString,r._pConstantString) == 0;
		}

		if (_pAllocString == NULL && r._pConstantString == NULL) {
			return strcmp(_pConstantString,r._pAllocString) == 0;
		}

		if (_pConstantString == NULL && r._pAllocString == NULL) {
			return strcmp(_pAllocString,r._pConstantString) == 0;
		}
	}
	return false;
}

bool ConstString::operator!=(const ConstString& r) const{
	return !((*this) == r);
}

size_t ConstString::getLen() const {
	return _iLen;
}

char* ConstString::getPtr() const {
	if (_pAllocString != NULL) {return _pAllocString;}
	if (_pConstantString != NULL) {return (char*)_pConstantString;}
	return NULL;
}

bool ConstString::isEmpty() const {
	if (_pConstantString == NULL && _pAllocString == NULL) {return true;}
	return false;
}
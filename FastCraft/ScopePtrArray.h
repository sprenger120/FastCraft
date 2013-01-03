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

#ifndef _FASTCRAFTHEADER_SCOPEPTRARRAY
#define _FASTCRAFTHEADER_SCOPEPTRARRAY
#include "FCException.h"

template <typename type>
class ScopePtrArray {
	type* _pData;
	int _iSize;
public:
	/*
	* Constructor
	* Throws FCException if element count is illegal 

	Parameter
	@1 : Element count
	*/
	ScopePtrArray(int);


	/*
	* Destructor
	*/
	~ScopePtrArray();


	/*
	* Get operator
	* Throws FCException if index is out of range
	*/ 
	type& operator[](int);


	/*
	* Returns internal pointer to array
	*/
	type* getPtr();
};


template <typename type>
ScopePtrArray<type>::ScopePtrArray(int iSize) {
	if (iSize <= 0) {throw FCException("Illegal size");}
	_pData = new type[iSize];
	_iSize = iSize;
}


template <typename type>
ScopePtrArray<type>::~ScopePtrArray() {
	delete [] _pData;
}


template <typename type>
type& ScopePtrArray<type>::operator[](int index) {
	if (index < 0 || index > _iSize -1) {throw FCException("Illegal index");}
	return _pData[index];
}

template <typename type>
type* ScopePtrArray<type>::getPtr() {
	return _pData;
}
#endif

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

#ifndef _FASTCRAFTHEADER_SCOPEPTR	
#define _FASTCRAFTHEADER_SCOPEPTR
#include "FCException.h"

template <typename type>
class ScopePtr {
	type* _pPtr;
public:
	/*
	* Constructor
	* "Takes care" of the given pointer (deletes it on destruction)
	* Throws FCException if pointer is NULL

	Parameter:
	@1 : Pointer
	*/
	ScopePtr(type*);


	/*
	* Constructor
	* Allocates a new instance of type
	*/
	ScopePtr();


	/*
	* Destructor
	*/
	~ScopePtr()


	/*
	* Operators
	*/
	type* operator->();
	type& operator*();
};



template <typename type>
ScopePtr<type>::ScopePtr(type* pData) {
	if (pData == NULL) {throw FCException("Nullpointer");}
	_pPtr = pData;
}

template <typename type>
ScopePtr<type>::ScopePtr() {
	if (pData == NULL) {throw FCException("Nullpointer");}
	_pPtr = new type;
}

template <typename type>
ScopePtr<type>::~ScopePtr() {
	delete _pPtr;
}


template <typename type>
type* ScopePtr<type>::operator->() {
	return _pPtr;
}


template <typename type>
type& ScopePtr<type>::operator*() {
	return *_pPtr;
}
#endif

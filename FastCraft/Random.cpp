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

#include "Random.h"
#include <random>


long long Random::Int64() {
	_uInt64 iInt = 0L;
	char iShift = rand();
	
        iInt += uInt64();
        
	if(iShift%2) {
		iInt <<= 48 + iShift%2; 
	}

	return _sInt64(iInt);
}

_uInt64 Random::uInt64() {
	_uInt64 iVal1=0L,iVal2=0L;

	iVal1 += rand();
	iVal2 += rand();

	if (rand()%2) {
		iVal2 = 0;
	}

	return  (iVal2<<32) | iVal1;
}

int Random::Int() {
	unsigned int iInt;
	char iShift = rand();
	
	iInt = uInt();
	

	if(iShift%2) {
		iInt <<= 16 + iShift%2; 
	}

	return int(iInt);
}

unsigned int Random::uInt() {
	return rand();
}

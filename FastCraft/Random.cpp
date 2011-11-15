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
	unsigned long long iInt;
	char iShift = rand();
	
	iInt = uInt64();
	
	if(iShift%2) {
		iInt <<= 48 + iShift%2; 
	}

	return signed long long(iInt);
}

unsigned long long Random::uInt64() {
	unsigned long long iRandVal=0,iVal1=0,iVal2=0;

	iVal1=0;
	iVal2=0;
	iVal1 += std::rand();
	iVal2 += std::rand();

	if (rand()%2) {
		iVal2 = 0;
	}

	return  (iVal2<<32) | iVal1;
}

signed int Random::Int() {
	unsigned int iInt;
	char iShift = rand();
	
	iInt = uInt();
	

	if(iShift%2) {
		iInt <<= 16 + iShift%2; 
	}

	return signed int(iInt);
}

unsigned int Random::uInt() {
	return rand();
}

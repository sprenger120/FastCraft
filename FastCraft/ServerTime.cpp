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
#include "ServerTime.h"
#include <ctime>

ServerTime::ServerTime() {
	_iLastTimestamp = 0;
	_iLastTimestamp += time(NULL);
	_iServerTime = 1500;
}

ServerTime::~ServerTime() {
}

long long ServerTime::getTime() {
	long long iDiff=0,iTimestamp=0;
	
	/*iTimestamp += time(NULL);

	if (iTimestamp < _iLastTimestamp) {
		std::cout<<"***INTERNAL SERVER WARNING: Time ran backwards! Ignore time increment.";
		return _iLastTimestamp;
	}

	iDiff = iTimestamp - _iLastTimestamp;

	_iServerTime += iDiff; //1 real second = 20 minecraft seconds

	*/
	return 10000L;
}
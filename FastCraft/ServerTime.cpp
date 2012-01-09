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
#include "ServerTime.h"
#include <ctime>
#include <sstream>
#include <Poco/Thread.h>

using Poco::Thread;

ServerTime::ServerTime() {
}

ServerTime::~ServerTime() {
}

long long ServerTime::_iServerTime = 24000*2;

void ServerTime::tick() {
	_iServerTime += 20;
}


string ServerTime::getTimeFormated() {
	int iDay,iHour,iMinute,iYear=0;
	long long iTicks;
	double dTicks=0.0;
	
	iTicks = _iServerTime;

	
	//Thx 2 Bukkit Essentials Team for this formula
	iTicks -= 18000 + 24000; 


	iDay = int(iTicks/24000);
	iTicks -= iDay*24000;
	iYear = iDay / 356;
	iDay = iDay%356;

	iHour = iDay%356;

	iHour = int(iTicks/1000);
	iTicks-= iHour*1000;

	dTicks += double(iTicks);

	iMinute = (int)(dTicks/16.66666);
	
	std::stringstream strStrm;

	strStrm<<(iHour<10 && iHour >= 0 ? "0" : "")
		   <<iHour<<":"
		   <<(iMinute<10 && iMinute >= 0 ? "0" : "")
		   <<iMinute;

	return strStrm.str();
}

long long ServerTime::getTime() {
	return _iServerTime;
}
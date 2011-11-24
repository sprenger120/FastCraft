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
#include <Poco/Thread.h>

using Poco::Thread;

ServerTime::ServerTime() {
}

ServerTime::~ServerTime() {
}

long long ServerTime::_iServerTime = 0;


void ServerTime::run() {
	while ( 1 ) {
		Thread::sleep(1000); //idle

		_iServerTime += 20;

	}
}



long long ServerTime::getTime() {
	return _iServerTime;
}
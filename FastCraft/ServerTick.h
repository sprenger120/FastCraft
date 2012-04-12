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

#ifndef _FASTCRAFTHEADER_SERVERTICK 
#define _FASTCRAFTHEADER_SERVERTICK
#include "MinecraftServer.h"

//This class is just a container for the ticks since the server startet
//Use the getTicks function from MinecraftServer to get the ticks since the fist configuration run

class ServerTick {
	static Tick _iTick;

	ServerTick();
	~ServerTick();
public:
	/*
	* Returns the ticks that have gone till the server has startet
	*/
	static Tick getTicks();


	/*
	* Increments the tick var
	*/
	static void doTick();
};
#endif
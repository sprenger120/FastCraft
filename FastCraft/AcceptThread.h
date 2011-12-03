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

#ifndef _FASTCRAFTHEADER_ACCEPTHREAD
#define _FASTCRAFTHEADER_ACCEPTHREAD

#include <iostream>
#include <Poco/Runnable.h>
#include <string>
#include "PlayerPool.h"

class SettingsHandler;

using std::string;

class AcceptThread : public Poco::Runnable {
private:
	short _iPort;
	string _sIP;
	string _ServerFullMsg;
	
	PlayerPool _PlayerPool;
public:
	//De- /constructor
	AcceptThread(SettingsHandler*);
	~AcceptThread();

	virtual void run(); //Thread Main
};

#endif
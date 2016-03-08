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

#ifndef _FASTCRAFTHEADER_SERVERTHREADBASE
#define _FASTCRAFTHEADER_SERVERTHREADBASE
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <string>

class ServerThreadBase : public Poco::Runnable {
	Poco::Thread _Thread;
	std::string _sName;
protected:
	char _iThreadStatus;
public:
	/* 
	* Constructor

	Parameter:
	@1 : Thread name
	*/
	ServerThreadBase(std::string);


	/*
	* Destructor
	* Stops the thread if it runs
	*/
	virtual ~ServerThreadBase();


	/*
	* Returns true if thread runs
	*/
	bool isRunning();

	
	/*
	* Starts the thread
	*/
	void startThread(Poco::Runnable*);


	/*
	* Forces the thread to exit
	*/
	void killThread();
};
#endif
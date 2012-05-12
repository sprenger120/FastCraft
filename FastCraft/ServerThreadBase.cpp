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

#include "ServerThreadBase.h"
#include "Constants.h"
#include <iostream>

ServerThreadBase::ServerThreadBase(std::string sName) : 
    _Thread(sName),
	_sName(sName)
{
	_iThreadStatus = FC_THREADSTATUS_DEAD;
}

ServerThreadBase::~ServerThreadBase(){
	if (_iThreadStatus == FC_THREADSTATUS_RUNNING) {
		std::cout<<"Thread: "<<_sName<<" wasn't shut down via ServerThreadBase::killThread!\n";
		killThread();
	}
}

bool ServerThreadBase::isRunning() {
	return (_iThreadStatus != FC_THREADSTATUS_DEAD);
}

void ServerThreadBase::startThread(Poco::Runnable* p){
	_Thread.start(*p);
	while(_iThreadStatus != FC_THREADSTATUS_RUNNING) {Poco::Thread::sleep(10);}
}

void ServerThreadBase::killThread() {
	_iThreadStatus = FC_THREADSTATUS_TERMINATING;
	while(_iThreadStatus == FC_THREADSTATUS_TERMINATING){
		Poco::Thread::sleep(50);
	}
	_iThreadStatus = FC_THREADSTATUS_DEAD;
	std::cout<<"Thread: "<<_sName<<" exited\n";
}
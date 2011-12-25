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
#include "NetworkOutRoot.h"
#include "Constants.h"
#include <iostream>

NetworkOutRoot::NetworkOutRoot() :
_lowQueue(),
_highQueue()
{
}

unsigned long long NetworkOutRoot::_iWriteTraffic = 0;

NetworkOutRoot::~NetworkOutRoot() {
	std::cout<<"Destroying NetworkOutRoot";
	_lowQueue.clear();
	_highQueue.clear();
}


NetworkOut NetworkOutRoot::New() {
	NetworkOut Out(this);
	return Out;
}

ThreadSafeQueue<string> & NetworkOutRoot::getLowQueue() {
	return _lowQueue;
}

ThreadSafeQueue<string> & NetworkOutRoot::getHighQueue() {
	return _highQueue;
}

void NetworkOutRoot::Add(char iType,string& rData) {
	switch(iType) {
	case FC_QUEUE_LOW:
		_iWriteTraffic+=rData.length();
		_lowQueue.push(rData);
		break;
	case FC_QUEUE_HIGH:
		_iWriteTraffic+=rData.length();
		_highQueue.push(rData);
		break;
	default:
		throw Poco::RuntimeException("Unknown queue type");
	}
}
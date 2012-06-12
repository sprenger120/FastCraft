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

#ifndef _FASTCRAFTHEADER_NETWORKOUTROOT
#define _FASTCRAFTHEADER_NETWORKOUTROOT
#include "ThreadSafeQueue.h"
#include "NetworkOut.h"
#include <string>

using std::string;

/*
* This class represents a root for the NetworkOut classes
* It holds the out queue
*/
class NetworkOutRoot {
private:
	ThreadSafeQueue<string> _lowQueue;
	ThreadSafeQueue<string> _highQueue;

	static unsigned long long _iWriteTraffic;
public:
	/*
	* Constructor
	*/
	NetworkOutRoot();


	/*
	* Destructor
	*/
	~NetworkOutRoot();


	/*
	* Returns a reference to the low queue
	*/
	ThreadSafeQueue<string> & getLowQueue();


	/*
	* Returns a reference to the high queue
	*/
	ThreadSafeQueue<string> & getHighQueue();


	/*
	* Adds something to the given queue
	* Will throw FCRuntimeException if Queue type is unknown

	Parameter:
	@1 : Queue type (FC_QUEUE_LOW / FC_QUEUE_HIGH)
	@2 : Reference to string that have to pushed to queue
	*/
	void Add(char,string&);


	/*
	* Returns write traffic in bytes 
	*/
	static unsigned long long getWriteTraffic();
};
#endif

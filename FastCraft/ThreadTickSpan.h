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

/*
* This class implements a easy way to check if a specific timespan is gone
*/

#ifndef _FASTCRAFTHEADER_THREADTICKSPAN
#define _FASTCRAFTHEADER_THREADTICKSPAN

class ThreadTickSpan {
private:
	long long* _pThreadTicks;
	long long _iTimespan;
	long long _iStart;
public:
	/*
	* Init a ThreadTickSpan instance

	Parameters:
	@1 : Pointer to variable which contains the actual thread ticks
	@2 : Timespan 
	*/
	ThreadTickSpan(long long*,long long);


	/*
	* Destructor
	*/
	~ThreadTickSpan();


	/*
	* Resets gone time
	*/
	void reset();


	/*
	* Checks whether time is gone
	*/
	bool isGone();


	/*
	* Returns alreay gone time 
	*/
	long long getGoneTime();
};
#endif
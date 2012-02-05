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
#include "ThreadTickSpan.h"
#include <iostream>

ThreadTickSpan::ThreadTickSpan(long long* p,long long iSpan) {
	_pThreadTicks = p;
	_iTimespan = iSpan;
	_iStart=*_pThreadTicks;
	if (iSpan<0) {std::cout<<"ThreadTickSpan::ThreadTickSpan iSpan lower than 0\n";}
	if ((*_pThreadTicks)<0) {std::cout<<"ThreadTickSpan::ThreadTickSpan actual thread ticks lower than 0\n";}
}

ThreadTickSpan::~ThreadTickSpan(){
}

void ThreadTickSpan::reset() {
	_iStart = *_pThreadTicks;
}

bool ThreadTickSpan::isGone() {
	return  getGoneTime() >= _iTimespan;
}

long long ThreadTickSpan::getGoneTime() {
	return  (*_pThreadTicks)-_iStart;
}

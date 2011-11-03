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
#ifndef _FASTCRAFTHEADER_TEXTHANDLER
#define _FASTCRAFTHEADER_TEXTHANDLER
#include <iostream>

class TextHandler {
public:
	static void packString16(std::string &, std::string &);
	
	//Functions to append numbers to a std string
	static void Append(std::string &,int,bool = false);
	static void Append(std::string &,signed long long,bool = false);
	static void Append(std::string &,double, bool = false);
	static void Append(std::string &,float, bool = false);
	static void Append(std::string &,short, bool = false);
};


#endif
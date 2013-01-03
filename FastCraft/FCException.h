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


#ifndef _FASTCRAFTHEADER_FCEXCEPTION
#define _FASTCRAFTHEADER_FCEXCEPTION
#include <iostream>
#include <string>
#if defined(_WIN32)
#include <Windows.h>
#include <DbgHelp.h>    
#elif defined(__linux__)
#include <execinfo.h>
#endif


using std::string;

class FCException {
private:
	string _sMessage;
	int _iCode;
public:
	/*
	* Constructor
	* The actual stacktrace will be written to console if the bool parameter is set to true

	Parameter:
	@1 : Description of the occurred error
	@2 : See above 

	Parameter:
	@1 : Description of the occurred error
	@2 : Error-Code
	@3 : See above
	*/
	FCException(string,bool = true);
	FCException(string,int,bool = true);


	/*
	* Destructor
	*/
	virtual ~FCException();


	/*
	* Returns the description of the occurred error
	*/
	char* what();
	string getMessage();


	/*
	* Returns the error code
	*/
	int getErrorCode();


	/*
	* Rethrows exception
	*/
	void rethrow();
private:
	void printStacktrace();
};



#define FC_DECLARE_EXCEPTION(NAME)		\
class NAME : public FCException	{		\
public:									\
	NAME(bool = true);					\
	NAME(string,bool = true);			\
	virtual ~NAME();					\
};													 

#define FC_IMPLEMENT_EXCEPTION(NAME)               \
NAME::NAME(std::string str,bool fPrintStacktrace) :\
	FCException(str,fPrintStacktrace)              \
{                                                  \
}                                                  \
NAME::NAME(bool fPrintStacktrace) :                \
FCException(std::string(""),fPrintStacktrace)	   \
{                                                  \
}                                                  \
NAME::~NAME()									   \
{                                                  \
}


FC_DECLARE_EXCEPTION(NBTIncompatibleTypeException)
FC_DECLARE_EXCEPTION(NBTNotFoundException)


#endif

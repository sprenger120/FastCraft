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


#ifndef _FASTCRAFTHEADER_FCRUNTIMEXCEPTION
#define _FASTCRAFTHEADER_FCRUNTIMEXCEPTION
#if defined(_WIN32)
    #include <Windows.h>
    #include <DbgHelp.h>
#elif defined(__linux__)
    #include <execinfo.h>
#endif
#include <iostream>
#include <string>
using std::string;

class FCRuntimeException {
private:
	string _sMessage;
	int _iCode;
public:
	/*
	* Constructor
	* The actual stacktrace will be written to console

	Parameter:
	@1 : Description of the occurred error

	Parameter:
	@1 : Description of the occurred error
	@2 : Error-Code
	*/
	FCRuntimeException();
	FCRuntimeException(string);
	FCRuntimeException(string,int);


	/*
	* Destructor
	*/
	virtual ~FCRuntimeException();


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
#endif
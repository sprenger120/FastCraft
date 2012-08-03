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
#include "FCRuntimeException.h"


FCRuntimeException::FCRuntimeException(string sDesc,bool fStackTrace) : 
_sMessage(sDesc)
{
	if (fStackTrace){printStacktrace();}
	_iCode = 0;
}

FCRuntimeException::FCRuntimeException(string sDesc,int i,bool fStackTrace) : 
_sMessage(sDesc)
{
	if (fStackTrace){printStacktrace();}
	_iCode = i;
}

FCRuntimeException::~FCRuntimeException() {
}

char* FCRuntimeException::what() {
	return (char*)_sMessage.c_str();
}

string FCRuntimeException::getMessage() {
	return _sMessage;
}

int FCRuntimeException::getErrorCode() {
	return _iCode;
}


void FCRuntimeException::printStacktrace() {
	std::cout<<"\nException threw: "<<_sMessage<<"\n";
    #if defined(_WIN32)
        short iCapuredFrames;
        void* stack[15];
        BOOL f;

        iCapuredFrames = CaptureStackBackTrace(0,15,stack,NULL);
		HANDLE process  = GetCurrentProcess();

        SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 255*sizeof(TCHAR));
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 254;

        for (short x=2;x<=iCapuredFrames-1;x++) {
            f = SymFromAddr(process,(DWORD64)stack[x],0,symbol);
			if(f){
				std::cout<<"\t["<<x<<"] "<<symbol->Name<<"\n";
			}else{
				std::cout<<"\t["<<x<<"] "<<"Unable to fetch symbol!\n";
			}
        }

        free(symbol);
    #elif defined(__linux__)
        void *array[15];
        size_t size;

        size = backtrace(array, 15);
        char** p = backtrace_symbols(array, size);

        for (int x=2;x<=size-1;x++) {
			std::cout<<"\t["<<x<<"] "<<p[x]<<"\n";		
		}
        delete p;
    #else
        cout<<"Not supported.\n";
    #endif
}

void FCRuntimeException::rethrow() {
	throw *(this);
}
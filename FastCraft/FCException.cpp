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
#include "FCException.h"
#if defined(__linux__)
    #include <cxxabi.h>
    #include <iostream>
    #include <cstring>  
    #include <cstdlib>
#endif
using std::string;

FCException::FCException(string sDesc, bool fStackTrace) :
_sDesc(sDesc) 
{
    if (fStackTrace) {printStacktrace();}
    _iCode = 0;
}

FCException::FCException(string sDesc, int i, bool fStackTrace) :
_sDesc(sDesc) 
{
    if (fStackTrace) {printStacktrace();}
    _iCode = i;
}

FCException::FCException(const char* sDesc,bool fStackTrace) : 
    _sDesc(sDesc)
{
    if (fStackTrace) {printStacktrace();}
    _iCode = 0;
}

FCException::~FCException() {
}

char* FCException::what() {
    return (char*) _sDesc.c_str();
}

string FCException::getMessage() {
    return _sDesc;
}

int FCException::getErrorCode() {
    return _iCode;
}

const char* FCException::name() {
    return "FCException";
}

void FCException::printStacktrace() {
    std::cout << "\nException threw: " << _sDesc << "\n";
#if defined(_WIN32)
    short iCapuredFrames;
    void* stack[15];
    BOOL f;

    iCapuredFrames = CaptureStackBackTrace(0, 15, stack, NULL);
    HANDLE process = GetCurrentProcess();

    SYMBOL_INFO* symbol = (SYMBOL_INFO*) malloc(sizeof (SYMBOL_INFO) + 255 * sizeof (TCHAR));
    symbol->SizeOfStruct = sizeof (SYMBOL_INFO);
    symbol->MaxNameLen = 254;

    for (short x = 2; x <= iCapuredFrames - 1; x++) {
        f = SymFromAddr(process, (DWORD64) stack[x], 0, symbol);
        if (f) {
            std::cout << "\t[" << x << "] " << symbol->Name << "\n";
        } else {
            std::cout << "\t[" << x << "] " << "Unable to fetch symbol!\n";
        }
    }

    free(symbol);
#elif defined(__linux__)
    void *array[15];
    size_t size;
    int status, end = 0;
    bool found = false;
    char * demangled, * prepared;

    size = backtrace(array, 15);
    char** p = backtrace_symbols(array, size);

    for (int x = 2; x <= size - 1; x++) {
        for (end = strlen(p[x]) - 1; end >= 0; end--) {
            //Remove +xx) [....]
            if (p[x][end] == '+') {
                p[x][end] = 0;
                found = true;
            }
            //Remove filepath
            if (p[x][end] == '(') {
                break;
            }
        }

        if (found) {
            prepared = (char*) (&(p[x][end]) + 1);
            demangled = abi::__cxa_demangle(prepared, 0, 0, &status);

            if (status == 0) {
                std::cout << "\t[" << x << "] " << demangled << "\n";
                free(demangled);
            } else {
                p[x][end] = '+';
                std::cout << "\t[" << x << "] " << prepared << "\t(status:" << status << ")" << "\n";
            }
            found = false;
        } else {
            std::cout << "\t[" << x << "] " << p[x] << "(no demangle)\n";
        }
    }
    free(p);
#else
    cout << "Backtracing is not supported.\n";
#endif
}

void FCException::rethrow() {
    throw *this;
}

//FastCraft
FC_IMPLEMENT_EXCEPTION(NBTIncompatibleTypeException,"Incompatible type")
FC_IMPLEMENT_EXCEPTION(NBTNotFoundException,"Not found")
FC_IMPLEMENT_EXCEPTION(NBTIllegalFormatException,"Illegal format");
FC_IMPLEMENT_EXCEPTION(NBTOverflowException,"Noo much elements");
FC_IMPLEMENT_EXCEPTION(NBTIllegalCompressionTypeException,"Illegal compression type");



FC_IMPLEMENT_EXCEPTION(FileNotFoundException,"File not found");
FC_IMPLEMENT_EXCEPTION(EndOfFileException,"File stream ended unexpected");
FC_IMPLEMENT_EXCEPTION(FileIOException,"File not read-/writeable")
FC_IMPLEMENT_EXCEPTION(NullpointerException,"Nullpointer");
FC_IMPLEMENT_EXCEPTION(IllegalLengthException,"Illegal length passed")

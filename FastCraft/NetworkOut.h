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

#ifndef _FASTCRAFTHEADER_NETWORKOUT
#define _FASTCRAFTHEADER_NETWORKOUT
#include <string>
#include "Structs.h"
#include <modes.h>
#include <aes.h>
#include <filters.h>
using std::string;


class NetworkOutRoot;

class NetworkOut {
private:
	IntToFloat _ItF;
	Int64ToDouble _ItD;

	char _sEndianBuffer[8];
	string* _pNetworkBuffer;

	NetworkOutRoot* _pMaster;
	CryptoPP::StreamTransformationFilter* _cfbEncryptor;
public:
	/* 
	* Constructor

	Parameter:
	@1 : pointer from NetworkOutRoot
	*/
	NetworkOut(NetworkOutRoot*);


	/*
	* Destructor
	*/
	~NetworkOut();


	/*
	* Static write members
	*/
	static void addByte(string&,char);
	static void addByte(string&,unsigned char);
	static void addBool(string&,bool);
	static void addShort(string&,short);
	static void addInt(string&,int);
	static void addInt64(string&,long long);
	static void addFloat(string&,float);
	static void addDouble(string&,double);
	static void addString(string&,string&);
	static void addByteArray(string&,std::pair<char*,short>&);


	/*
	* Explicit write members for this class
	*/
	void addByte(unsigned char);
	void addBool(bool);
	void addShort(short);
	void addInt(int);
	void addInt64(long long);
	void addFloat(float);
	void addDouble(double);
	void addString(string&);
	void addByteArray(std::pair<char*,short>&);


	/*
	* Returns a reference to the internal string buffer
	*/
	string& getStr();


	/*
	* Pushes data to network queue

	Parameter:
	@1 : Queue type (FC_QUEUE_LOW / FC_QUEUE_HIGH)
	*/
	void Finalize(char);
};
#endif
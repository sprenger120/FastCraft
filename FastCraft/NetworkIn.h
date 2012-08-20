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

#ifndef _FASTCRAFTHEADER_NETWORKIN
#define _FASTCRAFTHEADER_NETWORKIN
#include <Poco/Net/StreamSocket.h>
#include <string>
#include "Structs.h"
#include <utility>
#include <rsa.h>
#include <modes.h>
#include <aes.h>

using Poco::Net::StreamSocket;
using std::string;
using CryptoPP::RSAES;
using CryptoPP::PKCS1v15;
using CryptoPP::AES;

class MinecraftServer;
class PlayerThread;

class NetworkIn {
private:
	IntToFloat _ItF;
	Int64ToDouble _ItD;
	char _sReadBuffer[8];

	StreamSocket & _rSocket;
	MinecraftServer* _pMCServer;
	PlayerThread* _pPlayer;

	RSAES<PKCS1v15>::Decryptor _rsaDecryptor;
	CryptoPP::CFB_Mode<AES>::Decryption* _aesDecryptor;
	byte _IV[AES::BLOCKSIZE];
	CryptoPP::StreamTransformationFilter* _cfbDecryptor;

	bool _fCryptMode;
	string _sDecryptOutput;
public:
	/*
	* Constructor

	Parameter:
	@1 : Reference to a StreamSocket
	@2 : a MinecraftServer instance
	*/
	NetworkIn(StreamSocket&,MinecraftServer*,PlayerThread*);
	

	/*
	* Destructor
	*/
	~NetworkIn();

	/*
	* Read functions
	* Throw FCRuntimeException if 
	   - connection aborts
	   - client isn't connected
	*/
	char readByte(); 
	bool readBool();
	short readShort();
	int readInt();
	long long readInt64();
	float readFloat();
	double readDouble();
	string readString();
	/* Ignores the decrypt parameter if CryptMode is activated */
	std::pair<char*,short> readByteArray(bool = false); //Set to true to decrypt via AES


	/*
	* De-/activates encryption
	*/ 
	void setCryptMode(bool);
private:
	void read(int,char* pBuffer);
	void AESdecrypt(char*,int);
};
#endif

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

#include "PlayerThread.h"
#include "TextHandler.h"
#include "Constants.h"
#include <Poco/Timespan.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Net/NetException.h>
#include <sstream>
#include <istream>
#include <ctime>
#include "TCPHelper.h"

PlayerThread::PlayerThread(SettingsHandler* pSettingsHandler,EntityProvider* pEntityProvider,ServerTime* pServerTime) : 
	_sName(""),
	_sNickName(""),
	_sIP(""),
	_Connection(),
	_sTemp(""),
	_Web_Session("session.minecraft.net"),
	_Web_Response(),
	_sConnectionHash("")
{
	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;
	_Coordinates.OnGround = false;
	_Coordinates.Pitch = 0.0F;
	_Coordinates.Stance = 0.0;
	_Coordinates.X = 0.0;
	_Coordinates.Y = 0.0;
	_Coordinates.Yaw = 0.0F;
	_Coordinates.Z = 0.0;

	_iLoginProgress = 0;
	_iEntityID=0;
	_fAssigned = false;

	_iLastConnHash = InstanceCounter+10;

	_pSettings = pSettingsHandler;
	_pEntityProvider = pEntityProvider;
	_pServerTime = pServerTime;

	InstanceCounter++;
	_iThreadID = InstanceCounter;

	std::srand(time(NULL));
}

int PlayerThread::InstanceCounter = 0;
int PlayerThread::PlayerCount = 0;

void PlayerThread::ClearQueue() {
	for (int x = 1;x<=_SendQueue.size();x++) {
		_SendQueue.pop();
	}
}

PlayerThread::~PlayerThread() {
	ClearQueue();
	InstanceCounter--;
}


void PlayerThread::run() {
	int iTemp;

	while (1) {
		if (!isAssigned()) {
			Thread::sleep(50);
			continue;
		} 

		ProcessQueue();

		if (!isAssigned()) {continue;} //Connection was closed by queue processor 


		_sBuffer[0] = 240; //Set to a unused packet id  (used for connection aborting indentifying)

		try {
			_Connection.receiveBytes(_sBuffer,1);
		}catch(Poco::Net::InvalidSocketException) {
			Disconnect(true);
			continue;
		}catch(Poco::TimeoutException) {
			if (isNameSet()) { //Handshake isn't done
				cout<<"Player:"<<_sName<<" timed out"<<"\n";
			}else{
				cout<<_sIP<<" timed out."<<"\n";	
			}

			Disconnect();
			continue;
		}


		switch (_sBuffer[0]) {
		case 0x1: //Login
			iTemp = TCPHelper::readInt(_Connection); //Protocol Version
			
			if (iTemp > _pSettings->getSupportedProtocolVersion()) {
				Kick("Outdated server!");
				continue;
			}
			
			if (iTemp < _pSettings->getSupportedProtocolVersion()) {
				Kick("Outdated client!");
				continue;
			}

			TCPHelper::readString16(_Connection); //Username (already known)
			_Connection.receiveBytes(_sBuffer,16); //Unused fields

			//Check premium 
			if (_pSettings->isOnlineModeActivated()) {
				string sPath("/game/checkserver.jsp?user=");
				sPath.append(_sName);
				sPath.append("&serverId=");
				sPath.append(_sConnectionHash);

				Poco::Net::HTTPRequest Request ( 
					Poco::Net::HTTPRequest::HTTP_GET, 
					sPath,
					Poco::Net::HTTPMessage::HTTP_1_1);

				_Web_Session.sendRequest(Request);
				std::istream &is = _Web_Session.receiveResponse(_Web_Response);
				string sErg;
				is>>sErg;
				
				if (sErg.compare("YES") != 0) {
					Kick("Failed to verify username!");
					continue;
				}
			}

			//Answer
			_sTemp.assign(""); //Clar
			_sTemp.append<int>(1,0x1); //packet id
			TextHandler::Append(_sTemp,_iEntityID); //EntityID
			_sTemp.append<int>(2,0); //unused

			TextHandler::Append(_sTemp,_pSettings->getMapSeed());//MapSeed 
			TextHandler::Append(_sTemp,_pSettings->getServerMode()); //ServerMode
			_sTemp.append<char>(1,0); //Default Dimension
			TextHandler::Append(_sTemp,_pSettings->getDifficulty()); //Difficulty
			_sTemp.append<char>(1,_pSettings->getWorldHeight()); //WorldHeight
			_sTemp.append<char>(1,_pSettings->getMaxClients()); //Max Players

			_Connection.sendBytes(_sTemp.c_str(),_sTemp.length());	

			setAuthStep(FC_AUTHSTEP_VERIFYDONE); 
			break;
		case 0x2: //Handshake
			if (_iLoginProgress != FC_AUTHSTEP_CONNECTEDONLY) {
				Kick("False login order!");
				continue;
			}
			_sName = _sNickName = TCPHelper::readString16(_Connection);

			PlayerCount++;
			setAuthStep(FC_AUTHSTEP_HANDSHAKE);
			_iEntityID = _pEntityProvider->Add(FC_ENTITY_PLAYER); //Fetch a new entity id

			//Send response (Connection Hash)
			_sTemp.assign("");
			_sTemp.append<int>(1,0x02); 

			if (_pSettings->isOnlineModeActivated()) {
				generateConnectionHash();
				cout<<_sConnectionHash<<"\n";
				TextHandler::packString16(_sTemp,_sConnectionHash);
			}else{
				TextHandler::packString16(_sTemp,string("-"));
			}
			
			_Connection.sendBytes(_sTemp.c_str(),_sTemp.length());
			cout<<_sName<<" joined ("<<_sIP<<") EID:"<<_iEntityID<<endl;
			break;
		case 0xFE: //Server List Ping
			_sTemp.assign("");
			_sTemp.assign(_pSettings->getServerDescription()); //Server Description
			_sTemp.append<int>(1,0xA7);
			Poco::NumberFormatter::append(_sTemp,PlayerCount); //Player count
			_sTemp.append<int>(1,0xA7);
			Poco::NumberFormatter::append(_sTemp,_pSettings->getMaxClients()); //player slots
			_sTemp.append("§");
			Kick(_sTemp);
			break;
		case 0xF0: //if unused packed id not changed -> connection aborted
			Disconnect();
			break;
		default: 
			Kick("Unknown packet!");
			cout<<"Unknown packet received! 0x"<<std::hex<<int(_sBuffer[0])<<endl;
			break;
		}

	}
}

void PlayerThread::Disconnect(bool fKicked) {
	if (isNameSet()) {
		PlayerCount--;
	}

	if (isNameSet() && !fKicked) { // If names is known
		cout<<_sName<<" left server."<<"\n"; 
		_pEntityProvider->Remove(_iEntityID);
		_iEntityID = 0;
	}

	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;
	_Coordinates.OnGround = false;
	_Coordinates.Pitch = 0.0F;
	_Coordinates.Stance = 0.0;
	_Coordinates.X = 0.0;
	_Coordinates.Y = 0.0;
	_Coordinates.Yaw = 0.0F;
	_Coordinates.Z = 0.0;

	setAuthStep(FC_AUTHSTEP_NOTCONNECTED);

	_fAssigned = false;
	_iEntityID = 0;

	_sName.assign("");
	_sNickName.assign("");
	_sIP.assign("");
	ClearQueue();
	_Connection.close();
}


void PlayerThread::Connect(Poco::Net::StreamSocket& Sock) {
	if (_fAssigned) {
		cout<<"***INTERNAL WARNING: PlayerPool tryed to assign an already assigned player thread"<<"\n";
		Disconnect();
	}
	_fAssigned=true;
	setAuthStep(FC_AUTHSTEP_CONNECTEDONLY);
	
	_Connection = Sock; 
	_Connection.setReceiveTimeout( Poco::Timespan( 1000 * 5000) );

	_sIP.assign(_Connection.peerAddress().toString());
}

bool PlayerThread::isAssigned() {
	return _fAssigned;
}

string PlayerThread::getUsername() {
	return _sName;
}

string PlayerThread::getNickname() {
	return _sNickName;
}

string PlayerThread::getIP() {
	return _sIP;
}

int PlayerThread::getThreadID() {
	return _iThreadID;
}

void PlayerThread::Kick(string sReason) {
	QueueJob qJob;

	qJob.Data.assign("");
	qJob.Data.append<int>(1,0xFF);
	TextHandler::packString16(qJob.Data,sReason);
	qJob.Special = FC_JOB_CLOSECONN;

	if (isNameSet()) { // If names is known
		cout<<_sName<<" was kicked for: "<<sReason<<"\n"; 
	}

	appendQueue(qJob);
}

void PlayerThread::Kick() {
	QueueJob qJob;

	qJob.Data.assign("");
	qJob.Data.append<int>(1,0xFF);
	qJob.Data.append<int>(2,0x0);
	qJob.Special = FC_JOB_CLOSECONN;

	if (_sName.compare("") !=0) { // If names is known
		cout<<_sName<<" was kicked"<<"\n"; 
	}

	appendQueue(qJob);
}

void PlayerThread::ProcessQueue() {
	QueueJob qJob;
	if (_SendQueue.size() == 0) {return;}

	qJob = _SendQueue.front();
	_SendQueue.pop();

	_Connection.sendBytes(qJob.Data.c_str(),qJob.Data.length());

	switch(qJob.Special) {
	case FC_JOB_CLOSECONN:
		Thread::sleep(200); 
		Disconnect(true);
		break;
	default:
		cout<<"***INTERNAL SERVER ERROR: Unknown job ID: ("<<qJob.Special<<") !"<<"\n";
		break;
	}
}

void PlayerThread::appendQueue(QueueJob& Job) {
	_SendQueue.push(Job);
}

bool PlayerThread::isNameSet() {
	return _iLoginProgress >= FC_AUTHSTEP_HANDSHAKE;
}

void PlayerThread::generateConnectionHash() {
	_iLastConnHash += std::rand();
	_iLastConnHash <<=1;

	std::stringstream StringStream;
	string sHash;
	StringStream<<std::hex<<_iLastConnHash;
	StringStream>>sHash;
	
	_sConnectionHash.assign(sHash);
}

void PlayerThread::setAuthStep(char iMode) {
	_iLoginProgress = iMode;
}
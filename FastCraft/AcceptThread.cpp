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
#include "AcceptThread.h" 
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/NetException.h>
#include <Poco/Thread.h>
#include "MinecraftServer.h"
#include "NetworkOut.h"
#include "PlayerPool.h"
#include "FCRuntimeException.h"
#include "PlayerThread.h"
#include <utility>

using Poco::Net::StreamSocket;
using std::cout;
using Poco::Thread;


AcceptThread::AcceptThread(MinecraftServer* pServer) 
try :
ServerThreadBase("AcceptThread"),
_ServerSock(pServer->getPort()),
_preparedServerFullMsg("")
{
	_pMinecraftServer = pServer;
	_preparedServerFullMsg.append(1,0xFF);
	NetworkOut::addString(_preparedServerFullMsg,pServer->getServerFullMessage());

	startThread(this);

	_ServerSock.setReusePort(false);

}catch(Poco::IOException& ex) {
	cout<<"Unable to bind 0.0.0.0:"<<pServer->getPort()<<" ("<<ex.message()<<")\n";
	throw FCRuntimeException("Unable to start AcceptThread");
}


AcceptThread::~AcceptThread() {
	_ServerSock.close();
	killThread();
}

void AcceptThread::run() {
	Poco::Net::StreamSocket StrmSock;
	string sNewConnectionIP;

	vector<std::pair<string,Poco::Timestamp::TimeDiff>> vConnections,vBannedIPs;
	vector<	std::pair<
						string,  /* IP */
						std::pair<
								Timestamp::TimeDiff, /* Ban time */
								Timestamp::TimeDiff /* Entry reset */
								 >
					>
	       > vBanLenght;

	Poco::Stopwatch Timer;
	Poco::Timestamp::TimeDiff lowest,actual;
	
	int x,i;
	short iCount;
	bool fExit;
	
	Timer.start();


	_iThreadStatus = FC_THREADSTATUS_RUNNING;
	while(_iThreadStatus==FC_THREADSTATUS_RUNNING) {
		try {
			_ServerSock.listen();
			StrmSock = _ServerSock.acceptConnection(); 
			cutOffPort(StrmSock.peerAddress().toString(),sNewConnectionIP);
			
			
			/* Check banlist */
			if (!vBannedIPs.empty()) {
				actual = (Timer.elapsed()/1000);
				fExit = false;

				for (x=vBannedIPs.size()-1;x>=0;x--){
					if (vBannedIPs[x].first.compare(sNewConnectionIP) != 0) {continue;}
					i = search(vBanLenght,sNewConnectionIP);

					if (actual - vBannedIPs[x].second >= vBanLenght[i].second.first) { /* Ban timed out */			
						/* Remove from connection list */
						if (!vConnections.empty()){
							for (i=vConnections.size()-1;i>=0;i--) { 
								if (vConnections[i].first.compare(vBannedIPs[x].first) == 0){vConnections.erase(vConnections.begin()+i);}
							}
						}

						vBannedIPs.erase(vBannedIPs.begin()+x); /* Remove from banlist */
						continue;
					}
					if (vBannedIPs[x].first.compare(sNewConnectionIP) == 0) {
						StrmSock.close();
						fExit = true;
						break;
					}
				}
				if (fExit) {continue;}
			}

			/* Remove old banlenght entries */
			if (!vBanLenght.empty()) {
				for (x = vBanLenght.size()-1;x>=0;x--) {
					if (vBanLenght[x].second.second < Timer.elapsed()/1000) { /* last ban was for 5 minutes -> remove it*/
						vBanLenght.erase( vBanLenght.begin()+x);
					}
				}
			}

			/* Manage connection list + ban if needed */
			vConnections.push_back(std::make_pair(sNewConnectionIP,Timer.elapsed()/1000L)); /* Add connection to list */
			iCount = 0;
			lowest = 30000;
			fExit = false;
			for (x=vConnections.size()-1;x>=0;x--) {
				if ((Timer.elapsed()/1000) - vConnections[x].second >= 30000){ /* Entry is old, remove it */
					vConnections.erase(vConnections.begin()+x);
					continue;
				}

				/* Timespan between actual time and connection time of this entry */
				actual = (Timer.elapsed()/1000) - vConnections[x].second;

				if (vConnections[x].first.compare(sNewConnectionIP) == 0) { /* This IP connected more than one time */
					iCount++;
					if (actual < lowest) {lowest = actual;}
				}

				/* More than four connections in a timespan of 10 seconds -> ban IP */
				if (iCount > 4 && lowest <= 10000) {
					StrmSock.close();
					vBannedIPs.push_back(std::make_pair(sNewConnectionIP,Timer.elapsed()/1000));

					/* Set banlenght */
					if ((x = search(vBanLenght,sNewConnectionIP)) == -1) {
						vBanLenght.push_back(std::make_pair(sNewConnectionIP,std::make_pair(10000,Timer.elapsed()/1000 + 5*60*1000)));
						/*x = vBanLenght.size()-1;*/
					}else{
						vBanLenght[x].second.first *= 2;
						vBanLenght[x].second.second = Timer.elapsed()/1000 + (5*60*1000) + vBanLenght[x].second.first;
					}
					/*cout<<vBanLenght[x].first<<" banned for:"<< (vBanLenght[x].second.first/1000)<<" seconds\n";*/

					fExit = true;
					break;
				}
			}
			if (fExit) {continue;}
			

			if (!_pMinecraftServer->getPlayerPool()->isAnySlotFree()) { //There is no free slot
				StrmSock.sendBytes(_preparedServerFullMsg.c_str(),_preparedServerFullMsg.length());
				StrmSock.close();
				continue;
			}

			_pMinecraftServer->getPlayerPool()->Assign(StrmSock);
		}catch(...) {       /* Only happen if socket become closed */
			_iThreadStatus = FC_THREADSTATUS_DEAD;
			return;
		}
	}
}


void AcceptThread::cutOffPort(string sSource,string& rTarget) {
	auto it = sSource.begin();
	while (*it != ':') { it++;}
	rTarget.assign(sSource.begin(),it);
}

int AcceptThread::search(vector<std::pair<string,std::pair<Timestamp::TimeDiff,Timestamp::TimeDiff>>>& rVec,string sIP) {
	if (rVec.empty()) {return -1;}
	for (int x=0;x<=rVec.size()-1;x++) {
		if (rVec[x].first.compare(sIP) == 0) {return x;}
	}
	return -1;
}

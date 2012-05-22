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

#ifndef _FASTCRAFTHEADER_CONSTANTS
#define _FASTCRAFTHEADER_CONSTANTS
#include <utility>
#include <map>
#include <string>

//Core Constants
#define FC_VERSION						"0.0.2 Alpha"
#define FC_SUPPORTED_MINCRAFTVERSION	"1.1"
#define FC_SUPPORTED_DOCVER				2
#define FC_SUPPORTED_PROTOCOLVERSION	23

//Various Constants
#define FC_PLAYERSPAWNRADIUS			100.0
#define FC_MAXLOGINTIME					10000 //10 Seconds to complete login procedure
#define FC_EMPTYITEMID					std::make_pair<short,char>(-1,-1)
#define FC_WORLDHEIGHT					128
#define FC_UNKNOWNEID					-1


//Chunk Constants
#define FC_CHUNK_BLOCKCOUNT				32768
#define FC_CHUNK_NIBBLECOUNT			16384
#define FC_CHUNK_DATACOUNT              81920

//Thread status
#define FC_THREADSTATUS_DEAD			1
#define FC_THREADSTATUS_RUNNING			2
#define FC_THREADSTATUS_TERMINATING		3

//Queue types
#define FC_QUEUE_LOW					1
#define FC_QUEUE_HIGH					2

//Leave reasons
#define FC_LEAVE_KICK				    1
#define FC_LEAVE_QUIT					2
#define FC_LEAVE_OTHER					3

//vServer status constants
#define FC_VSERVERERROR_OK				0
#define FC_VSERVERERROR_INACTIVE		1
#define FC_VSERVERERROR_ERROR			2

//Entity actions
#define FC_ENTITY_ACTION_CROUCH			1
#define FC_ENTITY_ACTION_UNCROUCH		2
#define FC_ENTITY_ACTION_LEAVEBED		3
#define FC_ENTITY_ACTION_STARTSPRINTING	4
#define FC_ENTITY_ACTION_STOPSPRINTING	5

using std::string;

class Constants {
private:
	static std::map<string,char> _Map;
public:
	/*
	* Inits all maps
	*/
	static void init();


	/*
	* Returns value of given key
	* For all valid pathes, look into Constants.cpp
	* Throws Poco::RuntimeException if key doesn't exists

	Parameter:
	@1 : Key name
	*/
	static char get(string);


	/*
	* Returns true if a node contains an element with the given value
	* False otherwise

	Parameter:
	@1 : Value
	@2 : Node to search in
	*/
	static bool isDefined(char,string);
private:
	static string cutOffEndElement(string);
}; 
#endif
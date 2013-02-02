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


#ifndef _FASTCRAFTHEADER_NBTTAGCOMPOUND
#define _FASTCRAFTHEADER_NBTTAGCOMPOUND
#include "NBTBase.h"
#include <map>
#include <Poco\Path.h>
using std::map;

class NBTTagByte;
class NBTTagDouble;
class NBTTagFloat;
class NBTTagInt;
class NBTTagInt64;
class NBTTagShort;
class NBTTagString;
class NBTTagByteArray;
class NBTTagIntArray;
class NBTTagList;

class NBTTagCompound : public NBTBase {
private:
	map<string,NBTBase*> _tagMap;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Name
	*/
	NBTTagCompound(string);


	/*
	* Constructor
	* Opens given file and initializes compound
	* Stops parsing on error
	* Throws no exceptions

	Parameter:
	@1 : Compression Type (FC_NBT_COMPRESS @ NBTConstants.h)
	@2 : File Path
	*/
	NBTTagCompound(char,Poco::Path&);


	/*
	* Destructor
	*/
	~NBTTagCompound();


	/*
	* Adds an element to the tagmap
	* If the element already exist, it will be deleted 
	*/
	void add(NBTBase*);


	/*
	* Returns a pointer to the element
	* Throws FCException if
	   - Element wasn't found
	   - Element type is not fitting into the requested one 
	Parameter:
	@1 : Name
	@2 : Set to true to create a new element if it not exists
	*/
	NBTTagCompound*  getCompound	(string);
	NBTTagByte*		 getByte		(string);
	NBTTagDouble*	 getDouble		(string);
	NBTTagFloat*	 getFloat		(string); 
	NBTTagInt*		 getInt			(string);
	NBTTagInt64*	 getInt64		(string);
	NBTTagShort*	 getShort		(string);
	NBTTagString*	 getString		(string);
	NBTTagByteArray* getByteArray	(string);
	NBTTagIntArray*  getIntArray	(string);
	NBTTagList*	     getList		(string);


	/*
		NBTTagCompound*  getCompound	(string,bool = false);
	NBTTagByte*		 getByte		(string,bool = false);
	NBTTagDouble*	 getDouble		(string,bool = false);
	NBTTagFloat*	 getFloat		(string,bool = false); 
	NBTTagInt*		 getInt			(string,bool = false);
	NBTTagInt64*	 getInt64		(string,bool = false);
	NBTTagShort*	 getShort		(string,bool = false);
	NBTTagString*	 getString		(string,bool = false);
	NBTTagByteArray* getByteArray	(string,bool = false);
	NBTTagIntArray*  getIntArray	(string,bool = false);
	NBTTagList*	     getList		(string,bool = false);
	*/

	/*
	* Initializes a new NBT structure
	* Throws FCException if stream starts with unexpected bytes
	* Rethrows all errors that occur in the parsing process

	Parameter:
	@1 : Target stream
	*/
	void init(istream&);


	/*
	* Returns the elements type
	*/ 
	char getType();


	/*
	* Returns true if given element exists
	* Returns false if:
	          -  element doesn't exist
			  -  element exists but has a incorrect type (deletes element automaticly)

	Parameter:
	@1 : Name
	@2 : Needed type (FC_NBT_TYPE_ @ NBTConstants.h)
	*/
	bool has(string,char);


	/* 
	* Internal use only
	*/
	void save(ostream&);
	void load(istream&,bool = true);

	static short readShort(istream&);
	static string readString(istream&);
	static int readInt(istream&);
	
	static void writeShort(ostream&);
	static void writeString(ostream&);
	static void writeInt(ostream&);

	union uniIntFloat {
		int i;
		float f;
		char s[4];
	};

	union uniLongDouble {
		long long i;
		double f;
		char s[8];
	};
private: 
	template<typename Tag> Tag* getElement(string);
};
#endif

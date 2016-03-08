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

#ifndef _FASTCRAFTHEADER_IOFILE
#define _FASTCRAFTHEADER_IOFILE
#include <fstream>
#include <Poco\File.h>
#include <Poco\Path.h>

using std::fstream;

class IOFile {
private:
	fstream _stream;
	int _iSize;
public:
	/*
	* Constructor
	* get and put pointers are set to 0
	* Throws FCException if is 
	    - unable to read file (permission)
		- unable to write file (permissions)
		- unable to find file
		- unable to open file

	Paramter:
	@1 : Path to file
	*/
	IOFile(Poco::File&);


	/*
	* Destructor
	*/
	~IOFile();


	/*
	* Writes content to stream
	* Throws FCException if
	   - Size is invalid (<= 0)
	   - Data pointer is 0
	Parameter:
	@1 : Pointer to content to write
	@2 : Size of content
	*/
	void write(char*,int);

	
	/*
	* Reads content from file into variable
	* Returns false if Count wasn't reached before file end

	Paramter:
	@1 : Destination 
	@2 : Count
	*/
	bool read(char*,int);


	/*
	* Sets the file-position-pointer
	* Returns false if new position is beyond file size

	Parameter:
	@1 : New position
	*/ 
	bool setPosition(int);


	/*
	* Returns size of file
	*/
	int getSize();


	/*
	* Creates an directory if it doesn't exist
	* Throws FCException if an error occurs
	*/
	static void createDir(Poco::Path&);
};
#endif

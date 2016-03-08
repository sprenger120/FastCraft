#ifndef _FASTCRAFTHEADER_CONSTSTRING
#define _FASTCRAFTHEADER_CONSTSTRING
#include <string>
#include <istream>

/* A class that handles constant and non constant char arrays*/
/* The string has to be \0 terminated */
class ConstString {
private:
	char * _pAllocString;
	const char* _pConstantString;
	size_t _iLen;
public:
	/*
	* Constructors
	* Throws NullpointerException if passed pointers are NULL
	*/
	ConstString(const char*);
	ConstString(char*);

	
	/*
	Parameter:
	@1 : A constant char array
	*/
	ConstString(std::string&);

	
	/*
	Parameter:
	@1 : An on the heap allocated char array (will be freed on destruction)

	Parameter:
	@1 : A std::string (content will be copied into a char)

	Parameter:
	@1 : An istream that holds the string's contents (will be copied into a char)
	@2 : Length of the string (without +1 for \0)
	* Throws IllegalLengthException
	* Throws EndOfFileException and keeps object unassigned if eof flag was set before reading
	* If the stream reaches its end before Len bytes were readed
	  ConstString sets _iLen to its actual size
	*/
	ConstString(std::istream&,size_t);


	/*
	* Destructor
	*/
	~ConstString();


	/*
	* Operators
	*/
	void operator=(ConstString&); 
	//Compares length
	/*bool operator>(const ConstString&) const;*/
	bool operator<(const ConstString&) const;
	//bool operator<=(const ConstString&) const;
	//bool operator>=(const ConstString&) const;
	//Compares content
	bool operator==(const ConstString&) const;
	bool operator!=(const ConstString&) const;


	/*
	* Returns length
	*/
	size_t getLen() const;


	/*
	* Returns a pointer to data
	* Returns NULL if no char was assigned so far
	*/
	char* getPtr() const;


	/*
	* Checks if a char array was assigned before
	*/
	bool isEmpty() const;
};
#endif
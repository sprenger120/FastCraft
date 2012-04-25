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

#ifndef _FASTCRAFTHEADER_HEAP
#define _FASTCRAFTHEADER_HEAP
#include <iostream>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>
using std::string;


template<typename tDataType,typename tAdressType> 
class Heap {
	class HeapElement {
	public:
		tDataType* pElement;
		HeapElement* pLow;
		HeapElement* pHigh;
		HeapElement* pNextRowElement;

		HeapElement();
		~HeapElement();
	};

	string toBinary(tAdressType);

	HeapElement* _Root;
	HeapElement* _pLastAddedEntry;

	tAdressType _iSize;
	tAdressType _iNodeCount;
	tAdressType* _pSingleBitMaskArray;
	tAdressType* _pEndBitsMaskArray;

	Poco::Mutex _Mutex;
public:
	class HeapInterator {
		HeapElement* _pBegin;
		HeapElement* _pActual;
	public:
		/*
		* Constructor
		*/
		HeapInterator(HeapElement*);


		/*
		* Destructor
		*/
		~HeapInterator();


		/*
		* Moves interator to next element
		*/
		void operator++();

		/*
		* Access operator
		*/
		tDataType* operator->();


		/*
		* Access operator
		*/
		tDataType& operator*();


		/*
		* Returns pointer of start element
		*/
		tDataType* beg();


		/*
		* Returns true if interator has reached the end
		*/
		bool isEndReached();
	};

	/*
	* Constructor
	*/
	Heap();


	/*
	* Destructor
	*/
	~Heap();


	/*
	* Adds a new element to heap
	* Throws Poco::RuntimeException if element already exists

	Parameter:
	@1 : ID of element 
	@2 : Reference to new element (will be copied)
	*/
	void add(tAdressType,tDataType&);


	/*
	* Returns true if element with given ID exists

	Parameter:
	@1 : ID of element
	*/
	bool has(tAdressType);


	/*
	* Gets element by ID operator
	* Returns NULL if element wasn'f found
	*/
	tDataType* operator[](tAdressType);


	/*
	* Deallocates all elements
	* Doesn't remove nodes
	*/
	void cleanupElements();


	/*
	* Removes all nodes and all elements
	*/
	void cleanupNodes();


	/*
	* Returns count of allocated elements
	*/
	void getElementCount();


	/*
	* Returns count of allocated nodes
	*/
	void getNodeCount();


	/*
	* Returns interator to beginning
	*/
	typename Heap<tDataType,tAdressType>::HeapInterator begin();	
};

template<typename tDataType,typename  tAdressType>
Heap<tDataType,tAdressType>::Heap() {
	/* Init variables */
	_iSize = 0;
	_iNodeCount = 1;
	_pLastAddedEntry = _Root = new HeapElement;


	/* building bit tables */
	_pSingleBitMaskArray = new tAdressType[sizeof(tAdressType)*8];
	_pEndBitsMaskArray   = new tAdressType[sizeof(tAdressType)*8];
	for(char i = 0; i <= sizeof(tAdressType)*8-1;i++) {
		_pSingleBitMaskArray[i] = tAdressType(1)<<i;
		_pEndBitsMaskArray[i]   = tAdressType(~0)<<i+1;
	} 
}

template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::~Heap() {
	delete [] _pSingleBitMaskArray;
	delete [] _pEndBitsMaskArray;

	if (_iSize > 0) {cleanupNodes();}
}


template<typename tDataType,typename tAdressType>
string Heap<tDataType,tAdressType>::toBinary(tAdressType i) {
	std::string sData("");

	do{
		if (i%2) {
			sData = "1" + sData;
		}else{
			sData = "0" + sData;
		}
		i >>= 1;
	} while(i != 0);

	while (sData.length() < sizeof(tAdressType)*8) {
		sData = "0" + sData;
	}

	return sData;
}

template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::add(tAdressType ID,tDataType& rElement) {
	tDataType* pElement = new tDataType(rElement);
	HeapElement* pPath = _Root;
	HeapElement* p = _Root;
	bool fEnd;

	Poco::ScopedLock<Poco::Mutex> sLock(_Mutex);

	for (char i = 0;i<=sizeof(tAdressType)*8-1;i++) {
		fEnd = (ID & _pEndBitsMaskArray[i])   == 0;

		switch(ID & _pSingleBitMaskArray[i]) {
		case 0:
			if (pPath->pLow == NULL) { 
				pPath->pLow = new HeapElement;
				_iNodeCount++;

				_pLastAddedEntry->pNextRowElement = pPath->pLow;
				_pLastAddedEntry = pPath->pLow;
			}
			p = pPath->pLow;
			break;
		default:
			if (pPath->pHigh == NULL) { 
				pPath->pHigh = new HeapElement;
				_iNodeCount++;

				_pLastAddedEntry->pNextRowElement = pPath->pHigh;
				_pLastAddedEntry = pPath->pHigh;
			}
			p = pPath->pHigh;
			break;
		}

		if (fEnd) {
			if (p->pElement != NULL) {
				delete pElement;
				throw Poco::RuntimeException("Already existing");
			}
			p->pElement = pElement; 

			_iSize++;
			return;
		}else{
			pPath = p;
		}
	}
}


template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::HeapElement::HeapElement() {
	pElement = NULL;
	pLow = NULL;
	pHigh = NULL;
	pNextRowElement = NULL;
}

template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::HeapElement::~HeapElement() {
}

template<typename tDataType,typename tAdressType>
tDataType* Heap<tDataType,tAdressType>::operator[](tAdressType ID) {
	HeapElement* pPath = _Root;
	HeapElement* p;
	bool fEnd;

	for (char i = 0;i<=sizeof(tAdressType)*8-1;i++) {
		fEnd = (ID & _pEndBitsMaskArray[i]) == 0;

		switch(ID & _pSingleBitMaskArray[i]) {
		case 0:
			if (pPath->pLow == NULL) { return NULL;}
			p = pPath->pLow;
			break;
		default:
			if (pPath->pHigh == NULL) { return NULL;}
			p = pPath->pHigh;
			break;
		}

		if (fEnd) {
			return p->pElement;
		}else{
			pPath = p;
		}
	}
	return NULL;
}

template<typename tDataType,typename tAdressType>
bool Heap<tDataType,tAdressType>::has(tAdressType ID) {
	return (*this)[ID] != NULL;
}


template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::HeapInterator::HeapInterator(HeapElement* p) {
	_pActual = _pBegin = p;
}

template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::HeapInterator::~HeapInterator() {
}


template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::HeapInterator::operator++() {
	if (_pActual == NULL) {return;}
	//if (_pActual->pNextRowElement == NULL) {return;}

	/* mode to next node with a allocated element */	
	_pActual = _pActual->pNextRowElement;
	while (1) {
		if (_pActual == NULL) {break;}
		if (_pActual->pElement != NULL) {break;}
		_pActual = _pActual->pNextRowElement;
	}
}

template<typename tDataType,typename tAdressType>
tDataType* Heap<tDataType,tAdressType>::HeapInterator::operator->() {
	return _pActual->pElement;
}

template<typename tDataType,typename tAdressType>
tDataType& Heap<tDataType,tAdressType>::HeapInterator::operator*() {
	return *(_pActual->pElement);
}

template<typename tDataType,typename tAdressType>
tDataType* Heap<tDataType,tAdressType>::HeapInterator::beg() {
	return _pBegin->pElement;
}

template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::cleanupElements() {
	HeapElement* p = _Root;

	do {
		if (p->pElement != NULL) { 
			delete p->pElement;
			p->pElement = NULL;
			_iSize--;
		}
		p = p->pNextRowElement;
	}while(p != NULL);
}

template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::cleanupNodes() {
	HeapElement* pE = _Root;
	HeapElement* pNextE = NULL;
	do{
		if (pE->pElement != NULL) { 
			delete pE->pElement;
			pE->pElement = NULL;
			_iSize--;
		}
		pNextE = pE->pNextRowElement;//Save pointer to nex element

		delete pE;					// remove node
		_iNodeCount--;


		pE = pNextE;
	}while(pE != NULL);

}

template<typename tDataType,typename tAdressType>
typename Heap<tDataType,tAdressType>::HeapInterator Heap<tDataType,tAdressType>::begin() {
	HeapInterator interator(_Root->pNextRowElement);
	return interator;
}

template<typename tDataType,typename tAdressType>
bool Heap<tDataType,tAdressType>::HeapInterator::isEndReached() {
	if (_pActual == NULL)  {return true;}

	HeapElement* p = _pActual;
	while (1) {
		if (p->pElement != NULL) {return false;}
		if (p->pNextRowElement == NULL) {return true;}
		p = p->pNextRowElement;
	}
	return true;
}
#endif
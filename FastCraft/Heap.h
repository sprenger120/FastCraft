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
#include "FCException.h"
using std::string;


template<typename tDataType,typename tAdressType> 
class Heap {
	class HeapElement {
	public:
		tDataType pElement;
		HeapElement* pLow;
		HeapElement* pHigh;
		HeapElement* pNextRowElement;

		HeapElement();
	};

	HeapElement* _Root;
	HeapElement* _pLastAddedEntry;

	tAdressType _iSize;
	tAdressType _iNodeCount;
	tAdressType* _pSingleBitMaskArray;
	tAdressType* _pEndBitsMaskArray;

	Poco::Mutex _Mutex;
public:
	class HeapIterator {
		HeapElement* _pBegin;
		HeapElement* _pActual;
		bool _fEnd;
	public:
		/*
		* Constructor
		*/
		HeapIterator(HeapElement*);


		/*
		* Moves Iterator to next element
		*/
		void operator++();
		void operator++(int);


		/*
		* Access operator
		* Returns last element if Iterator reaches the end
		* Throws FCException if heap doesn't contains any elements
		*/
		tDataType operator->();


		/*
		* Returns pointer of actual element
		* Returns last element if Iterator reaches the end
		* Throws FCException if heap doesn't contains any elements
		*/
		tDataType getPtr();


		/*
		* Returns true if Iterator has reached the end
		*/
		bool isEndReached();
	};

	/*
	* Constructor
	* This heap can only store pointers!
	*/
	Heap();


	/*
	* Destructor
	*/
	~Heap();


	/*
	* Adds a new element to heap
	* Throws FCException if element already exists

	Parameter:
	@1 : ID of element 
	@2 : the new element (allocated in the heap)
	*/
	void add(tAdressType,tDataType);


	/*
	* Gets element by ID 
	* Returns NULL if element wasn't found

	Parameter:
	@1 : Index of element
	*/
	tDataType get(tAdressType);


	/*
	* Returns true if element with given ID exists

	Parameter:
	@1 : ID of element
	*/
	bool has(tAdressType);


	/*
	* Removes an element
	* Throws FCException if element doesn't exists

	Parameter:
	@1 : ID of element
	*/
	void erase(tAdressType);


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
	tAdressType getElementCount();


	/*
	* Returns count of allocated nodes
	*/
	tAdressType getNodeCount();


	/*
	* Returns Iterator to beginning
	*/
	typename Heap<tDataType,tAdressType>::HeapIterator begin();	
private:
	string toBinary(tAdressType);
	typename Heap<tDataType,tAdressType>::HeapElement* getElement(tAdressType);
};


/*
 * Heap
 */
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
		_pEndBitsMaskArray[i]   = tAdressType(~0)<<(i+1);
	} 
}

template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::~Heap() {
	delete [] _pSingleBitMaskArray;
	delete [] _pEndBitsMaskArray;

	cleanupNodes();
	delete _Root;
	_iNodeCount--;
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
void Heap<tDataType,tAdressType>::add(tAdressType ID,tDataType pElement) {
	HeapElement* pPath = _Root;
	HeapElement* p = _Root;
	bool fEnd;

	Poco::ScopedLock<Poco::Mutex> sLock(_Mutex);

	for (char i = 0;i<=sizeof(tAdressType)*8-1;i++) {
		fEnd = (ID & _pEndBitsMaskArray[i])  == 0;

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
				throw FCException("Already existing");
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
typename Heap<tDataType,tAdressType>::HeapElement* Heap<tDataType,tAdressType>::getElement(tAdressType ID) {
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
			if (p->pElement == NULL) {return NULL;}
			return p;
		}else{
			pPath = p;
		}
	}
	return NULL;
}


template<typename tDataType,typename tAdressType>
tDataType Heap<tDataType,tAdressType>::get(tAdressType ID) {
	HeapElement* p = getElement(ID);

	if (p == NULL || p->pElement == NULL) {return NULL;}

	return p->pElement;
}

template<typename tDataType,typename tAdressType>
bool Heap<tDataType,tAdressType>::has(tAdressType ID) {
	HeapElement* p = getElement(ID);

	if (p == NULL || p->pElement == NULL) {
		return false;
	}else{
		return true;
	}
}

template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::erase(tAdressType ID) {
	HeapElement* p = getElement(ID);
	if (p == NULL || p->pElement == NULL) {throw FCException("Element doesn't exists");}
	
	Poco::ScopedLock<Poco::Mutex> sLock(_Mutex);

	delete p->pElement;
	p->pElement = NULL;
	_iSize--;
}

template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::cleanupElements() {
	HeapElement* p = _Root;
	
	Poco::ScopedLock<Poco::Mutex> sLock(_Mutex);

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

	Poco::ScopedLock<Poco::Mutex> sLock(_Mutex);

	do{
		if (pE->pElement != NULL) { 
			delete pE->pElement;
			_iSize--;
		}
		pNextE = pE->pNextRowElement;//Save pointer to nex element

		if (pE != _Root) {delete pE;} // remove node
		_iNodeCount--;


		pE = pNextE;
	}while(pE != NULL);
}

template<typename tDataType,typename tAdressType>
typename Heap<tDataType,tAdressType>::HeapIterator Heap<tDataType,tAdressType>::begin() {
	HeapIterator Iterator(_Root->pNextRowElement);
	return Iterator;
}


template<typename tDataType,typename tAdressType>
tAdressType Heap<tDataType,tAdressType>::getElementCount() {
	return _iSize;
}


template<typename tDataType,typename tAdressType>
tAdressType Heap<tDataType,tAdressType>::getNodeCount() {
	return _iNodeCount;
}


/* 
 * HeapElement
 */
template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::HeapElement::HeapElement() {
	pElement = NULL;
	pLow = NULL;
	pHigh = NULL;
	pNextRowElement = NULL;
}



/*
 * HeapIterator
 */
template<typename tDataType,typename tAdressType>
Heap<tDataType,tAdressType>::HeapIterator::HeapIterator(HeapElement* p) {
	_pBegin = p;
	_pActual = NULL;
	_fEnd = false;

	if(_pBegin == NULL) { 
		_fEnd = true;
	}else {
		(*this)++;
	}
}

template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::HeapIterator::operator++(int) {
	++(*this);
}

template<typename tDataType,typename tAdressType>
void Heap<tDataType,tAdressType>::HeapIterator::operator++() {
	if (_fEnd) {return;}

	HeapElement* pActual = (_pActual == NULL ? _pBegin : _pActual->pNextRowElement); //if _pActual == NULL -> first call of ++
	while (1) {
		if (pActual == NULL) { /* doesn't happen on first call */ /* returns if rows end has reached end */
			_fEnd = true;
			return;
		} 
		if (pActual->pElement != NULL) {break;} /* there is a node with a new element; break */
		pActual = pActual->pNextRowElement; /* no element found:  next element */
	}
	if (pActual != NULL) {
		_pActual = pActual;
	}
}

template<typename tDataType,typename tAdressType>
tDataType Heap<tDataType,tAdressType>::HeapIterator::operator->() {
	if (_pActual == NULL) {throw FCException("Doesn't contain any elements");} /* constructor moves to a valid element; keeps NULL if heap is empty */
	return _pActual->pElement;
}

template<typename tDataType,typename tAdressType>
bool Heap<tDataType,tAdressType>::HeapIterator::isEndReached() {
	return _fEnd;
}

template<typename tDataType,typename tAdressType>
tDataType Heap<tDataType,tAdressType>::HeapIterator::getPtr() {
	if (_pActual == NULL) {throw FCException("Doesn't contain any elements");} 
	return _pActual->pElement;
}
#endif

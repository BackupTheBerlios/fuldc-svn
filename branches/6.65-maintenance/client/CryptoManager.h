/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#if !defined(CRYPTO_MANAGER_H)
#define CRYPTO_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SettingsManager.h"

#include "Exception.h"
#include "Singleton.h"
#include "FastAlloc.h"
#include "version.h"

STANDARD_EXCEPTION(CryptoException);

class Node : public FastAlloc<Node> {
public:
	// What's this? The only way (I've found out) to avoid a Internal Compiler Error! If this class is moved into
	// CryptoManager along with the greater specialization, it generates a ICE on the greater class. The typedefs
	// had to be removed in order to avoid template instatiation.
//	typedef Node* Ptr;
//	typedef list<Ptr> List;
//	typedef List::iterator Iter;
	int chr;
	int weight;

	Node* left;
	Node* right;

	Node(int aChr, int aWeight) : chr(aChr), weight(aWeight), left(NULL), right(NULL) { }
	Node(Node* aLeft, Node* aRight) : chr(-1), weight(aLeft->weight + aRight->weight), left(aLeft), right(aRight) { }
	~Node() {
		delete left;
		delete right;
	}
	bool operator <(const Node& rhs) const {
		return weight<rhs.weight;
	}
	bool operator >(const Node& rhs) const {
		return weight>rhs.weight;
	}
	bool operator <=(const Node& rhs) const {
		return weight<=rhs.weight;
	}
	bool operator >=(const Node& rhs) const {
		return weight>rhs.weight;
	}
};

class File;
class FileException;

class CryptoManager : public Singleton<CryptoManager>
{
public:
	string makeKey(const string& aLock);
	const string& getLock() { return lock; }
	const string& getPk() { return pk; }
	bool isExtended(const string& aLock) { return strncmp(aLock.c_str(), "EXTENDEDPROTOCOL", 16) == 0; }

	void decodeBZ2(const u_int8_t* is, size_t sz, string& os) throw(CryptoException);
private:

	friend class Singleton<CryptoManager>;

	CryptoManager();
	virtual ~CryptoManager();

	const string lock;
	const string pk;

	string keySubst(const u_int8_t* aKey, size_t len, size_t n);
	bool isExtra(u_int8_t b) {
		return (b == 0 || b==5 || b==124 || b==96 || b==126 || b==36);
	}
};

#endif // !defined(CRYPTO_MANAGER_H)

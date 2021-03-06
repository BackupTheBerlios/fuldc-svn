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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "CryptoManager.h"

#include "BitInputStream.h"
#include "BitOutputStream.h"
#include "ResourceManager.h"

#if defined(_WIN32) || defined(_WIN64)
#include "../bzip2/bzlib.h"
#else
#include <bzlib.h>
#endif

CryptoManager::CryptoManager()
:
	lock("EXTENDEDPROTOCOLABCABCABCABCABCABC"),
	pk("DCPLUSPLUS" VERSIONSTRING "ABCABC")
{ }

CryptoManager::~CryptoManager() { }

void CryptoManager::decodeBZ2(const uint8_t* is, size_t sz, string& os) throw (CryptoException) {
	bz_stream bs = { 0 };

	if(BZ2_bzDecompressInit(&bs, 0, 0) != BZ_OK)
		throw(CryptoException(STRING(DECOMPRESSION_ERROR)));

	// We assume that the files aren't compressed more than 2:1...if they are it'll work anyway,
	// but we'll have to do multiple passes...
	size_t bufsize = 2*sz;
	AutoArray<char> buf(bufsize);

	bs.avail_in = sz;
	bs.avail_out = bufsize;
	bs.next_in = (char*)(const_cast<uint8_t*>(is));
	bs.next_out = buf;

	int err;

	os.clear();

	while((err = BZ2_bzDecompress(&bs)) == BZ_OK) {
		if (bs.avail_in == 0 && bs.avail_out > 0) { // error: BZ_UNEXPECTED_EOF
			BZ2_bzDecompressEnd(&bs);
			throw CryptoException(STRING(DECOMPRESSION_ERROR));
		}
		os.append(buf, bufsize-bs.avail_out);
		bs.avail_out = bufsize;
		bs.next_out = buf;
	}

	if(err == BZ_STREAM_END)
		os.append(buf, bufsize-bs.avail_out);

	BZ2_bzDecompressEnd(&bs);

	if(err < 0) {
		// This was a real error
		throw CryptoException(STRING(DECOMPRESSION_ERROR));
	}
}

string CryptoManager::keySubst(const uint8_t* aKey, size_t len, size_t n) {
	AutoArray<uint8_t> temp(len + n * 10);

	size_t j=0;

	for(size_t i = 0; i<len; i++) {
		if(isExtra(aKey[i])) {
			temp[j++] = '/'; temp[j++] = '%'; temp[j++] = 'D';
			temp[j++] = 'C'; temp[j++] = 'N';
			switch(aKey[i]) {
			case 0: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '0'; break;
			case 5: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '5'; break;
			case 36: temp[j++] = '0'; temp[j++] = '3'; temp[j++] = '6'; break;
			case 96: temp[j++] = '0'; temp[j++] = '9'; temp[j++] = '6'; break;
			case 124: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '4'; break;
			case 126: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '6'; break;
			}
			temp[j++] = '%'; temp[j++] = '/';
		} else {
			temp[j++] = aKey[i];
		}
	}
	return string((char*)(uint8_t*)temp, j);
}

string CryptoManager::makeKey(const string& aLock) {
	if(aLock.size() < 3)
		return Util::emptyString;

	AutoArray<uint8_t> temp(aLock.length());
	uint8_t v1;
	size_t extra=0;

	v1 = (uint8_t)(aLock[0]^5);
	v1 = (uint8_t)(((v1 >> 4) | (v1 << 4)) & 0xff);
	temp[0] = v1;

	string::size_type i;

	for(i = 1; i<aLock.length(); i++) {
		v1 = (uint8_t)(aLock[i]^aLock[i-1]);
		v1 = (uint8_t)(((v1 >> 4) | (v1 << 4))&0xff);
		temp[i] = v1;
		if(isExtra(temp[i]))
			extra++;
	}

	temp[0] = (uint8_t)(temp[0] ^ temp[aLock.length()-1]);

	if(isExtra(temp[0])) {
		extra++;
	}

	return keySubst(temp, aLock.length(), extra);
}


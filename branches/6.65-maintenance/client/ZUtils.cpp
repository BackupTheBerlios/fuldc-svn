/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#include "ZUtils.h"
#include "Exception.h"
#include "ResourceManager.h"

const double ZFilter::MIN_COMPRESSION_LEVEL = 0.9;

ZFilter::ZFilter() : totalIn(0), totalOut(0), compressing(true) {
	memset(&zs, 0, sizeof(zs));

	if(deflateInit(&zs, 3) != Z_OK) {
		throw Exception(STRING(COMPRESSION_ERROR));
	}
}

ZFilter::~ZFilter() {
	dcdebug("ZFilter end, %ld/%ld = %.04f\n", zs.total_out, zs.total_in, (float)zs.total_out / max((float)zs.total_in, (float)1)); 
	deflateEnd(&zs);
}

bool ZFilter::operator()(const void* in, size_t& insize, void* out, size_t& outsize) {
	if(outsize == 0)
		return 0;

	zs.avail_in = insize;
	zs.next_in = (Bytef*)in;
	zs.avail_out = outsize;
	zs.next_out = (Bytef*)out;

	if(insize == 0) {
		int err = ::deflate(&zs, Z_FINISH);
		if(err != Z_OK && err != Z_STREAM_END)
			throw Exception(STRING(COMPRESSION_ERROR));

		outsize = outsize - zs.avail_out;
		insize = insize - zs.avail_in;
		totalOut += outsize;
		totalIn += insize;
		return err == Z_OK;
	} else {
		int err = ::deflate(&zs, Z_NO_FLUSH);
		if(err != Z_OK)
			throw Exception(STRING(COMPRESSION_ERROR));

		outsize = outsize - zs.avail_out;
		insize = insize - zs.avail_in;
		totalOut += outsize;
		totalIn += insize;
		return true;
	}
}

UnZFilter::UnZFilter() {
	memset(&zs, 0, sizeof(zs));

	if(inflateInit(&zs) != Z_OK) 
		throw Exception(STRING(DECOMPRESSION_ERROR));

}

UnZFilter::~UnZFilter() {
	dcdebug("UnZFilter end, %ld/%ld = %.04f\n", zs.total_out, zs.total_in, (float)zs.total_out / max((float)zs.total_in, (float)1)); 
	inflateEnd(&zs);
}

bool UnZFilter::operator()(const void* in, size_t& insize, void* out, size_t& outsize) {
	if(outsize == 0)
		return 0;

	zs.avail_in = insize;
	zs.next_in = (Bytef*)in;
	zs.avail_out = outsize;
	zs.next_out = (Bytef*)out;

	int err = ::inflate(&zs, Z_NO_FLUSH);

	// see zlib/contrib/minizip/unzip.c, Z_BUF_ERROR means we should have padded
	// with a dummy byte if at end of stream - since we don't do this it's not a real
	// error
	if(!(err == Z_OK || err == Z_STREAM_END || (err == Z_BUF_ERROR && in == NULL)))
		throw Exception(STRING(DECOMPRESSION_ERROR));

	outsize = outsize - zs.avail_out;
	insize = insize - zs.avail_in;
	return err == Z_OK;
}

/**
 * @file
 * $Id: ZUtils.cpp,v 1.1 2004/02/14 13:27:24 trem Exp $
 */
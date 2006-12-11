/*
* Copyright (C) 2006 Pär Björklund, per.bjorklund@gmail.com
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

#if !defined(TRANSFER_INFO_H)
#define TRANSFER_INFO_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/User.h"

#include "WinUtil.h"
#include "UserInfoBase.h"

class TransferInfo : public UserInfoBase {
public:
	enum Status {
		STATUS_RUNNING,
		STATUS_WAITING
	};

	enum {
		COLUMN_FIRST,
		COLUMN_USER = COLUMN_FIRST,
		COLUMN_HUB,
		COLUMN_STATUS,
		COLUMN_TIMELEFT,
		COLUMN_TOTALTIMELEFT,
		COLUMN_SPEED,
		COLUMN_FILE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_IP,
		COLUMN_RATIO,
		COLUMN_LAST
	};

	TransferInfo(const User::Ptr& u, bool aDownload);

	bool download;
	bool transferFailed;
	bool filelist;
	Status status;
	int64_t pos;
	int64_t size;
	int64_t start;
	int64_t actual;
	int64_t speed;
	int64_t timeLeft;
	int64_t totalTimeLeft;

	tstring columns[COLUMN_LAST];

	void disconnect();
	void removeAll();
	void deleteSelf() { delete this; }

	double getRatio() { return (pos > 0) ? (double)actual / (double)pos : 1.0; }

	const tstring& getText(int col) const {
		dcassert(col >= 0 && col < COLUMN_LAST);
		return columns[col];
	}

	const tstring& copy(int col) {
		if(col >= 0 && col < COLUMN_LAST)
			return getText(col);

		return Util::emptyStringT;
	}

	static int compareItems(TransferInfo* a, TransferInfo* b, int col) {
		if(a->status == b->status) {
			if(a->download != b->download) {
				return a->download ? -1 : 1;
			}
		} else {
			return (a->status == TransferInfo::STATUS_RUNNING) ? -1 : 1;
		}
		switch(col) {
				case COLUMN_STATUS: return 0;
				case COLUMN_TIMELEFT: return compare(a->timeLeft, b->timeLeft);
				case COLUMN_TOTALTIMELEFT: return compare(a->totalTimeLeft, b->totalTimeLeft);
				case COLUMN_SPEED: return compare(a->speed, b->speed);
				case COLUMN_SIZE: return compare(a->size, b->size);
				case COLUMN_RATIO: return compare(a->getRatio(), b->getRatio());
				default: return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
		}
	}

	bool operator==(const TransferInfo& ti) const {
		return this->user == ti.user && this->download == ti.download;
	}
};

#endif // TRANSFER_INFO_H
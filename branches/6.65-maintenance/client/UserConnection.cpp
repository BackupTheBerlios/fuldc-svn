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
#include "ResourceManager.h"

#include "UserConnection.h"
#include "ClientManager.h"
#include "LogManager.h"

#include "StringTokenizer.h"
#include "AdcCommand.h"

const string UserConnection::FEATURE_MINISLOTS = "MiniSlots";
const string UserConnection::FEATURE_XML_BZLIST = "XmlBZList";
const string UserConnection::FEATURE_ADCGET = "ADCGet";
const string UserConnection::FEATURE_ZLIB_GET = "ZLIG";
const string UserConnection::FEATURE_TTHL = "TTHL";
const string UserConnection::FEATURE_TTHF = "TTHF";

const string UserConnection::FILE_NOT_AVAILABLE = "File Not Available";
const string UserConnection::COMMAND_NOT_SUPPORTED = "Client too old, not supported";

const string UserConnection::UPLOAD = "Upload";
const string UserConnection::DOWNLOAD = "Download";

void Transfer::updateRunningAverage() {
	u_int32_t tick = GET_TICK();
	if(tick > lastTick) {
		u_int32_t diff = tick - lastTick;
		int64_t tot = getTotal();
		if(diff == 0) {
			// No time passed, don't update runningAverage;
		} else if( ((tick - getStart()) < AVG_PERIOD) ) {
			runningAverage = getAverageSpeed();
		} else {
			int64_t bdiff = tot - last;
			int64_t avg = bdiff * (int64_t)1000 / diff;
			if(diff > AVG_PERIOD) {
				runningAverage = avg;
			} else {
				// Weighted average...
				runningAverage = ((avg * diff) + (runningAverage*(AVG_PERIOD-diff)))/AVG_PERIOD;
			}
		}
		last = tot;
	}
	lastTick = tick;
}

void UserConnection::on(BufferedSocketListener::Line, const string& aLine) throw () {

	if(aLine.length() < 2)
		return;

	if(aLine[0] == 'C' && !isSet(FLAG_NMDC)) {
		dispatch(aLine);
		return;
	} else if(aLine[0] == '$') {
		setFlag(FLAG_NMDC);
	} else {
		// We shouldn't be here?
		dcdebug("Unknown UserConnection command: %.50s\n", aLine.c_str());
		return;
	}
	string cmd;
	string param;

	string::size_type x;
	
	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		param = aLine.substr(x+1);
	}
	
	if(cmd == "$MyNick") {
		if(!param.empty())
			fire(UserConnectionListener::MyNick(), this, Text::acpToUtf8(param));
	} else if(cmd == "$Direction") {
		x = param.find(" ");
		if(x != string::npos) {
			fire(UserConnectionListener::Direction(), this, param.substr(0, x), param.substr(x+1));
		}
	} else if(cmd == "$Error") {
		if(Util::stricmp(param.c_str(), FILE_NOT_AVAILABLE) == 0 || 
			param.rfind(/*path/file*/" no more exists") != string::npos) { 
			fire(UserConnectionListener::FileNotAvailable(), this);
		} else {
			fire(UserConnectionListener::Failed(), this, param);
		}
	} else if(cmd == "$FileLength") {
		if(!param.empty())
			fire(UserConnectionListener::FileLength(), this, Util::toInt64(param));
	} else if(cmd == "$GetListLen") {
		fire(UserConnectionListener::GetListLength(), this);
	} else if(cmd == "$Get") {
		notSupported();
		disconnect();
		StringMap params;
		params["user"] = getUser()->getNick();
		params["hub"] = getUser()->getClientAddressPort();
		params["ip"] = getRemoteIp();
		string tmp = Util::formatParams(STRING(OLD_CLIENT), params, false);
		LogManager::getInstance()->message(tmp);
	} else if(cmd == "$GetZBlock" || cmd == "$UGetZBlock" || cmd == "$UGetBlock") {
		notSupported();
		disconnect();
		StringMap params;
		params["user"] = getUser()->getNick();
		params["hub"] = getUser()->getClientAddressPort();
		params["ip"] = getRemoteIp();
		string tmp = Util::formatParams(STRING(OLD_CLIENT), params, false);
		LogManager::getInstance()->message(tmp);
	} else if(cmd == "$Key") {
		if(!param.empty())
			fire(UserConnectionListener::Key(), this, param);
	} else if(cmd == "$Lock") {
		if(!param.empty()) {
			x = param.find(" Pk=");
			if(x != string::npos) {
				fire(UserConnectionListener::CLock(), this, param.substr(0, x), param.substr(x + 4));
			} else {
				// Workaround for faulty linux clients...
				x = param.find(' ');
				if(x != string::npos) {
					setFlag(FLAG_INVALIDKEY);
					fire(UserConnectionListener::CLock(), this, param.substr(0, x), Util::emptyString);
				} else {
					fire(UserConnectionListener::CLock(), this, param, Util::emptyString);
				}
			}
		}
	} else if(cmd == "$Send") {
		fire(UserConnectionListener::Send(), this);
	} else if(cmd == "$Sending") {
		int64_t bytes = -1;
		if(!param.empty())
			bytes = Util::toInt64(param);
		fire(UserConnectionListener::Sending(), this, bytes);
	} else if(cmd == "$MaxedOut") {
		fire(UserConnectionListener::MaxedOut(), this);
	} else if(cmd == "$Supports") {
		if(!param.empty()) {
			fire(UserConnectionListener::Supports(), this, StringTokenizer<string>(param, ' ').getTokens());
		}
	} else if(cmd.compare(0, 4, "$ADC") == 0) {
		dispatch(aLine, true);
	} else {
		dcdebug("Unknown NMDC command: %.50s\n", aLine.c_str());
	}
}

void UserConnection::connect(const string& aServer, short aPort) throw(SocketException, ThreadException) { 
	dcassert(!socket);

	socket = BufferedSocket::getSocket(0);
	socket->addListener(this);
	socket->connect(aServer, aPort, secure, true);
}

void UserConnection::accept(const Socket& aServer) throw(SocketException, ThreadException) {
	dcassert(!socket);
	socket = BufferedSocket::getSocket(0);
	socket->addListener(this);
	socket->accept(aServer, secure);
}

void UserConnection::inf(bool withToken) { 
	/*AdcCommand c(AdcCommand::CMD_INF);
	c.addParam("CI", ClientManager::getInstance()->getMe()->getCID().toBase32());
	if(withToken) {
		c.addParam("TO", getToken());
	}
	send(c);
	*/
}

void UserConnection::on(BufferedSocketListener::Failed, const string& aLine) throw() {
	setState(STATE_UNCONNECTED);
	fire(UserConnectionListener::Failed(), this, aLine);

	delete this;
}

/**
 * @file
 * $Id: UserConnection.cpp,v 1.4 2004/02/25 00:27:26 trem Exp $
 */

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

#if !defined(NMDC_HUB_H)
#define NMDC_HUB_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"
#include "SettingsManager.h"

#include "User.h"
#include "CriticalSection.h"
#include "Text.h"
#include "Client.h"

class ClientManager;

class NmdcHub : public Client, private Flags
{
public:
	using Client::send;

	typedef NmdcHub* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	virtual void connect();
	virtual void connect(const User::Ptr aUser);
	
#define checkstate() if(state != STATE_CONNECTED) return

	virtual void hubMessage(const string& aMessage) { checkstate(); send(toNmdc( "<" + getNick() + "> " + validateMessage(aMessage, false) + "|" ) ); }
	virtual void privateMessage(const User* aUser, const string& aMessage) { privateMessage(aUser->getNick(), string("<") + getNick() + "> " + aMessage); }
	virtual void sendUserCmd(const string& aUserCmd) throw() {
		send(toNmdc(aUserCmd));
	}
	virtual void search(int aSizeType, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	virtual void password(const string& aPass) { send("$MyPass " + toNmdc(aPass) + "|"); }
	virtual void info(bool force) { myInfo(force); }
	
	virtual size_t getUserCount() const {  Lock l(cs); return users.size(); }
	virtual int64_t getAvailable() const;
	virtual const string& getName() const { return BOOLSETTING(REMOVE_TOPIC) ? shortName : name; };
	virtual const string& getNameWithTopic() const { return name; }
	virtual bool getOp() const { return getMe() ? getMe()->isSet(User::OP) : false; };

	virtual User::NickMap& lockUserList() { cs.enter(); return users; };
	virtual void unlockUserList() { cs.leave(); };

	virtual string escape(string const& str) const { return validateMessage(str, false); };

	void myInfo(bool alwaysSend);
	
	void refreshUserList(bool unknownOnly = false);

	void validateNick(const string& aNick) { send("$ValidateNick " + toNmdc(aNick) + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); };	
	void version() { send("$Version 1,0091|"); };
	void getNickList() { checkstate(); send("$GetNickList|"); };
	void getInfo(User::Ptr aUser) { checkstate(); send("$GetINFO " + toNmdc(aUser->getNick()) + " " + toNmdc(getNick()) + "|"); };
	void getInfo(User* aUser) {  checkstate(); send("$GetINFO " + toNmdc(aUser->getNick()) + " " + toNmdc(getNick()) + "|"); };

	void privateMessage(const User::Ptr& aUser, const string& aMessage) {
		privateMessage(aUser->getNick(), string("<") + getNick() + "> " + aMessage);
	}
	void privateMessage(const string& aNick, const string& aMessage) {
		checkstate(); 
		send("$To: " + toNmdc(aNick) + " From: " + toNmdc(getNick()) + " $" + toNmdc(validateMessage(aMessage, false)) + "|");
	}
	void supports(const StringList& feat) { 
		string x;
		for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
			x+= *i + ' ';
		}
		send("$Supports " + x + '|');
	}

	static string validateMessage(string tmp, bool reverse, bool checkNewLines = true);

	GETSET(int, supportFlags, SupportFlags);
private:
	friend class ClientManager;
	enum SupportFlags {
		SUPPORTS_USERCOMMAND = 0x01,
		SUPPORTS_NOGETINFO = 0x02,
		SUPPORTS_USERIP2 = 0x04
	};

	enum States {
		STATE_CONNECT,
		STATE_LOCK,
		STATE_HELLO,
		STATE_CONNECTED
	} state;

	string name;
	string shortName;

	mutable CriticalSection cs;

	User::NickMap users;

	u_int32_t lastUpdate;
	string lastMyInfoA, lastMyInfoB;

	typedef list<pair<string, u_int32_t> > FloodMap;
	typedef FloodMap::iterator FloodIter;
	FloodMap seekers;
	FloodMap flooders;

	NmdcHub(const string& aHubURL);	
	virtual ~NmdcHub() throw();

	// Dummy
	NmdcHub(const NmdcHub&);
	NmdcHub& operator=(const NmdcHub&);

	void clearUsers();
	void onLine(const string& aLine) throw();

	string fromNmdc(const string& str) const { return Text::acpToUtf8(str); }
	string toNmdc(const string& str) const { return Text::utf8ToAcp(str); }

	virtual string checkNick(const string& aNick);

	// TimerManagerListener
	virtual void on(Second, u_int32_t aTick) throw();

	virtual void on(Line, const string& l) throw() { onLine(l); }
	virtual void on(Failed, const string&) throw();

};

#endif // !defined(NMDC_HUB_H)

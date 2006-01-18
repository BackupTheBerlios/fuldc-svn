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

#if !defined(CLIENT_H)
#define CLIENT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "User.h"
#include "BufferedSocket.h"
#include "SettingsManager.h"

class Client;
class AdcCommand;

class ClientListener  
{
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Connecting;
	typedef X<1> Connected;
	typedef X<2> BadPassword;
	typedef X<3> UserUpdated;
	typedef X<4> UsersUpdated;
	typedef X<5> UserRemoved;
	typedef X<6> Redirect;
	typedef X<7> Failed;
	typedef X<8> GetPassword;
	typedef X<9> HubUpdated;
	typedef X<11> Message;
	typedef X<12> StatusMessage;
	typedef X<13> PrivateMessage;
	typedef X<14> UserCommand;
	typedef X<15> HubFull;
	typedef X<16> NickTaken;
	typedef X<17> SearchFlood;
	typedef X<18> NmdcSearch;
	typedef X<19> AdcSearch;
	typedef X<20> UserIp;

	virtual void on(Connecting, Client*) throw() { }
	virtual void on(Connected, Client*) throw() { }
	virtual void on(BadPassword, Client*) throw() { }
	virtual void on(UserUpdated, Client*, const OnlineUser&) throw() { }
	virtual void on(UsersUpdated, Client*, const OnlineUser::List&) throw() { }
	virtual void on(UserRemoved, Client*, const OnlineUser&) throw() { }
	virtual void on(Redirect, Client*, const string&) throw() { }
	virtual void on(Failed, Client*, const string&) throw() { }
	virtual void on(GetPassword, Client*) throw() { }
	virtual void on(HubUpdated, Client*) throw() { }
	virtual void on(Message, Client*, const OnlineUser&, const string&) throw() { }
	virtual void on(StatusMessage, Client*, const string&) throw() { }
	virtual void on(PrivateMessage, Client*, const OnlineUser&, const OnlineUser&, const OnlineUser&, const string&) throw() { }
	virtual void on(UserCommand, Client*, int, int, const string&, const string&) throw() { }
	virtual void on(HubFull, Client*) throw() { }
	virtual void on(NickTaken, Client*) throw() { }
	virtual void on(SearchFlood, Client*, const string&) throw() { }
	virtual void on(NmdcSearch, Client*, const string&, int, int64_t, int, const string&) throw() { }
	virtual void on(AdcSearch, Client*, const AdcCommand&) throw() { }
	virtual void on(UserIp, Client*, const User::List&) throw() { }
};

/** Yes, this should probably be called a Hub */
class Client : public Speaker<ClientListener>, public BufferedSocketListener {
public:
	typedef Client* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	Client(const string& hubURL, char separator, bool secure_);
	virtual ~Client() throw();

	virtual void connect();
	bool isConnected() const { return socket && socket->isConnected(); }
	void disconnect(bool graceless) { if(socket) socket->disconnect(graceless); }

	virtual void connect(const OnlineUser& user) = 0;
	virtual void hubMessage(const string& aMessage) = 0;
	virtual void privateMessage(const OnlineUser& user, const string& aMessage) = 0;
	virtual void sendUserCmd(const string& aUserCmd) = 0;
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) = 0;
	virtual void password(const string& pwd) = 0;
	virtual void info(bool force) = 0;
    
	virtual size_t getUserCount() const = 0;
	virtual int64_t getAvailable() const = 0;
	
	bool isOp() const { return getMyIdentity().isOp(); }

	short getPort() const { return port; }
	const string& getAddress() const { return address; }

	const string& getIp() const { return (!socket || socket->getIp().empty()) ? getAddress() : socket->getIp(); };
	string getIpPort() const { return getIp() + ':' + Util::toString(port); };
	string getLocalIp() const;

	void updated(const OnlineUser& aUser) { 
		fire(ClientListener::UserUpdated(), this, aUser);
	}

	static string getCounts() {
		char buf[128];
		return string(buf, sprintf(buf, "%ld/%ld/%ld", counts.normal, counts.registered, counts.op));
	}

	virtual string escape(string const& str) const { return str; };
	StringMap& escapeParams(StringMap& sm) {
		for(StringMapIter i = sm.begin(); i != sm.end(); ++i) {
			i->second = escape(i->second);
		}
		return sm;
	}

	void send(const string& aMessage) { send(aMessage.c_str(), aMessage.length()); }
	void send(const char* aMessage, size_t aLen) {
		dcassert(socket);
		if(!socket)
			return;
		updateActivity();
		socket->write(aMessage, aLen);
	}
	const string& getMyNick() const { return getMyIdentity().getNick(); }
	const string& getHubName() const { return getHubIdentity().getNick().empty() ? getHubUrl() : getHubIdentity().getNick(); }
	const string& getHubDescription() const { return getHubIdentity().getDescription(); }
	const string getHubNameWithDescription() { return getHubName() + " - " + getHubDescription(); }

	Identity& getMyIdentity() { return myIdentity; }
	Identity& getHubIdentity() { return hubIdentity; }

	const string& getHubUrl() const { return hubUrl; }

	GETSET(Identity, myIdentity, MyIdentity);
	GETSET(Identity, hubIdentity, HubIdentity);

	GETSET(string, defpassword, Password);
	GETSET(u_int32_t, reconnDelay, ReconnDelay);
	GETSET(u_int32_t, lastActivity, LastActivity);
	GETSET(bool, registered, Registered);

protected:
	struct Counts {
		Counts(long n = 0, long r = 0, long o = 0) : normal(n), registered(r), op(o) { };
		volatile long normal;
		volatile long registered;
		volatile long op;
		bool operator !=(const Counts& rhs) { return normal != rhs.normal || registered != rhs.registered || op != rhs.op; };
	};

	BufferedSocket* socket;

	static Counts counts;
	Counts lastCounts;

	void updateCounts(bool aRemove);
	void updateActivity();

	// reload nick from settings, other details from favmanager
	void reloadSettings();

	virtual string checkNick(const string& nick) = 0;

private:

	enum CountType {
		COUNT_UNCOUNTED,
		COUNT_NORMAL,
		COUNT_REGISTERED,
		COUNT_OP
	};

	Client(const Client&);
	Client& operator=(const Client&);

	string hubUrl;
	string address;
	u_int16_t port;
	char separator;
	bool secure;

	CountType countType;

	// BufferedSocketListener
	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw() { updateActivity(); fire(ClientListener::Connected(), this); }
};

#endif // !defined(CLIENT_H)

/**
 * @file
 * $Id: Client.h,v 1.5 2004/02/14 13:24:02 trem Exp $
 */

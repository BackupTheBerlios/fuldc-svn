/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ShareManager.h"
#include "SearchManager.h"
#include "QueueManager.h"
#include "ClientManager.h"
#include "HashManager.h"
#include "LogManager.h"
#include "HubManager.h"
#include "SettingsManager.h"
#include "FinishedManager.h"
#include "ADLSearch.h"
#include "Webshortcuts.h"
#include "HighlightManager.h"
#include "..\windows\PopupManager.h"

#include "StringTokenizer.h"

void startup(void (*f)(void*, const string&, const string&), void* p) {
	// "Dedicated to the near-memory of Nev. Let's start remembering people while they're still alive."
	// Nev's great contribution to dc++
	while(1) break;
	
	string tku[9] = {
		"Fastkedjad i templet, ormar �verallt, kukar �verallt\r\n",
		"Jag s�g en sk�ggig herre, han var t�ckt av s�d\r\n", 
		"L�der och homosex, r�tt vin och havrekex, det vill vi ha!\r\n",
		"Jag f�rdas naken, allt �r som sig b�r\r\n",
		"Polisen och S�PO de jagar oss, agenterna sl�pper hundarna loss\r\n",
		"Sprit och gott, massor av flott, pang p� pungen i Portugal!\r\n",
		"Den kristna tjackfabriken puttrar p�, samh�llet vet ej vad som p�g�r\r\n",
		"Arbetsl�s och utan tjej, d� blir det en fet j�vla holk och en bitelinepizza\r\n",
		"B�rs som b�rs, det smakar ju lika\r\n",
	};
	Util::initialize();
	int index = Util::rand(9);

	ResourceManager::newInstance();
	SettingsManager::newInstance();

	LogManager::newInstance();
	TimerManager::newInstance();
	HashManager::newInstance();
	HighlightManager::newInstance();
	ShareManager::newInstance();
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	HubManager::newInstance();
	QueueManager::newInstance();
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();
	PopupManager::newInstance();
	WebShortcuts::newInstance();

	SettingsManager::getInstance()->load();

	WebShortcuts::getInstance()->load();

	if(!SETTING(LANGUAGE_FILE).empty()) {
		ResourceManager::getInstance()->loadLanguage(SETTING(LANGUAGE_FILE));
	}

	HubManager::getInstance()->load();
	
	double v = Util::toDouble(SETTING(CONFIG_VERSION));
	if(v <= 0.22) {
		// Disable automatic public hublist opening
		SettingsManager::getInstance()->set(SettingsManager::OPEN_PUBLIC, false);
	}
	if(v <= 0.251) {
		StringTokenizer st(SETTING(HUBLIST_SERVERS), ';');
		StringList& sl = st.getTokens();
		StringList sl2;
		bool defFound = false;
		StringIter si;
		for(si = sl.begin(); si != sl.end(); ++si) {
			if((si->find("http://dcplusplus.sourceforge.net") != string::npos) ||
				(si->find("http://dcpp.lichlord.org") != string::npos))
			{
				if(!defFound) {
					sl2.push_back("http://www.hublist.org/PublicHubList.config.bz2");
					defFound = true;
				}
			} else {
				sl2.push_back(*si);
			}
		}
		string tmp;
		for(si = sl2.begin(); si != sl2.end(); ++si) {
			tmp += *si + ';';
		}

		if(!tmp.empty()) {
			tmp.erase(tmp.length()-1);
			SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, tmp);
		}
	}

	if(f != NULL)
		(*f)(p, tku[index], STRING(HASH_DATABASE));
	HashManager::getInstance()->startup();
	if(f != NULL)
		(*f)(p, tku[index], STRING(SHARED_FILES));

	if(ShareManager::getInstance()->loadXmlList()){
		ShareManager::getInstance()->refresh(false, true, true, false, false, false);
	} else {
		ShareManager::getInstance()->refresh(false, false, true, false, false, true);
	}

	if(f != NULL)
		(*f)(p, tku[index], STRING(DOWNLOAD_QUEUE));
	QueueManager::getInstance()->loadQueue();

}

void shutdown() {
	ConnectionManager::getInstance()->shutdown();

	WebShortcuts::getInstance()->save();
	HashManager::getInstance()->shutdown();

	TimerManager::getInstance()->removeListeners();
	SettingsManager::getInstance()->save();
	
	ADLSearchManager::deleteInstance();
	FinishedManager::deleteInstance();
	HighlightManager::deleteInstance();
	ShareManager::deleteInstance();
	CryptoManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	QueueManager::deleteInstance();
	ConnectionManager::deleteInstance();
	SearchManager::deleteInstance();
	ClientManager::deleteInstance();
	HubManager::deleteInstance();
	HashManager::deleteInstance();
	LogManager::deleteInstance();
	SettingsManager::deleteInstance();
	TimerManager::deleteInstance();
	ResourceManager::deleteInstance();
}

/**
 * @file
 * $Id: DCPlusPlus.cpp,v 1.5 2004/02/15 01:20:30 trem Exp $
 */


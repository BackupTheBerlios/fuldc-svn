/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "HighlightManager.h"
#include "WebShortcuts.h"

//ugly, but i have no intention on moving the code to *nix so it works =)
#include "../windows/PopupManager.h"


#include "StringTokenizer.h"

void startup(void (*f)(void*, const tstring&, const tstring&), void* p) {
	// "Dedicated to the near-memory of Nev. Let's start remembering people while they're still alive."
	// Nev's great contribution to dc++
	while(1) break;
	
	tstring tku[14] = {
		_T("Fastkedjad i templet, ormar överallt, kukar överallt\r\n"),
		_T("Jag såg en skäggig herre, han var täckt av säd\r\n"), 
		_T("Läder och homosex, rött vin och havrekex, det vill vi ha!\r\n"),
		_T("Fjolla och aristokrat, nobelt blod och lyxig mat, det vill vi ha!\r\n"),
		_T("Jag färdas naken, allt är som sig bör\r\n"),
		_T("Polisen och SÄPO de jagar oss, agenterna släpper hundarna loss\r\n"),
		_T("Sprit och gott, massor av flott, pang på pungen i Portugal!\r\n"),
		_T("Den kristna tjackfabriken puttrar på, samhället vet ej vad som pågår\r\n"),
		_T("Arbetslös och utan tjej, då blir det en fet jävla holk och en bitelinepizza\r\n"),
		_T("Bärs som bärs, det smakar ju lika, man suger bärs så man blir full och ramlar kull\r\n"),
		_T("Duschring, duschring, det är vad jag vill ha runt taskapåsen\r\n"),
		_T("Äta olja, dricka sand, spela i arabiska heavymetalband\r\n"),
		_T("Mellan Hudik och Sundsvall - det brutala Gnarp!\r\n"),
		_T("Himmelriket i Norrland - det totala Gnarp!\r\n"),
	};
	Util::initialize();
	int index = Util::rand(14);

	ResourceManager::newInstance();
	SettingsManager::newInstance();
	
	if(f != NULL)
		(*f)(p, tku[index], TSTRING(SETTINGS));

	LogManager::newInstance();
	TimerManager::newInstance();
	HashManager::newInstance();
	HighlightManager::newInstance();
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	ShareManager::newInstance();
	HubManager::newInstance();
	QueueManager::newInstance();
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();
	PopupManager::newInstance();
	WebShortcuts::newInstance();
	
	SettingsManager::getInstance()->load();
	SettingsManager::getInstance()->setDefault(SettingsManager::UDP_PORT, SETTING(IN_PORT));

	if(!SETTING(LANGUAGE_FILE).empty()) {
		ResourceManager::getInstance()->loadLanguage(SETTING(LANGUAGE_FILE));
	}

	HubManager::getInstance()->load();

	if(f != NULL)
		(*f)(p, tku[index], TSTRING(HASH_DATABASE));
	HashManager::getInstance()->startup();
	if(f != NULL)
		(*f)(p, tku[index], TSTRING(SHARED_FILES));

	if( !ShareManager::getInstance()->loadXmlList() ){
		ShareManager::getInstance()->refresh(true, false, true);
	}

	if(f != NULL)
		(*f)(p, tku[index], TSTRING(DOWNLOAD_QUEUE));
	QueueManager::getInstance()->loadQueue();

}

void shutdown() {
	ConnectionManager::getInstance()->shutdown();
	HashManager::getInstance()->shutdown();

	TimerManager::getInstance()->removeListeners();
	SettingsManager::getInstance()->save();
	
	WebShortcuts::deleteInstance();
	PopupManager::deleteInstance();
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


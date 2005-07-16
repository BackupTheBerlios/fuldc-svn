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
#include "FavoriteManager.h"
#include "SettingsManager.h"
#include "FinishedManager.h"
#include "ADLSearch.h"
#include "HighlightManager.h"
#include "WebShortcuts.h"
#include "IgnoreManager.h"

//ugly, but i have no intention on moving the code to *nix so it works =)
#include "../windows/PopupManager.h"


#include "StringTokenizer.h"

void startup(void (*f)(void*, const tstring&, const tstring&), void* p) {
	// "Dedicated to the near-memory of Nev. Let's start remembering people while they're still alive."
	// Nev's great contribution to dc++
	while(1) break;
	
	tstring tku[] = {
		_T("Fastkedjad i templet, ormar �verallt, kukar �verallt\r\n"),
		_T("Jag s�g en sk�ggig herre, han var t�ckt av s�d\r\n"), 
		_T("L�der och homosex, r�tt vin och havrekex, det vill vi ha!\r\n"),
		_T("Fjolla och aristokrat, nobelt blod och lyxig mat, det vill vi ha!\r\n"),
		_T("Jag f�rdas naken, allt �r som sig b�r\r\n"),
		_T("Polisen och S�PO de jagar oss, agenterna sl�pper hundarna loss\r\n"),
		_T("Sprit och gott, massor av flott, pang p� pungen i Portugal!\r\n"),
		_T("Den kristna tjackfabriken puttrar p�, samh�llet vet ej vad som p�g�r\r\n"),
		_T("Arbetsl�s och utan tjej, d� blir det en fet j�vla holk och en bitelinepizza\r\n"),
		_T("B�rs som b�rs, det smakar ju lika, man suger b�rs s� man blir full och ramlar kull\r\n"),
		_T("Duschring, duschring, det �r vad jag vill ha runt taskap�sen\r\n"),
		_T("�ta olja, dricka sand, spela i arabiska heavymetalband\r\n"),
		_T("Mellan Hudik och Sundsvall - det brutala Gnarp!\r\n"),
		_T("Himmelriket i Norrland - det totala Gnarp!\r\n"),
		_T("<[Slampa]Monolina> Hellre en slampa �n en sockertopp :/\r\n"),
		_T("<[Sockertopp]Monolina> Jag tycker illa om Irean nu.. ��h vad hon �r dum :(\r\n"),
		_T("<[Sockertopp]Monolina> Dummadummadummadummadumma Irean! *pekar p� sitt prefix och gr�ter*\r\n"),
		_T("<[Sockertopp]Monolina> ZooR r�dda mig!\r\n"),
		_T("<[Rosenknopp]Monolina> Ni - *pekar anklagande p� alla tanter* - �r onda! *gny*\r\n"),
		_T("<[Rosenknopp]Monolina> Ni sabbar mitt rykte och min badboy-image\r\n"),
		_T("<[Rosenknopp]Monolina> Mitt prefix �r en skam\r\n"),
		_T("<[Rosenknopp]Monolina> Jag �r en POJKE\r\n"),
		_T("<[Rosenknopp]Monolina> *tittar under Whoevers kjol*\r\n"),
		_T("<[Rosenknopp]Monolina  spelar Edguy och t�nker p� ZooR\r\n"),
		_T("<[KasslerLover]Monolina> Siren hj�lp mig\r\n"),
		_T("<[SexySingle]Monolina> elakisar *peka p� Irean* och tystisar *peka p� resten av er*\r\n"),
		_T("<[Wet&Kinky]Monolina> Men.. Du ska ju stoppa sedlar i mina kalsonger ju\r\n"),
		_T("<[Wet&Kinky]Monolina> *lyder Siren*\r\n"),
		_T("<[Wet&Kinky]Monolina> /me springer naken och skrikande genom chatten\r\n"),
		_T("<[Wet&Kinky]Monolina> Irean �r elak :/\r\n"),
		_T("<[Sockers�t]Monolina> Elaka Irean\r\n"),
		_T("<[Spirit]Mono> S� om du �r sugen s�.. I'm open for suggestions.\r\n"),
		_T("<[Brudn�bb]Monolina> *surar*\r\n"),
		_T("<[Spirit]Mono> Men vafan.. n�r du eggar mig s�d�r s�.. vem kan motst�?\r\n"),
		_T("<[Brudn�bb]Monolina> Men h�rddiskar �r ju det som lockar folk mest\r\n"),
		_T("<[Brudn�bb]Monolina> *cries like a baby*\r\n"),
		_T("<[Brudn�bb]Monolina> Jag �r Normal\r\n"),
		_T("<[Spirit]Mono> F�nga kor? Jag f�rs�ker ju f�nga Siren.. det �r v�l ungef�r samma sak?\r\n")
	};
	Util::initialize();
	int index = Util::rand(38);

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
	FavoriteManager::newInstance();
	QueueManager::newInstance();
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();
	PopupManager::newInstance();
	WebShortcuts::newInstance();
	IgnoreManager::newInstance();

	
	SettingsManager::getInstance()->load();

	if(!SETTING(LANGUAGE_FILE).empty()) {
		ResourceManager::getInstance()->loadLanguage(SETTING(LANGUAGE_FILE));
	}

	FavoriteManager::getInstance()->load();

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
	
	IgnoreManager::deleteInstance();
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
	FavoriteManager::deleteInstance();
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

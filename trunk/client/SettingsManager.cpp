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

#include "SettingsManager.h"
#include "ResourceManager.h"

#include "SimpleXML.h"
#include "Util.h"
#include "File.h"
#include "version.h"
#include "CID.h"
#include "StringTokenizer.h"

StringList SettingsManager::connectionSpeeds;

const string SettingsManager::settingTags[] =
{
	// Strings
	"Nick", "UploadSpeed", "Description", "DownloadDirectory", "EMail", "ExternalIp",
	"Font", "MainFrameOrder", "MainFrameWidths", "HubFrameOrder", "HubFrameWidths", 
	"LanguageFile", "SearchFrameOrder", "SearchFrameWidths", "FavoritesFrameOrder", "FavoritesFrameWidths", 
	"HublistServers", "QueueFrameOrder", "QueueFrameWidths", "PublicHubsFrameOrder", "PublicHubsFrameWidths", 
	"UsersFrameOrder", "UsersFrameWidths", "HttpProxy", "LogDirectory", "NotepadText", "LogFormatPostDownload",
	"LogFormatPostUpload", "LogFormatMainChat", "LogFormatPrivateChat", "FinishedOrder", "FinishedWidths",	
	"TempDownloadDirectory", "BindAddress", "SocksServer", "SocksUser", "SocksPassword", "ConfigVersion",
	"DefaultAwayMessage", "TimeStampsFormat", "ADLSearchFrameOrder", "ADLSearchFrameWidths", 
	"FinishedULWidths", "FinishedULOrder", "CID", "SpyFrameWidths", "SpyFrameOrder", "LogFileMainChat", 
	"LogFilePrivateChat", "LogFileStatus", "LogFileUpload", "LogFileDownload", "LogFileSystem", 
	"LogFormatSystem", "LogFormatStatus", "DirectoryListingFrameOrder", "DirectoryListingFrameWidths",
	"TLSPrivateKeyFile", "TLSCertificateFile", "TLSTrustedCertificatesPath",

	"DownloadSkiplist", "ShareSkiplist", "PopupFont", "FreeSlotsExtentions",
	"HubFrameVisible", "MainFrameVisible", "SearchFrameVisible",
	"QueueFrameVisible", "DirectoryListingFrameVisible","FinishedVisible", "FinishedULVisible",
	"HighPrioFiles", 
	"SENTRY", 
	// Ints
	"IncomingConnections", "InPort", "Slots", "Rollback", "AutoFollow", "ClearSearch",
	"BackgroundColor", "TextColor", "UseOemMonoFont", "ShareHidden", "FilterMessages", "MinimizeToTray",
	"AutoSearch", "TimeStamps", "ConfirmExit", "PopupHubPms", "PopupBotPms", "IgnoreHubPms", "IgnoreBotPms",
	"ListDuplicates", "BufferSize", "DownloadSlots", "MaxDownloadSpeed", "LogMainChat", "LogPrivateChat",
	"LogDownloads", "LogUploads", "StatusInChat", "ShowJoins", "PrivateMessageBeep", "PrivateMessageBeepOpen",
	"UseSystemIcons", "PopupPMs", "MinUploadSpeed", "GetUserInfo", "UrlHandler", "MainWindowState", 
	"MainWindowSizeX", "MainWindowSizeY", "MainWindowPosX", "MainWindowPosY", "AutoAway",
	"SocksPort", "SocksResolve", "KeepLists", "AutoKick", "QueueFrameShowTree",
	"CompressTransfers", "ShowProgressBars", "SFVCheck", "MaxTabRows",
	"MaxCompression", "AntiFrag", "MDIMaxmimized", "NoAwayMsgToBots",
	"SkipZeroByte", "AdlsBreakOnFirst", 
	"HubUserCommands", "AutoSearchAutoMatch", "DownloadBarColor", "UploadBarColor", "LogSystem",
	"LogFilelistTransfers", "SendUnknownCommands", "MaxHashSpeed", "OpenUserCmdHelp",
	"GetUserCountry", "FavShowJoins", "LogStatusMessages", "ShowStatusbar",
	"ShowToolbar", "ShowTransferview", "PopunderPm", "PopunderFilelist", "MagnetAsk", "MagnetAction", "MagnetRegister",
	"AddFinishedInstantly", "DontDLAlreadyShared", "UseCTRLForLineHistory", "ConfirmHubRemoval", 
	"OpenNewWindow", "UDPPort", "SearchOnlyTTH", "ShowLastLinesLog", "ConfirmItemRemoval",
	"AdvancedResume", "AdcDebug", "ToggleActiveWindow", "SearchHistory", "SetMinislotSize", "MaxFilelistSize", 
	"HighestPrioSize", "HighPrioSize", "NormalPrioSize", "LowPrioSize", "LowestPrio", 
	"AutoDropSpeed", "AutoDropInterval", "AutoDropElapsed", "AutoDropInactivity", "AutoDropMinSources", "AutoDropFilesize", 
	"AutoDropAll", "AutoDropFilelists", "AutoDropDisconnect", 
	"OpenPublic", "OpenFavoriteHubs", "OpenFavoriteUsers", "OpenQueue", "OpenFinishedDownloads",
	"OpenFinishedUploads", "OpenSearchSpy", "OpenNetworkStatistics", "OpenNotepad", "OutgoingConnections",
	"NoIpOverride", "SearchOnlyFreeSlots", "LastSearchType", "BoldFinishedDownloads", "BoldFinishedUploads", "BoldQueue", 
	"BoldHub", "BoldPm", "BoldSearch", "SocketInBuffer", "SocketOutBuffer", "OnlyDlTthFiles", 
	"OpenWaitingUsers", "BoldWaitingUsers", "OpenSystemLog", "BoldSystemLog", "AutoUpdateList",
	"UseTLS", "AutoSearchLimit", "AltSortOrder", "AutoKickNoFavs", "PromptPassword", "SpyFrameIgnoreTthSearches", 
	"DontDlAlreadyQueued", "MaxCommandLength", "AllowUntrustedHubs", "AllowUntrustedClients",
	"TLSPort",

	"IncomingRefreshTime", "ShareRefreshTime", "ChatBuffersize", "AutoUpdateIncoming", 
	"ExpandQueue", "StripIsp", "StripIspPm", "HubBoldTabs",
	"PopupTimeout", "PopupAway", "PopupMinimized", "PopupPm", "PopupNewPm", "PopupHubStatus", 
	"HubFrameConfirmation",
	"TabActiveBG", "TabActiveText", "TabActiveBorder", "TabInactiveBG", "TabShowIcons",
	"TabInactiveText", "TabInactiveBorder", "TabInactiveBGNotify", "TabInactiveBGDisconnected", 
	"TabDirtyBlend", "PopupTextColor", "FreeSlotsSize", "CustomSound", "TabSize", "RemovePopups", 
	"ShowTopic", "MaxAutoMatchSource", "MaxMsgLength", "BlendTabs", "PopupActivateOnClick",
	"PopupDontShowOnActive", "DupeColor", "NoTTHColor", "DropStupidConnection", "FlashWindowOnPM", "FlashWindowOnNewPM",
	"RefreshIncomingBetween", "RefreshShareBetween", "RefreshIncomingBegin", "RefreshIncomingEnd",
    "RefreshShareBegin", "RefreshShareEnd", "MuteOnAway", "NotifyUpdates", "NotifyBetaUpdates",
	"SpyIgnoreTTH",
	"SENTRY",
	// Int64
	"TotalUpload", "TotalDownload",
	"SENTRY"
};

SettingsManager::SettingsManager()
{
	//make sure it can fit our events without using push_back since
	//that might cause them to be in the wrong position.
	fileEvents.resize(2);

	connectionSpeeds.push_back("0.005");
	connectionSpeeds.push_back("0.01");
	connectionSpeeds.push_back("0.02");
	connectionSpeeds.push_back("0.05");
	connectionSpeeds.push_back("0.1");
	connectionSpeeds.push_back("0.2");
	connectionSpeeds.push_back("0.5");
	connectionSpeeds.push_back("1");
	connectionSpeeds.push_back("2");
	connectionSpeeds.push_back("5");
	connectionSpeeds.push_back("10");
	connectionSpeeds.push_back("20");
	connectionSpeeds.push_back("50");
	connectionSpeeds.push_back("100");

	for(int i=0; i<SETTINGS_LAST; i++)
		isSet[i] = false;

	for(int j=0; j<INT_LAST-INT_FIRST; j++) {
		intDefaults[j] = 0;
		intSettings[j] = 0;
	}
	for(int k=0; k<INT64_LAST-INT64_FIRST; k++) {
		int64Defaults[k] = 0;
		int64Settings[k] = 0;
	}
	
	setDefault(DOWNLOAD_DIRECTORY, Util::getConfigPath() + "Downloads" PATH_SEPARATOR_STR);
	setDefault(TEMP_DOWNLOAD_DIRECTORY, Util::getConfigPath() + "Incomplete" PATH_SEPARATOR_STR);
	setDefault(SLOTS, 1);
	setDefault(TCP_PORT, 0);
	setDefault(UDP_PORT, 0);
	setDefault(TLS_PORT, 0);
	setDefault(INCOMING_CONNECTIONS, INCOMING_DIRECT);
	setDefault(OUTGOING_CONNECTIONS, OUTGOING_DIRECT);
	setDefault(ROLLBACK, 4096);
	setDefault(AUTO_FOLLOW, true);
	setDefault(CLEAR_SEARCH, true);
	setDefault(SHARE_HIDDEN, false);
	setDefault(FILTER_MESSAGES, true);
	setDefault(MINIMIZE_TRAY, false);
	setDefault(AUTO_SEARCH, true);
	setDefault(TIME_STAMPS, true);
	setDefault(CONFIRM_EXIT, true);
	setDefault(POPUP_HUB_PMS, true);
	setDefault(POPUP_BOT_PMS, true);
	setDefault(IGNORE_HUB_PMS, false);
	setDefault(IGNORE_BOT_PMS, false);
	setDefault(LIST_DUPES, true);
	setDefault(BUFFER_SIZE, 64);
	setDefault(HUBLIST_SERVERS, "http://www.hublist.org/PublicHubList.xml.bz2;http://dc.selwerd.nl/hublist.xml.bz2");
	setDefault(DOWNLOAD_SLOTS, 3);
	setDefault(MAX_DOWNLOAD_SPEED, 0);
	setDefault(LOG_DIRECTORY, Util::getConfigPath() + "Logs" PATH_SEPARATOR_STR);
	setDefault(LOG_UPLOADS, false);
	setDefault(LOG_DOWNLOADS, false);
	setDefault(LOG_PRIVATE_CHAT, false);
	setDefault(LOG_MAIN_CHAT, false);
	setDefault(STATUS_IN_CHAT, true);
	setDefault(SHOW_JOINS, false);
	setDefault(UPLOAD_SPEED, connectionSpeeds[0]);
	setDefault(PRIVATE_MESSAGE_BEEP, false);
	setDefault(PRIVATE_MESSAGE_BEEP_OPEN, false);
	setDefault(USE_OEM_MONOFONT, false);
	setDefault(POPUP_PMS, true);
	setDefault(MIN_UPLOAD_SPEED, 0);
	setDefault(LOG_FORMAT_POST_DOWNLOAD, "%Y-%m-%d %H:%M: %[target]" + STRING(DOWNLOADED_FROM) + "%[userNI] (%[userCID]), %[fileSI] (%[fileSIchunk]), %[speed], %[time]");
	setDefault(LOG_FORMAT_POST_UPLOAD, "%Y-%m-%d %H:%M: %[source]" + STRING(UPLOADED_TO) + "%[userNI] (%[userCID]), %[fileSI] (%[fileSIchunk]), %[speed], %[time]");
	setDefault(LOG_FORMAT_MAIN_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_PRIVATE_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_STATUS, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_SYSTEM, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FILE_MAIN_CHAT, "%[hubURL].log");
	setDefault(LOG_FILE_STATUS, "%[hubURL]_status.log");
	setDefault(LOG_FILE_PRIVATE_CHAT, "%[userNI].%[userCID].log");
	setDefault(LOG_FILE_UPLOAD, "Uploads.log");
	setDefault(LOG_FILE_DOWNLOAD, "Downloads.log");
	setDefault(LOG_FILE_SYSTEM, "system.log");
	setDefault(GET_USER_INFO, true);
	setDefault(URL_HANDLER, false);
	setDefault(AUTO_AWAY, false);
	setDefault(BIND_ADDRESS, "0.0.0.0");
	setDefault(SOCKS_PORT, 1080);
	setDefault(SOCKS_RESOLVE, 1);
	setDefault(CONFIG_VERSION, "0.181");		// 0.181 is the last version missing configversion
	setDefault(KEEP_LISTS, false);
	setDefault(AUTO_KICK, false);
	setDefault(QUEUEFRAME_SHOW_TREE, true);
	setDefault(COMPRESS_TRANSFERS, true);
	setDefault(SHOW_PROGRESS_BARS, true);
	setDefault(SFV_CHECK, false);
	setDefault(DEFAULT_AWAY_MESSAGE, "I'm away. State your business and I might answer later if you're lucky.");
	setDefault(TIME_STAMPS_FORMAT, "%H:%M:%S");
	setDefault(MAX_TAB_ROWS, 2);
	setDefault(MAX_COMPRESSION, 6);
	setDefault(ANTI_FRAG, false);
	setDefault(NO_AWAYMSG_TO_BOTS, true);
	setDefault(SKIP_ZERO_BYTE, false);
	setDefault(ADLS_BREAK_ON_FIRST, false);
	setDefault(HUB_USER_COMMANDS, true);
	setDefault(AUTO_SEARCH_AUTO_MATCH, false);
	setDefault(LOG_FILELIST_TRANSFERS, false);
	setDefault(LOG_SYSTEM, false);
	setDefault(SEND_UNKNOWN_COMMANDS, false);
	setDefault(MAX_HASH_SPEED, 0);
	setDefault(OPEN_USER_CMD_HELP, true);
	setDefault(GET_USER_COUNTRY, true);
	setDefault(FAV_SHOW_JOINS, false);
	setDefault(LOG_STATUS_MESSAGES, false);
	setDefault(SHOW_TRANSFERVIEW, true);
	setDefault(SHOW_STATUSBAR, true);
	setDefault(SHOW_TOOLBAR, true);
	setDefault(POPUNDER_PM, false);
	setDefault(POPUNDER_FILELIST, false);
	setDefault(ADD_FINISHED_INSTANTLY, false);
	setDefault(DONT_DL_ALREADY_SHARED, false);
	setDefault(CONFIRM_HUB_REMOVAL, false);
	setDefault(USE_CTRL_FOR_LINE_HISTORY, true);
	setDefault(JOIN_OPEN_NEW_WINDOW, false);
	setDefault(SEARCH_ONLY_TTH, false);
	setDefault(SHOW_LAST_LINES_LOG, 0);
	setDefault(CONFIRM_ITEM_REMOVAL, true);
	setDefault(ADVANCED_RESUME, true);
	setDefault(ADC_DEBUG, false);
	setDefault(TOGGLE_ACTIVE_WINDOW, false);
	setDefault(SEARCH_HISTORY, 10);
	setDefault(OPEN_PUBLIC, false);
	setDefault(OPEN_FAVORITE_HUBS, false);
	setDefault(OPEN_FAVORITE_USERS, false);
	setDefault(OPEN_QUEUE, false);
	setDefault(OPEN_FINISHED_DOWNLOADS, false);
	setDefault(OPEN_FINISHED_UPLOADS, false);
	setDefault(OPEN_SEARCH_SPY, false);
	setDefault(OPEN_NETWORK_STATISTICS, false);
	setDefault(OPEN_NOTEPAD, false);
	setDefault(NO_IP_OVERRIDE, false);
	setDefault(SEARCH_ONLY_FREE_SLOTS, false);
	setDefault(LAST_SEARCH_TYPE, 0);
	setDefault(SOCKET_IN_BUFFER, 64*1024);
	setDefault(SOCKET_OUT_BUFFER, 64*1024);
	setDefault(ONLY_DL_TTH_FILES, false);
	setDefault(OPEN_WAITING_USERS, false);
	setDefault(OPEN_SYSTEM_LOG, true);
	setDefault(TLS_TRUSTED_CERTIFICATES_PATH, Util::getConfigPath() + "Certificates" PATH_SEPARATOR_STR);
	setDefault(TLS_PRIVATE_KEY_FILE, Util::getConfigPath() + "Certificates" PATH_SEPARATOR_STR "client.key");
	setDefault(TLS_CERTIFICATE_FILE, Util::getConfigPath() + "Certificates" PATH_SEPARATOR_STR "client.crt");
	setDefault(BOLD_FINISHED_DOWNLOADS, true);
	setDefault(BOLD_FINISHED_UPLOADS, true);
	setDefault(BOLD_QUEUE, true);
	setDefault(BOLD_HUB, true);
	setDefault(BOLD_PM, true);
	setDefault(BOLD_SEARCH, true);
	setDefault(BOLD_WAITING_USERS, true);
	setDefault(BOLD_SYSTEM_LOG, true);
	setDefault(AUTO_UPDATE_LIST, true);
	setDefault(USE_TLS, true);
	setDefault(AUTO_SEARCH_LIMIT, 5);
	setDefault(ALT_SORT_ORDER, false);
	setDefault(AUTO_KICK_NO_FAVS, false);
	setDefault(PROMPT_PASSWORD, false);
	setDefault(SPY_FRAME_IGNORE_TTH_SEARCHES, false);
	setDefault(DONT_DL_ALREADY_QUEUED, false);
	setDefault(MAX_COMMAND_LENGTH, 16*1024*1024);
	setDefault(ALLOW_UNTRUSTED_HUBS, true);
	setDefault(ALLOW_UNTRUSTED_CLIENTS, true);

	setDefault(INCOMING_REFRESH_TIME, 60);
	setDefault(SHARE_REFRESH_TIME, 360);
	setDefault(CHATBUFFERSIZE, 25000);
	setDefault(EXPAND_QUEUE, false);
	setDefault(STRIP_ISP, false);
	setDefault(STRIP_ISP_PM, false);
	setDefault(HUB_BOLD_TABS, true);
	setDefault(POPUP_TIMEOUT, 5);
	setDefault(POPUP_AWAY, false);
	setDefault(POPUP_ON_PM, false);
	setDefault(POPUP_ON_NEW_PM, false);
	setDefault(POPUP_ON_HUBSTATUS, false);
	setDefault(HUBFRAME_CONFIRMATION, false);
	setDefault(TAB_ACTIVE_BG, RGB(232, 232, 232));
	setDefault(TAB_ACTIVE_TEXT, RGB(0, 0, 0));
	setDefault(TAB_ACTIVE_BORDER, RGB(0, 0, 0));
	setDefault(TAB_INACTIVE_BG, RGB(255, 255, 255));
	setDefault(TAB_INACTIVE_BG_DISCONNECTED, RGB(104, 138, 176));
	setDefault(TAB_INACTIVE_TEXT, RGB(115, 115, 118));
	setDefault(TAB_INACTIVE_BORDER, RGB(157, 157, 161));
	setDefault(TAB_INACTIVE_BG_NOTIFY, RGB(176, 169, 185));
	setDefault(TAB_DIRTY_BLEND, 10);
	setDefault(POPUP_FONT, "MS Shell Dlg,-11,400,0");
	setDefault(POPUP_TEXTCOLOR, RGB(0, 0, 0));
	setDefault(FREE_SLOTS_EXTENSIONS, "*.nfo|*.sfv");
	setDefault(FREE_SLOTS_SIZE, 64);
	setDefault(SKIPLIST_SHARE, ".*|*All-Files-CRC-OK*|Descript.ion|thumbs.db|*.bad");
	setDefault(SKIPLIST_DOWNLOAD, ".*|*All-Files-CRC-OK*|Descript.ion|thumbs.db|*.bad|*.missing");
	setDefault(TAB_SHOW_ICONS, true);
	setDefault(CUSTOM_SOUND, false);
	setDefault(TAB_SIZE, 20);
	setDefault(REMOVE_POPUPS, true);
	setDefault(REMOVE_TOPIC, false);
	setDefault(MAX_AUTO_MATCH_SOURCES, 5);
	setDefault(MAX_MSG_LENGTH, 90);
	setDefault(POPUP_MINIMIZED, false);
	setDefault(BLEND_TABS, true);
	setDefault(POPUP_ACTIVATE_ON_CLICK, false);
	setDefault(POPUP_DONT_SHOW_ON_ACTIVE, false);
	setDefault(DUPE_COLOR, RGB(255, 128, 255));
	setDefault(NO_TTH_COLOR, RGB(128,128,128));
	setDefault(DROP_STUPID_CONNECTION, false);
	setDefault(FLASH_WINDOW_ON_PM, false);
	setDefault(FLASH_WINDOW_ON_NEW_PM, false);
	setDefault(DIRECTORYLISTINGFRAME_VISIBLE, "1,1,0,1,1");
	setDefault(FINISHED_VISIBLE, "1,1,1,1,1,1,1,1");
	setDefault(FINISHED_UL_VISIBLE, "1,1,1,1,1,1,1");
	setDefault(REFRESH_INCOMING_BETWEEN, false);
	setDefault(REFRESH_SHARE_BETWEEN, false);
	setDefault(REFRESH_INCOMING_BEGIN, 0);
	setDefault(REFRESH_INCOMING_END, 0);
	setDefault(REFRESH_SHARE_BEGIN, 0);
	setDefault(REFRESH_SHARE_END, 0);
	setDefault(MUTE_ON_AWAY, false);
	setDefault(HIGH_PRIO_FILES, "*.sfv|*.nfo|*sample*|*subs*|*.jpg|*cover*|*.pls|*.m3u");
	setDefault(NOTIFY_UPDATES, true);
	setDefault(NOTIFY_BETA_UPDATES, false);
	setDefault(SPY_IGNORE_TTH, true);

#ifdef _WIN32
	setDefault(MAIN_WINDOW_STATE, SW_SHOWNORMAL);
	setDefault(MAIN_WINDOW_SIZE_X, CW_USEDEFAULT);
	setDefault(MAIN_WINDOW_SIZE_Y, CW_USEDEFAULT);
	setDefault(MAIN_WINDOW_POS_X, CW_USEDEFAULT);
	setDefault(MAIN_WINDOW_POS_Y, CW_USEDEFAULT);
	setDefault(MDI_MAXIMIZED, true);
	setDefault(UPLOAD_BAR_COLOR, RGB(205, 60, 55));
	setDefault(DOWNLOAD_BAR_COLOR, RGB(55, 170, 85));

#endif
}

void SettingsManager::load(string const& aFileName)
{
	try {
		SimpleXML xml;
		
		xml.fromXML(File(aFileName, File::READ, File::OPEN).read());
		
		xml.resetCurrentChild();
		
		xml.stepIn();
		
		if(xml.findChild("Settings"))
		{
			xml.stepIn();

			int i;
			
			for(i=STR_FIRST; i<STR_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(StrSetting(i), xml.getChildData());
				xml.resetCurrentChild();
			}
			for(i=INT_FIRST; i<INT_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(IntSetting(i), Util::toInt(xml.getChildData()));
				xml.resetCurrentChild();
			}
			for(i=INT64_FIRST; i<INT64_LAST; i++)
			{
				const string& attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(Int64Setting(i), Util::toInt64(xml.getChildData()));
				xml.resetCurrentChild();
			}
			
			xml.stepOut();
		}

		double v = Util::toDouble(SETTING(CONFIG_VERSION));
		// if(v < 0.x) { // Fix old settings here }

		if(v <= 0.674 || SETTING(PRIVATE_ID).length() != 39 || CID(SETTING(PRIVATE_ID)).isZero()) {
			set(PRIVATE_ID, CID::generate().toBase32());

			// Formats changed, might as well remove these...
			set(LOG_FORMAT_POST_DOWNLOAD, Util::emptyString);
			set(LOG_FORMAT_POST_UPLOAD, Util::emptyString);
			set(LOG_FORMAT_MAIN_CHAT, Util::emptyString);
			set(LOG_FORMAT_PRIVATE_CHAT, Util::emptyString);
			set(LOG_FORMAT_STATUS, Util::emptyString);
			set(LOG_FORMAT_SYSTEM, Util::emptyString);
			set(LOG_FILE_MAIN_CHAT, Util::emptyString);
			set(LOG_FILE_STATUS, Util::emptyString);
			set(LOG_FILE_PRIVATE_CHAT, Util::emptyString);
			set(LOG_FILE_UPLOAD, Util::emptyString);
			set(LOG_FILE_DOWNLOAD, Util::emptyString);
			set(LOG_FILE_SYSTEM, Util::emptyString);
		}


#ifdef _DEBUG
		set(PRIVATE_ID, CID::generate().toBase32());
#endif
		setDefault(UDP_PORT, SETTING(TCP_PORT));

		xml.resetCurrentChild();
		if(xml.findChild("SearchHistory")) {
			xml.stepIn();
			while(xml.findChild("Search")) {
				addSearchToHistory(Text::toT(xml.getChildData()));
			}
			xml.stepOut();
		}

		xml.resetCurrentChild();
		if(xml.findChild("FilterHistory")) {
			xml.stepIn();
			while(xml.findChild("Filter")) {
				addFilterToHistory(Text::toT(xml.getChildData()));
			}
			xml.stepOut();
		}

		xml.resetCurrentChild();
		if(xml.findChild("FileEvents")) {
			xml.stepIn();
			if(xml.findChild("OnFileComplete")) {
				StringPair sp;
				sp.first = xml.getChildAttrib("Command");
				sp.second = xml.getChildAttrib("CommandLine");
				fileEvents[ON_FILE_COMPLETE] = sp;
			}
			xml.resetCurrentChild();
			if(xml.findChild("OnDirCreated")) {
				StringPair sp;
				sp.first = xml.getChildAttrib("Command");
				sp.second = xml.getChildAttrib("CommandLine");
				fileEvents[ON_DIR_CREATED] = sp;
			}
			xml.stepOut();
		}

		xml.resetCurrentChild();

		File::ensureDirectory(SETTING(TLS_TRUSTED_CERTIFICATES_PATH));
		
		fire(SettingsManagerListener::Load(), &xml);

		xml.stepOut();

	} catch(const Exception&) {
		if(CID(SETTING(PRIVATE_ID)).isZero())
			set(PRIVATE_ID, CID::generate().toBase32());
	}
}

void SettingsManager::save(string const& aFileName) {

	SimpleXML xml;
	xml.addTag("DCPlusPlus");
	xml.stepIn();
	xml.addTag("Settings");
	xml.stepIn();

	int i;
	string type("type"), curType("string");
	
	for(i=STR_FIRST; i<STR_LAST; i++)
	{
		if(i == CONFIG_VERSION) {
			xml.addTag(settingTags[i], VERSIONSTRING);
			xml.addChildAttrib(type, curType);
		} else if(isSet[i]) {
			xml.addTag(settingTags[i], get(StrSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}

	curType = "int";
	for(i=INT_FIRST; i<INT_LAST; i++)
	{
		if(isSet[i]) {
			xml.addTag(settingTags[i], get(IntSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	curType = "int64";
	for(i=INT64_FIRST; i<INT64_LAST; i++)
	{
		if(isSet[i])
		{
			xml.addTag(settingTags[i], get(Int64Setting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	xml.stepOut();

	xml.addTag("SearchHistory");
	xml.stepIn();
	{
		Lock l(cs);
		for(TStringIter i = searchHistory.begin(); i != searchHistory.end(); ++i) {
			string tmp = Text::fromT(*i);
			xml.addTag("Search", tmp);
		}
	}
	xml.stepOut();

	xml.addTag("FilterHistory");
	xml.stepIn();
	{
		Lock l(cs);
		for(TStringIter i = filterHistory.begin(); i != filterHistory.end(); ++i) {
			string tmp = Text::fromT(*i);
			xml.addTag("Filter", tmp);
		}
	}
	xml.stepOut();

	xml.addTag("FileEvents");
	xml.stepIn();
	xml.addTag("OnFileComplete");
	xml.addChildAttrib("Command", fileEvents[ON_FILE_COMPLETE].first);
	xml.addChildAttrib("CommandLine", fileEvents[ON_FILE_COMPLETE].second);
	xml.addTag("OnDirCreated");
	xml.addChildAttrib("Command", fileEvents[ON_DIR_CREATED].first);
	xml.addChildAttrib("CommandLine", fileEvents[ON_DIR_CREATED].second);
	xml.stepOut();
	
	fire(SettingsManagerListener::Save(), &xml);

	try {
		File out(aFileName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&out);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
		f.flush();
		out.close();
		File::deleteFile(aFileName);
		File::renameFile(aFileName + ".tmp", aFileName);
	} catch(const FileException&) {
		// ...
	}
}

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

#include "SettingsManager.h"
#include "ResourceManager.h"

#include "SimpleXML.h"
#include "Util.h"
#include "File.h"

const string SettingsManager::settingTags[] =
{
	// Strings
	"Connection", "Description", "DownloadDirectory", "EMail", "Nick", "Server",
	"ClientVersion", "Font", "MainFrameOrder", "MainFrameWidths", "HubFrameOrder", "HubFrameWidths", 
	"LanguageFile", "SearchFrameOrder", "SearchFrameWidths", "FavoritesFrameOrder", "FavoritesFrameWidths", 
	"HublistServers", "QueueFrameOrder", "QueueFrameWidths", "PublicHubsFrameOrder", "PublicHubsFrameWidths", 
	"UsersFrameOrder", "UsersFrameWidths", "HttpProxy", "LogDirectory", "NotepadText", "LogFormatPostDownload",
	"LogFormatPostUpload", "LogFormatMainChat", "LogFormatPrivateChat", "FinishedOrder", "FinishedWidths",	
	"TempDownloadDirectory", "SocksServer", "SocksUser", "SocksPassword", "ConfigVersion",
	"DefaultAwayMessage", "ADLSearchFrameOrder", "ADLSearchFrameWidths", 
	"FinishedULWidths", "FinishedULOrder",
	"DownloadSkiplist", "ShareSkiplist", "WebShortcuts", "PopupFont", "FreeSlotsExtentions",
	"DownloadToPaths", 
	"SENTRY", 
	// Ints
	"ConnectionType", "InPort", "Slots", "Rollback", "AutoFollow", "ClearSearch", "FullRow",
	"BackgroundColor", "TextColor", "UseOemMonoFont", "ShareHidden", "FilterMessages", "MinimizeToTray",
	"OpenPublic", "OpenQueue", "AutoSearch", "AutoAutoSearchString", "TimeStamps", "ConfirmExit", "IgnoreOffline", "PopupOffline",
	"RemoveDupes", "BufferSize", "DownloadSlots", "MaxDownloadSpeed", "LogMainChat", "LogPrivateChat",
	"LogDownloads", "LogUploads", "StatusInChat", "ShowJoins", "PrivateMessageBeep", "PrivateMessageBeepOpen",
	"UseSystemIcons", "PopupPMs", "MinUploadSpeed", "GetUserInfo", "UrlHandler", "MainWindowState", 
	"MainWindowSizeX", "MainWindowSizeY", "MainWindowPosX", "MainWindowPosY", "AutoAway",
	"SmallSendBuffer", "SocksPort", "SocksResolve", "KeepLists", "AutoKick", "QueueFrameShowTree",
	"CompressTransfers", "ShowProgressBars", "SFVCheck", "MaxTabRows", "AutoUpdateList",
	"MaxCompression", "FinishedDirty", "QueueDirty", "AntiFrag", "MDIMaxmimized", "NoAwayMsgToBots",
	"SkipZeroByte", "AdlsBreakOnFirst", "TabCompletion", "OpenFavoriteHubs", "OpenFinishedDownloads",
	"HubUserCommands", "AutoSearchAutoMatch", "DownloadBarColor", "UploadBarColor", "LogSystem",
	"LogFilelistTransfers", "AutoSearchExact", "SendUnknownCommands",
	"IncomingRefreshTime", "ShareRefreshTime", "ChatBuffersize", "AutoUpdateIncoming", 
	"ExpandQueue", "StripIsp", "StripIspPm", "HubBoldTabs", "PmBoldTabs", "HighPrioSample",
	"RotateLogs", "PopupTimeout", "PopupAway", "PopupPm", "PopupNewPm", "PopupHubStatus", 
	"HubFrameConfirmation", "QueueRemoveConfirmation", "PopunderPm", "PopunderDirlist",
	"TabActiveBG", "TabActiveText", "TabActiveBorder", "TabInactiveBG", "TabShowIcons",
	"TabInactiveText", "TabInactiveBorder", "TabInactiveBGNotify", "TabInactiveBGDisconnected", 
	"PopupTextColor", "FreeSlotsSize", "ShowStatusbar", "ShowToolbar", "ShowTransferView",
	"CustomSound", "TabSize", "RemovePopups", "ShowTopic", "MaxAutoMatchSource", 
	"SENTRY",
	// Int64
	"TotalUpload", "TotalDownload",
	"SENTRY"
};

const string SettingsManager::connectionSpeeds[] = { "28.8Kbps", "33.6Kbps", "56Kbps", "ISDN", 
"Satellite", "Cable", "DSL", "LAN(T1)", "LAN(T3)" };

SettingsManager::SettingsManager()
{
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
	
	setDefault(DOWNLOAD_DIRECTORY, Util::getAppPath() + "Downloads\\");
	setDefault(SLOTS, 1);
	//setDefault(SERVER, Util::getLocalIp());
	setDefault(IN_PORT, Util::rand(1025, 32000));
	setDefault(ROLLBACK, 4096);
	setDefault(CLIENTVERSION, "1,0091");
	setDefault(AUTO_FOLLOW, true);
	setDefault(CLEAR_SEARCH, true);
	setDefault(FULL_ROW_SELECT, true);
	setDefault(SHARE_HIDDEN, false);
	setDefault(FILTER_MESSAGES, true);
	setDefault(MINIMIZE_TRAY, false);
	setDefault(OPEN_PUBLIC, false);
	setDefault(OPEN_QUEUE, false);
	setDefault(AUTO_SEARCH, false);
	setDefault(AUTO_SEARCH_AUTO_STRING, false);
	setDefault(TIME_STAMPS, false);
	setDefault(CONFIRM_EXIT, false);
	setDefault(IGNORE_OFFLINE, false);
	setDefault(POPUP_OFFLINE, false);
	setDefault(REMOVE_DUPES, true);
	setDefault(BUFFER_SIZE, 64);
	setDefault(HUBLIST_SERVERS, "http://www.hublist.org/PublicHubList.config.bz2");
	setDefault(DOWNLOAD_SLOTS, 3);
	setDefault(MAX_DOWNLOAD_SPEED, 0);
	setDefault(LOG_DIRECTORY, Util::getAppPath() + "Logs\\");
	setDefault(LOG_UPLOADS, false);
	setDefault(LOG_DOWNLOADS, false);
	setDefault(LOG_PRIVATE_CHAT, false);
	setDefault(LOG_MAIN_CHAT, false);
	setDefault(STATUS_IN_CHAT, true);
	setDefault(SHOW_JOINS, false);
	setDefault(CONNECTION, connectionSpeeds[0]);
	setDefault(PRIVATE_MESSAGE_BEEP, false);
	setDefault(PRIVATE_MESSAGE_BEEP_OPEN, false);
	setDefault(USE_SYSTEM_ICONS, true);
	setDefault(USE_OEM_MONOFONT, false);
	setDefault(POPUP_PMS, true);
	setDefault(MIN_UPLOAD_SPEED, 0);
	setDefault(LOG_FORMAT_POST_DOWNLOAD, "%Y-%m-%d %H:%M: %[target]" + STRING(DOWNLOADED_FROM) + "%[user], %[size] (%[chunksize]), %[speed], %[time]");
	setDefault(LOG_FORMAT_POST_UPLOAD, "%Y-%m-%d %H:%M: %[source]" + STRING(UPLOADED_TO) + "%[user], %[size] (%[chunksize]), %[speed], %[time]");
	setDefault(LOG_FORMAT_MAIN_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_PRIVATE_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(GET_USER_INFO, true);
	setDefault(URL_HANDLER, false);
	setDefault(AUTO_AWAY, false);
	setDefault(SMALL_SEND_BUFFER, false);
	setDefault(SOCKS_PORT, 1080);
	setDefault(SOCKS_RESOLVE, 1);
	setDefault(CONFIG_VERSION, "0.181");		// 0.181 is the last version missing configversion
	setDefault(KEEP_LISTS, false);
	setDefault(AUTO_KICK, false);
	setDefault(QUEUEFRAME_SHOW_TREE, true);
	setDefault(COMPRESS_TRANSFERS, true);
	setDefault(SHOW_PROGRESS_BARS, true);
	setDefault(SFV_CHECK, false);
	setDefault(DEFAULT_AWAY_MESSAGE, "I'm away. I might answer later if you're lucky.");
	setDefault(MAX_TAB_ROWS, 2);
	setDefault(AUTO_UPDATE_LIST, true);
	setDefault(MAX_COMPRESSION, 6);
	setDefault(FINISHED_DIRTY, true);
	setDefault(QUEUE_DIRTY, true);
	setDefault(ANTI_FRAG, false);
	setDefault(NO_AWAYMSG_TO_BOTS, true);
	setDefault(SKIP_ZERO_BYTE, false);
	setDefault(ADLS_BREAK_ON_FIRST, false);
	setDefault(TAB_COMPLETION, true);
	setDefault(OPEN_FAVORITE_HUBS, false);
	setDefault(OPEN_FINISHED_DOWNLOADS, false);
	setDefault(HUB_USER_COMMANDS, true);
	setDefault(AUTO_SEARCH_AUTO_MATCH, false);
	setDefault(LOG_FILELIST_TRANSFERS, true);
	setDefault(AUTO_SEARCH_EXACT, true);
	setDefault(LOG_SYSTEM, false);
	setDefault(SEND_UNKNOWN_COMMANDS, true);
	setDefault(INCOMING_REFRESH_TIME, 60);
	setDefault(SHARE_REFRESH_TIME, 360);
	setDefault(CHATBUFFERSIZE, 25000);
	setDefault(EXPAND_QUEUE, true);
	setDefault(STRIP_ISP, false);
	setDefault(STRIP_ISP_PM, false);
	setDefault(HUB_BOLD_TABS, true);
	setDefault(PM_BOLD_TABS, true);
	setDefault(HIGH_PRIO_SAMPLE, false);
	setDefault(ROTATE_LOG, false);
	setDefault(POPUP_TIMEOUT, 5);
	setDefault(POPUP_AWAY, false);
	setDefault(POPUP_ON_PM, false);
	setDefault(POPUP_ON_NEW_PM, false);
	setDefault(POPUP_ON_HUBSTATUS, false);
	setDefault(HUBFRAME_CONFIRMATION, false);
	setDefault(QUEUE_REMOVE_CONFIRMATION, false);
	setDefault(POPUNDER_PM, false);
	setDefault(POPUNDER_DIRLIST, false);
	setDefault(WEB_SHORTCUTS, "As URL&#1;u&#1;%s&#2;Google&#1;g&#1;http://www.google.com/search?q=%s&#2;IMDB&#1;i&#1;http://www.imdb.com/Find?select=All&amp;for=%s&#2;TV Tome&#1;t&#1;http://www.tvtome.com/tvtome/servlet/Search?searchType=all&amp;searchString=%s");
	setDefault(TAB_ACTIVE_BG, RGB(232, 232, 232));
	setDefault(TAB_ACTIVE_TEXT, RGB(0, 0, 0));
	setDefault(TAB_ACTIVE_BORDER, RGB(0, 0, 0));
	setDefault(TAB_INACTIVE_BG, RGB(255, 255, 255));
	setDefault(TAB_INACTIVE_BG_DISCONNECTED, RGB(104, 138, 176));
	setDefault(TAB_INACTIVE_TEXT, RGB(115, 115, 118));
	setDefault(TAB_INACTIVE_BORDER, RGB(157, 157, 161));
	setDefault(TAB_INACTIVE_BG_NOTIFY, RGB(176, 169, 185));
	setDefault(POPUP_FONT, "MS Shell Dlg,-11,400,0");
	setDefault(POPUP_TEXTCOLOR, RGB(0, 0, 0));
	setDefault(FREE_SLOTS_EXTENSIONS, "*.nfo|*.sfv");
	setDefault(FREE_SLOTS_SIZE, 32);
	setDefault(SKIPLIST_SHARE, ".ioFTPD|.checked|.raidenftpd.acl|.SimSfvChk.log|*All-Files-CRC-OK*|.message|Descript.ion|.upChk.log|thumbs.db|.crc");
	setDefault(SKIPLIST_DOWNLOAD, ".ioFTPD|.checked|.raidenftpd.acl|.SimSfvChk.log|*All-Files-CRC-OK*|.message|Descript.ion|.upChk.log|thumbs.db|.crc");
	setDefault(SHOW_STATUSBAR, true);
	setDefault(SHOW_TOOLBAR, true);
	setDefault(SHOW_TRANSFERVIEW, true);
	setDefault(TAB_SHOW_ICONS, true);
	setDefault(DOWNLOAD_TO_PATHS, "");
	setDefault(CUSTOM_SOUND, false);
	setDefault(TAB_SIZE, 20);
	setDefault(REMOVE_POPUPS, true);
	setDefault(SHOW_TOPIC, true);
	setDefault(MAX_AUTO_MATCH_SOURCES, 5);
	
	
	

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
	string xmltext;
	try {
		File f(aFileName, File::READ, File::OPEN);
		xmltext = f.read();		
	} catch(const FileException&) {
		// ...
		return;
	}

	if(xmltext.empty()) {
		// Nothing to load...
		return;
	}

	try {
		SimpleXML xml;
		xml.fromXML(xmltext);
		
		xml.resetCurrentChild();
		
		xml.stepIn();
		
		if(xml.findChild("Settings"))
		{
			xml.stepIn();
			
			int i;
			string attr;
			
			for(i=STR_FIRST; i<STR_LAST; i++)
			{
				attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(StrSetting(i), xml.getChildData());
				xml.resetCurrentChild();
			}
			for(i=INT_FIRST; i<INT_LAST; i++)
			{
				attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(IntSetting(i), Util::toInt(xml.getChildData()));
				xml.resetCurrentChild();
			}
			for(i=INT64_FIRST; i<INT64_LAST; i++)
			{
				attr = settingTags[i];
				dcassert(attr.find("SENTRY") == string::npos);
				
				if(xml.findChild(attr))
					set(Int64Setting(i), Util::toInt64(xml.getChildData()));
				xml.resetCurrentChild();
			}
			
			xml.stepOut();
		}

		
				
		fire(SettingsManagerListener::LOAD, &xml);

		xml.stepOut();

	} catch(const Exception&) {
		// Oops, bad...
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

	
	
	fire(SettingsManagerListener::SAVE, &xml);

	try {
		File ff(aFileName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&ff);
		f.write(SimpleXML::w1252Header);
		xml.toXML(&f);
		f.flush();
		ff.close();
		File::deleteFile(aFileName);
		File::renameFile(aFileName + ".tmp", aFileName);
	} catch(const FileException&) {
		// ...
	}
}

/**
 * @file
 * $Id: SettingsManager.cpp,v 1.13 2004/02/23 16:02:19 trem Exp $
 */


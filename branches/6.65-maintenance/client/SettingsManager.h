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

#if !defined(SETTINGS_MANAGER_H)
#define SETTINGS_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Speaker.h"
#include "Singleton.h"
#include "Colorsettings.h"

class SimpleXML;

class SettingsManagerListener {
public:
	virtual ~SettingsManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Load;
	typedef X<1> Save;

	virtual void on(Load, SimpleXML&) throw() { }
	virtual void on(Save, SimpleXML&) throw() { }
};

class SettingsManager : public Singleton<SettingsManager>, public Speaker<SettingsManagerListener>
{
public:

	static StringList connectionSpeeds;

	enum StrSetting { STR_FIRST,
		NICK = STR_FIRST, UPLOAD_SPEED, DESCRIPTION, DOWNLOAD_DIRECTORY, EMAIL, EXTERNAL_IP,
		TEXT_FONT, MAINFRAME_ORDER, MAINFRAME_WIDTHS, HUBFRAME_ORDER, HUBFRAME_WIDTHS,
		LANGUAGE_FILE, SEARCHFRAME_ORDER, SEARCHFRAME_WIDTHS, FAVORITESFRAME_ORDER, FAVORITESFRAME_WIDTHS,
		HUBLIST_SERVERS, QUEUEFRAME_ORDER, QUEUEFRAME_WIDTHS, PUBLICHUBSFRAME_ORDER, PUBLICHUBSFRAME_WIDTHS,
		USERSFRAME_ORDER, USERSFRAME_WIDTHS, HTTP_PROXY, LOG_DIRECTORY, NOTEPAD_TEXT, LOG_FORMAT_POST_DOWNLOAD,
		LOG_FORMAT_POST_UPLOAD, LOG_FORMAT_MAIN_CHAT, LOG_FORMAT_PRIVATE_CHAT, FINISHED_ORDER, FINISHED_WIDTHS,
		TEMP_DOWNLOAD_DIRECTORY, BIND_ADDRESS, SOCKS_SERVER, SOCKS_USER, SOCKS_PASSWORD, CONFIG_VERSION,
		DEFAULT_AWAY_MESSAGE, TIME_STAMPS_FORMAT, ADLSEARCHFRAME_ORDER, ADLSEARCHFRAME_WIDTHS,
		FINISHED_UL_WIDTHS, FINISHED_UL_ORDER, CLIENT_ID, SPYFRAME_WIDTHS, SPYFRAME_ORDER, LOG_FILE_MAIN_CHAT,
		LOG_FILE_PRIVATE_CHAT, LOG_FILE_STATUS, LOG_FILE_UPLOAD, LOG_FILE_DOWNLOAD, LOG_FILE_SYSTEM,
		LOG_FORMAT_SYSTEM, LOG_FORMAT_STATUS, DIRECTORYLISTINGFRAME_ORDER, DIRECTORYLISTINGFRAME_WIDTHS,
		
        SKIPLIST_DOWNLOAD, SKIPLIST_SHARE, POPUP_FONT, FREE_SLOTS_EXTENSIONS, 
		HUBFRAME_VISIBLE, MAINFRAME_VISIBLE, SEARCHFRAME_VISIBLE, 
		QUEUEFRAME_VISIBLE, DIRECTORYLISTINGFRAME_VISIBLE, FINISHED_VISIBLE, FINISHED_UL_VISIBLE,
		HIGH_PRIO_FILES,
		STR_LAST };

	enum IntSetting { INT_FIRST = STR_LAST + 1,
		INCOMING_CONNECTIONS = INT_FIRST, TCP_PORT, SLOTS, ROLLBACK, AUTO_FOLLOW, CLEAR_SEARCH,
		BACKGROUND_COLOR, TEXT_COLOR, USE_OEM_MONOFONT, SHARE_HIDDEN, FILTER_MESSAGES, MINIMIZE_TRAY,
		AUTO_SEARCH, TIME_STAMPS, CONFIRM_EXIT, IGNORE_OFFLINE, POPUP_OFFLINE,
		LIST_DUPES, BUFFER_SIZE, DOWNLOAD_SLOTS, MAX_DOWNLOAD_SPEED, LOG_MAIN_CHAT, LOG_PRIVATE_CHAT,
		LOG_DOWNLOADS, LOG_UPLOADS, STATUS_IN_CHAT, SHOW_JOINS, PRIVATE_MESSAGE_BEEP, PRIVATE_MESSAGE_BEEP_OPEN,
		POPUP_PMS, MIN_UPLOAD_SPEED, GET_USER_INFO, URL_HANDLER, MAIN_WINDOW_STATE,
		MAIN_WINDOW_SIZE_X, MAIN_WINDOW_SIZE_Y, MAIN_WINDOW_POS_X, MAIN_WINDOW_POS_Y, AUTO_AWAY,
		SOCKS_PORT, SOCKS_RESOLVE, KEEP_LISTS, AUTO_KICK, QUEUEFRAME_SHOW_TREE,
		COMPRESS_TRANSFERS, SHOW_PROGRESS_BARS, SFV_CHECK, MAX_TAB_ROWS, AUTO_UPDATE_LIST,
		MAX_COMPRESSION, ANTI_FRAG, MDI_MAXIMIZED, NO_AWAYMSG_TO_BOTS,
		SKIP_ZERO_BYTE, ADLS_BREAK_ON_FIRST,
		HUB_USER_COMMANDS, AUTO_SEARCH_AUTO_MATCH, UPLOAD_BAR_COLOR, DOWNLOAD_BAR_COLOR, LOG_SYSTEM,
		LOG_FILELIST_TRANSFERS, SEND_UNKNOWN_COMMANDS, MAX_HASH_SPEED, OPEN_USER_CMD_HELP,
		GET_USER_COUNTRY, FAV_SHOW_JOINS, LOG_STATUS_MESSAGES, SHOW_STATUSBAR,
		SHOW_TOOLBAR, SHOW_TransfersFrame, POPUNDER_PM, POPUNDER_FILELIST,
		ADD_FINISHED_INSTANTLY, DONT_DL_ALREADY_SHARED, USE_CTRL_FOR_LINE_HISTORY, CONFIRM_HUB_REMOVAL,
		JOIN_OPEN_NEW_WINDOW, UDP_PORT, SHOW_LAST_LINES_LOG, CONFIRM_ITEM_REMOVAL,
		ADVANCED_RESUME, ADC_DEBUG, TOGGLE_ACTIVE_WINDOW, SEARCH_HISTORY, MAX_FILELIST_SIZE,
		OPEN_PUBLIC, OPEN_FAVORITE_HUBS, OPEN_FAVORITE_USERS, OPEN_QUEUE, OPEN_FINISHED_DOWNLOADS,
		OPEN_FINISHED_UPLOADS, OPEN_SEARCH_SPY, OPEN_NETWORK_STATISTICS, OPEN_NOTEPAD, OUTGOING_CONNECTIONS,
		NO_IP_OVERRIDE, SEARCH_ONLY_FREE_SLOTS, LAST_SEARCH_TYPE, BOLD_FINISHED_DOWNLOADS, BOLD_FINISHED_UPLOADS, BOLD_QUEUE,
		BOLD_HUB, BOLD_PM, BOLD_SEARCH, SOCKET_IN_BUFFER, SOCKET_OUT_BUFFER, ONLY_DL_TTH_FILES,
		OPEN_WAITING_USERS, BOLD_WAITING_USERS, OPEN_SYSTEM_LOG, BOLD_SYSTEM_LOG,
		AUTO_SEARCH_LIMIT, ALT_SORT_ORDER, AUTO_KICK_NO_FAVS, PROMPT_PASSWORD, SPY_FRAME_IGNORE_TTH_SEARCHES,
		DONT_DL_ALREADY_QUEUED, MAX_COMMAND_LENGTH,

		INCOMING_REFRESH_TIME, SHARE_REFRESH_TIME, CHATBUFFERSIZE, AUTO_UPDATE_INCOMING, EXPAND_QUEUE,
		STRIP_ISP, STRIP_ISP_PM,HUB_BOLD_TABS,
		POPUP_TIMEOUT, POPUP_AWAY, POPUP_MINIMIZED, POPUP_ON_PM, POPUP_ON_NEW_PM, POPUP_ON_HUBSTATUS,
		HUBFRAME_CONFIRMATION,
		TAB_ACTIVE_BG, TAB_ACTIVE_TEXT, TAB_ACTIVE_BORDER, TAB_INACTIVE_BG, TAB_SHOW_ICONS, 
		TAB_INACTIVE_TEXT, TAB_INACTIVE_BORDER, TAB_INACTIVE_BG_NOTIFY, TAB_INACTIVE_BG_DISCONNECTED, 
		TAB_DIRTY_BLEND, POPUP_TEXTCOLOR, FREE_SLOTS_SIZE, CUSTOM_SOUND, TAB_SIZE, REMOVE_POPUPS, REMOVE_TOPIC, 
		MAX_AUTO_MATCH_SOURCES, MAX_MSG_LENGTH, BLEND_TABS, POPUP_ACTIVATE_ON_CLICK, 
		POPUP_DONT_SHOW_ON_ACTIVE, DUPE_COLOR, NO_TTH_COLOR, DROP_STUPID_CONNECTION, FLASH_WINDOW_ON_PM,
		FLASH_WINDOW_ON_NEW_PM,
		REFRESH_INCOMING_BETWEEN, REFRESH_SHARE_BETWEEN, REFRESH_INCOMING_BEGIN, 
		REFRESH_INCOMING_END, REFRESH_SHARE_BEGIN, REFRESH_SHARE_END, MUTE_ON_AWAY, NOTIFY_UPDATES,
		NOTIFY_BETA_UPDATES, SPY_IGNORE_TTH,
        INT_LAST };

	enum Int64Setting { INT64_FIRST = INT_LAST + 1,
		TOTAL_UPLOAD = INT64_FIRST, TOTAL_DOWNLOAD, INT64_LAST, SETTINGS_LAST = INT64_LAST };

	enum {	INCOMING_DIRECT, INCOMING_FIREWALL_UPNP, INCOMING_FIREWALL_NAT,
		INCOMING_FIREWALL_PASSIVE };
	enum {	OUTGOING_DIRECT, OUTGOING_SOCKS5 };

	enum FileEvents { ON_FILE_COMPLETE, ON_DIR_CREATED};


	const string& get(StrSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? strSettings[key - STR_FIRST] : strDefaults[key - STR_FIRST];
	}

	int get(IntSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? intSettings[key - INT_FIRST] : intDefaults[key - INT_FIRST];
	}
	int64_t get(Int64Setting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? int64Settings[key - INT64_FIRST] : int64Defaults[key - INT64_FIRST];
	}

	bool getBool(IntSetting key, bool useDefault = true) const {
		return (get(key, useDefault) != 0);
	}

	void set(StrSetting key, string const& value) {
		if(((key == DESCRIPTION) || (key == NICK) || (key == UPLOAD_SPEED)) && (value.size() > 35)) {
			strSettings[key - STR_FIRST] = value.substr(0, 35);
		} else {
			strSettings[key - STR_FIRST] = value;
		}
		if(key == TEMP_DOWNLOAD_DIRECTORY)
			isSet[key] = true;
		else
			isSet[key] = !value.empty();
	}

	void set(IntSetting key, int value) {
		if((key == SLOTS) && (value <= 0)) {
			value = 1;
		} else if((key == FREE_SLOTS_SIZE) && (value < 64)) {
			value = 64;
		}
		intSettings[key - INT_FIRST] = value;
		isSet[key] = true;
	}

	void set(IntSetting key, const string& value) {
		if(value.empty()) {
			intSettings[key - INT_FIRST] = 0;
			isSet[key] = false;
		} else {
			intSettings[key - INT_FIRST] = Util::toInt(value);
			isSet[key] = true;
		}
	}

	void set(Int64Setting key, int64_t value) {
		int64Settings[key - INT64_FIRST] = value;
		isSet[key] = true;
	}

	void set(Int64Setting key, const string& value) {
		if(value.empty()) {
			int64Settings[key - INT64_FIRST] = 0;
			isSet[key] = false;
		} else {
			int64Settings[key - INT64_FIRST] = Util::toInt64(value);
			isSet[key] = true;
		}
	}

	void set(IntSetting key, bool value) { set(key, (int)value); }

	void setDefault(StrSetting key, string const& value) {
		strDefaults[key - STR_FIRST] = value;
	}

	void setDefault(IntSetting key, int value) {
		intDefaults[key - INT_FIRST] = value;
	}
	void setDefault(Int64Setting key, int64_t value) {
		int64Defaults[key - INT64_FIRST] = value;
	}

	bool isDefault(int aSet) { return !isSet[aSet]; }

	void load() {
		load(Util::getConfigPath() + "DCPlusPlus.xml");
	}
	void save() {
		save(Util::getConfigPath() + "DCPlusPlus.xml");
	}

	void load(const string& aFileName);
	void save(const string& aFileName);

	TStringList getSearchHistory() const {
		Lock l(cs);
		return searchHistory;
	}

	//returns true if the search string is added to the history
	//returns false if the search string already exists in the history.
	bool addSearchToHistory(const tstring& search) {
		if(search.empty())
			return false;

		Lock l(cs);

		if(find(searchHistory.begin(), searchHistory.end(), search) != searchHistory.end())
			return false;

		
		while(searchHistory.size() > static_cast<TStringList::size_type>(getInstance()->get(SEARCH_HISTORY)))
			searchHistory.erase(searchHistory.begin());

		searchHistory.push_back(search);

		return true;
	}

	void clearSearchHistory() {
		Lock l(cs);
		searchHistory.clear();
	}

	TStringList getFilterHistory() const {
		Lock l(cs);
		return filterHistory;
	}

	//returns true if the filter string is added to the history
	//returns false if the filter string already exists in the history.
	bool addFilterToHistory(const tstring& filter) {
		
		if(filter.empty())
			return false;

		Lock l(cs);

		if(find(filterHistory.begin(), filterHistory.end(), filter) != filterHistory.end())
			return false;

		while(filterHistory.size() > static_cast<TStringList::size_type>(getInstance()->get(SEARCH_HISTORY)))
			filterHistory.erase(filterHistory.begin());

		filterHistory.push_back(filter);

		return true;
	}

	void clearFilterHistory() {
		Lock l(cs);
		filterHistory.clear();
	}

	StringPair getFileEvent(SettingsManager::FileEvents fe) {
		return fileEvents[fe];
	}

private:
	friend class Singleton<SettingsManager>;
	SettingsManager();
	virtual ~SettingsManager() throw() { }

	static const string settingTags[SETTINGS_LAST+1];

	string strSettings[STR_LAST - STR_FIRST];
	int    intSettings[INT_LAST - INT_FIRST];
	int64_t int64Settings[INT64_LAST - INT64_FIRST];
	string strDefaults[STR_LAST - STR_FIRST];
	int    intDefaults[INT_LAST - INT_FIRST];
	int64_t int64Defaults[INT64_LAST - INT64_FIRST];
	bool isSet[SETTINGS_LAST];

	mutable CriticalSection cs;

	TStringList		searchHistory;
	TStringList		filterHistory;
	StringPairList fileEvents;
};

// Shorthand accessor macros
#define SETTING(k) (SettingsManager::getInstance()->get(SettingsManager::k, true))
#define BOOLSETTING(k) (SettingsManager::getInstance()->getBool(SettingsManager::k, true))

#endif // !defined(SETTINGS_MANAGER_H)

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

#if !defined(SETTINGSMANAGER_H)
#define SETTINGSMANAGER_H

#include "Util.h"
#include "Speaker.h"
#include "Singleton.h"
#include "Colorsettings.h"

class SimpleXML;

class SettingsManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Load;
	typedef X<1> Save;

	virtual void on(Load, SimpleXML*) throw() { }
	virtual void on(Save, SimpleXML*) throw() { }
};

class SettingsManager : public Singleton<SettingsManager>, public Speaker<SettingsManagerListener>
{
public:

	static const string connectionSpeeds[];

	enum StrSetting { STR_FIRST,
		CONNECTION = STR_FIRST, DESCRIPTION, DOWNLOAD_DIRECTORY, EMAIL, NICK, SERVER,
		TEXT_FONT, MAINFRAME_ORDER, MAINFRAME_WIDTHS, HUBFRAME_ORDER, HUBFRAME_WIDTHS, 
		LANGUAGE_FILE, SEARCHFRAME_ORDER, SEARCHFRAME_WIDTHS, FAVORITESFRAME_ORDER, FAVORITESFRAME_WIDTHS, 
		HUBLIST_SERVERS, QUEUEFRAME_ORDER, QUEUEFRAME_WIDTHS, PUBLICHUBSFRAME_ORDER, PUBLICHUBSFRAME_WIDTHS, 
		USERSFRAME_ORDER, USERSFRAME_WIDTHS, HTTP_PROXY, LOG_DIRECTORY, NOTEPAD_TEXT, LOG_FORMAT_POST_DOWNLOAD, 
		LOG_FORMAT_POST_UPLOAD, LOG_FORMAT_MAIN_CHAT, LOG_FORMAT_PRIVATE_CHAT, FINISHED_ORDER, FINISHED_WIDTHS, 
		TEMP_DOWNLOAD_DIRECTORY, SOCKS_SERVER, SOCKS_USER, SOCKS_PASSWORD, CONFIG_VERSION,
		DEFAULT_AWAY_MESSAGE, TIME_STAMPS_FORMAT, ADLSEARCHFRAME_ORDER, ADLSEARCHFRAME_WIDTHS, 
		FINISHED_UL_WIDTHS, FINISHED_UL_ORDER, CLIENT_ID, SPYFRAME_WIDTHS, SPYFRAME_ORDER, 
		SKIPLIST_DOWNLOAD, SKIPLIST_SHARE, POPUP_FONT, FREE_SLOTS_EXTENSIONS, 
		HUBFRAME_VISIBLE, MAINFRAME_VISIBLE, SEARCHFRAME_VISIBLE, 
		QUEUEFRAME_VISIBLE, 
		STR_LAST };

	enum IntSetting { INT_FIRST = STR_LAST + 1,
		CONNECTION_TYPE = INT_FIRST, IN_PORT, SLOTS, ROLLBACK, AUTO_FOLLOW, CLEAR_SEARCH, 
		BACKGROUND_COLOR, TEXT_COLOR, USE_OEM_MONOFONT, SHARE_HIDDEN, FILTER_MESSAGES, MINIMIZE_TRAY,
		OPEN_PUBLIC, OPEN_QUEUE, AUTO_SEARCH, TIME_STAMPS, CONFIRM_EXIT, IGNORE_OFFLINE, POPUP_OFFLINE,
		LIST_DUPES, BUFFER_SIZE, DOWNLOAD_SLOTS, MAX_DOWNLOAD_SPEED, LOG_MAIN_CHAT, LOG_PRIVATE_CHAT,
		LOG_DOWNLOADS, LOG_UPLOADS, STATUS_IN_CHAT, SHOW_JOINS, PRIVATE_MESSAGE_BEEP, PRIVATE_MESSAGE_BEEP_OPEN,
		USE_SYSTEM_ICONS, POPUP_PMS, MIN_UPLOAD_SPEED, GET_USER_INFO, URL_HANDLER, MAIN_WINDOW_STATE,
		MAIN_WINDOW_SIZE_X, MAIN_WINDOW_SIZE_Y, MAIN_WINDOW_POS_X, MAIN_WINDOW_POS_Y, AUTO_AWAY,
		SMALL_SEND_BUFFER, SOCKS_PORT, SOCKS_RESOLVE, KEEP_LISTS, AUTO_KICK, QUEUEFRAME_SHOW_TREE,
		COMPRESS_TRANSFERS, SHOW_PROGRESS_BARS, SFV_CHECK, MAX_TAB_ROWS, AUTO_UPDATE_LIST,
		MAX_COMPRESSION, FINISHED_DIRTY, QUEUE_DIRTY, ANTI_FRAG, MDI_MAXIMIZED, NO_AWAYMSG_TO_BOTS,
		SKIP_ZERO_BYTE, ADLS_BREAK_ON_FIRST, TAB_COMPLETION, OPEN_FAVORITE_HUBS, OPEN_FINISHED_DOWNLOADS,
		HUB_USER_COMMANDS, AUTO_SEARCH_AUTO_MATCH, UPLOAD_BAR_COLOR, DOWNLOAD_BAR_COLOR, LOG_SYSTEM,
		LOG_FILELIST_TRANSFERS, SEND_UNKNOWN_COMMANDS, MAX_HASH_SPEED, OPEN_USER_CMD_HELP,
		GET_USER_COUNTRY, FAV_SHOW_JOINS, LOG_STATUS_MESSAGES, SHOW_STATUSBAR,
		SHOW_TOOLBAR, SHOW_TRANSFERVIEW, POPUNDER_PM, POPUNDER_FILELIST,
		ADD_FINISHED_INSTANTLY, SETTINGS_USE_UPNP, DONT_DL_ALREADY_SHARED, SETTINGS_USE_CTRL_FOR_LINE_HISTORY, CONFIRM_HUB_REMOVAL, 
		INCOMING_REFRESH_TIME, SHARE_REFRESH_TIME, CHATBUFFERSIZE, AUTO_UPDATE_INCOMING, EXPAND_QUEUE,
		STRIP_ISP, STRIP_ISP_PM,HUB_BOLD_TABS, PM_BOLD_TABS, HIGH_PRIO_SAMPLE, ROTATE_LOG,
		POPUP_TIMEOUT, POPUP_AWAY, POPUP_MINIMIZED, POPUP_ON_PM, POPUP_ON_NEW_PM, POPUP_ON_HUBSTATUS,
		HUBFRAME_CONFIRMATION, QUEUE_REMOVE_CONFIRMATION,
		TAB_ACTIVE_BG, TAB_ACTIVE_TEXT, TAB_ACTIVE_BORDER, TAB_INACTIVE_BG, TAB_SHOW_ICONS, 
		TAB_INACTIVE_TEXT, TAB_INACTIVE_BORDER, TAB_INACTIVE_BG_NOTIFY, TAB_INACTIVE_BG_DISCONNECTED, 
		TAB_DIRTY_BLEND, POPUP_TEXTCOLOR, FREE_SLOTS_SIZE, CUSTOM_SOUND, TAB_SIZE, REMOVE_POPUPS, REMOVE_TOPIC, 
		MAX_AUTO_MATCH_SOURCES, MAX_MSG_LENGTH, BLEND_TABS, TOGGLE_ACTIVE_WINDOW, POPUP_ACTIVATE_ON_CLICK, 
		POPUP_DONT_SHOW_ON_ACTIVE, DUPE_COLOR, DROP_STUPID_CONNECTION, 
        INT_LAST };

	enum Int64Setting { INT64_FIRST = INT_LAST + 1,
		TOTAL_UPLOAD = INT64_FIRST, TOTAL_DOWNLOAD, INT64_LAST, SETTINGS_LAST = INT64_LAST };

	enum {	SPEED_288K, SPEED_336K, SPEED_576K, SPEED_ISDN, SPEED_SATELLITE, SPEED_CABLE,
			SPEED_DSL, SPEED_T1, SPEED_T3, SPEED_LAST };

	enum {	CONNECTION_ACTIVE, CONNECTION_PASSIVE, CONNECTION_SOCKS5 };

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
		if(((key == DESCRIPTION) || (key == NICK) || (key == CONNECTION)) && (value.size() > 35)) {
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

	bool isDefault(int aSet) { return !isSet[aSet]; };

	void load() {
		load(Util::getAppPath() + "DCPlusPlus.xml");
	}
	void save() {
		save(Util::getAppPath() + "DCPlusPlus.xml");
	}

	void load(const string& aFileName);
	void save(const string& aFileName);

	StringMap getDownloadPaths() const {
		Lock l(cs);
		return downloadPaths;
	}

	void setDownloadPaths( const StringMap& dl ) {
		Lock l(cs);
		downloadPaths = dl;
	}

private:
	friend class Singleton<SettingsManager>;
	SettingsManager();

	static const string settingTags[SETTINGS_LAST+1];

	string strSettings[STR_LAST - STR_FIRST];
	int    intSettings[INT_LAST - INT_FIRST];
	int64_t int64Settings[INT64_LAST - INT64_FIRST];
	string strDefaults[STR_LAST - STR_FIRST];
	int    intDefaults[INT_LAST - INT_FIRST];
	int64_t int64Defaults[INT64_LAST - INT64_FIRST];
	bool isSet[SETTINGS_LAST];

	mutable CriticalSection cs;
	StringMap downloadPaths;
};

// Shorthand accessor macros
#define SETTING(k) (SettingsManager::getInstance()->get(SettingsManager::k, true))
#define BOOLSETTING(k) (SettingsManager::getInstance()->getBool(SettingsManager::k, true))

#endif // SETTINGSMANAGER_H

/**
 * @file
 * $Id: SettingsManager.h,v 1.12 2004/02/23 16:02:19 trem Exp $
 */


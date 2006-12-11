/* 
* Copyright (C) 2003-2005 Pär Björklund, per.bjorklund@gmail.com
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

#ifndef IGNOREMANAGER_H
#define IGNOREMANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Singleton.h"
#include "SettingsManager.h"

class IgnoreManager: public Singleton<IgnoreManager>, private SettingsManagerListener
{
public:
	IgnoreManager() { 
		SettingsManager::getInstance()->addListener(this);
	}
	~IgnoreManager() {
		SettingsManager::getInstance()->removeListener(this);
	}

	bool ignore(const string& aPattern);
	bool unignore(const string& aPattern);

	//used to check if the user should be ignored
	//matches wildcards and regexps
	bool isIgnored(const string& aNick);

	//used to see if the users nick is in the ignore list
	bool isUserIgnored(const string& aNick);

	void copyPatterns(StringSet& patterns);
	void replacePatterns(const StringSet& patterns);
	
private:
	StringSet patterns;
	CriticalSection cs;

	void load(SimpleXML& aXml);
	void save(SimpleXML& aXml);

	virtual void on(SettingsManagerListener::Load, SimpleXML& xml) throw();
	virtual void on(SettingsManagerListener::Save, SimpleXML& xml) throw();
};

#endif // IGNOREMANAGER_H

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

	void load(SimpleXML *aXml);
	void save(SimpleXML *aXml);

	virtual void on(SettingsManagerListener::Load, SimpleXML* xml) throw();
	virtual void on(SettingsManagerListener::Save, SimpleXML* xml) throw();
};

#endif // IGNOREMANAGER_H

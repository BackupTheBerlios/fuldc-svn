#include "stdinc.h"
#include "DCPlusPlus.h"

#include "IgnoreManager.h"
#include "wildcards.h"
#include "pme.h"
#include "Text.h"
#include "SimpleXML.h"

bool IgnoreManager::ignore(const string& aPattern) {
	Lock l(cs);

	pair<StringSetIter, bool> p = patterns.insert(aPattern);

	return p.second;
}

bool IgnoreManager::unignore(const string& aPattern) {
	Lock l(cs);

	size_t count = patterns.erase(aPattern);

	return count > 0;
}

bool IgnoreManager::isUserIgnored(const string& aNick) {
	Lock l(cs);

	StringSetIter i = patterns.find(aNick);

	return i != patterns.end();
}

bool IgnoreManager::isIgnored(const string& aNick) {
	Lock l(cs);

	bool ret = false;

	for(StringSetIter i = patterns.begin(); i != patterns.end(); ++i) {
		if(Util::strnicmp(*i, "$Re:", 4) == 0) {
			if((*i).length() > 4) {
				PME regexp((*i).substr(4), "gims");

				if(regexp.match(aNick) > 0) {
					ret = true;
					break;
				}
			}
		} else {
			ret = Wildcard::patternMatch(Text::toLower(aNick), Text::toLower(*i), false);
			if(ret)
				break;
		}
	}

	return ret;
}

void IgnoreManager::copyPatterns(StringSet& patterns) {
	Lock l(cs);

	patterns = this->patterns;
}

void IgnoreManager::replacePatterns(const StringSet& patterns) {
	Lock l(cs);

	this->patterns = patterns;
}


void IgnoreManager::save(SimpleXML *aXml) {
	Lock l(cs);

	aXml->addTag("IgnorePatterns");
	aXml->stepIn();

	for(StringSetIter i = patterns.begin(); i != patterns.end(); ++i) {
		aXml->addTag("Pattern");
		aXml->addChildAttrib("Match", *i);
	}

	aXml->stepOut();
}

void IgnoreManager::load(SimpleXML* aXml) {
	Lock l(cs);

	if(aXml->findChild("IgnorePatterns")) {
		aXml->stepIn();
		
		while(aXml->findChild("Pattern")) {
			ignore(aXml->getChildAttrib("Match"));
		}
		
		aXml->stepOut();
	}
}

void IgnoreManager::on(SettingsManagerListener::Load, SimpleXML* aXml) {
	load(aXml);
}

void IgnoreManager::on(SettingsManagerListener::Save, SimpleXML* aXml) {
	save(aXml);
}

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "highlightmanager.h"


HighlightManager::HighlightManager(void)
{
	SettingsManager::getInstance()->addListener(this);
}

HighlightManager::~HighlightManager(void)
{
	SettingsManager::getInstance()->removeListener(this);

	ColorIter i = colorSettings.begin();
	for(; i != colorSettings.end(); ++i)
		delete *i;
}

void HighlightManager::load(SimpleXML *aXml){
	aXml->resetCurrentChild();

	if(aXml->findChild("Highlights")) {
		aXml->stepIn();
		while(aXml->findChild("Highlight")) {
			ColorSettings *cs = new ColorSettings;
#ifdef UNICODE
			cs->setMatch( Util::utf8ToWide( aXml->getChildAttrib("Match") ) );
#else
			cs->setMatch( aXml->getChildAttrib("Match") );
#endif
			cs->setBold(	aXml->getBoolChildAttrib("Bold") );
			cs->setItalic( aXml->getBoolChildAttrib("Italic") );
			cs->setUnderline( aXml->getBoolChildAttrib("Underline") );
			cs->setStrikeout( aXml->getBoolChildAttrib("Strikeout") );
			cs->setIncludeNick( aXml->getBoolChildAttrib("IncludeNick") );
			cs->setCaseSensitive( aXml->getBoolChildAttrib("CaseSensitive") );
			cs->setWholeLine( aXml->getBoolChildAttrib("WholeLine") );
			cs->setWholeWord( aXml->getBoolChildAttrib("WholeWord") );
			cs->setPopup( aXml->getBoolChildAttrib("Popup") );
			cs->setTab( aXml->getBoolChildAttrib("Tab") );
			cs->setPlaySound( aXml->getBoolChildAttrib("PlaySound") );
			cs->setLog( aXml->getBoolChildAttrib("LastLog") );
			cs->setMatchType( aXml->getIntChildAttrib("MatchType") );
			cs->setHasFgColor( aXml->getBoolChildAttrib("HasFgColor") );
			cs->setHasBgColor( aXml->getBoolChildAttrib("HasBgColor") );
			cs->setBgColor( (int)aXml->getLongLongChildAttrib("BgColor") );
			cs->setFgColor( (int)aXml->getLongLongChildAttrib("FgColor") );
#ifdef UNICODE
			cs->setSoundFile( Util::utf8ToWide( aXml->getChildAttrib("SoundFile") ) );
#else
			cs->setSoundFile( aXml->getChildAttrib("SoundFile") );
#endif

			colorSettings.push_back(cs);
		}
		aXml->stepOut();
	} else {
		aXml->resetCurrentChild();
	}
}

void HighlightManager::save(SimpleXML *aXml){
	aXml->addTag("Highlights");
	aXml->stepIn();

	ColorIter iter = colorSettings.begin();
	for(;iter != colorSettings.end(); ++iter) {
		aXml->addTag("Highlight");
#ifdef UNICODE
		aXml->addChildAttrib("Match", Util::wideToUtf8((*iter)->getMatch()));
#else
		aXml->addChildAttrib("Match", (*iter)->getMatch());
#endif
		aXml->addChildAttrib("Bold", (*iter)->getBold());
		aXml->addChildAttrib("Italic", (*iter)->getItalic());
		aXml->addChildAttrib("Underline", (*iter)->getUnderline());
		aXml->addChildAttrib("Strikeout", (*iter)->getStrikeout());
		aXml->addChildAttrib("IncludeNick", (*iter)->getIncludeNick());
		aXml->addChildAttrib("CaseSensitive", (*iter)->getCaseSensitive());
		aXml->addChildAttrib("WholeLine", (*iter)->getWholeLine());
		aXml->addChildAttrib("WholeWord", (*iter)->getWholeWord());
		aXml->addChildAttrib("Popup", (*iter)->getPopup());
		aXml->addChildAttrib("Tab", (*iter)->getTab());
		aXml->addChildAttrib("PlaySound", (*iter)->getPlaySound());
		aXml->addChildAttrib("LastLog", (*iter)->getLog());
		aXml->addChildAttrib("MatchType", (*iter)->getMatchType());
		aXml->addChildAttrib("HasFgColor", (*iter)->getHasFgColor());
		aXml->addChildAttrib("HasBgColor", (*iter)->getHasBgColor());
		aXml->addChildAttrib("FgColor", Util::toString((*iter)->getFgColor()));
		aXml->addChildAttrib("BgColor", Util::toString((*iter)->getBgColor()));
#ifdef UNICODE
		aXml->addChildAttrib("SoundFile", Util::wideToUtf8((*iter)->getSoundFile()));
#else
		aXml->addChildAttrib("SoundFile", (*iter)->getSoundFile());
#endif
	}//end for

	aXml->stepOut();
}

ColorList* HighlightManager::rLock(){
	lock.enterRead();
	return &colorSettings;
}
ColorList* HighlightManager::wLock(){
	lock.enterWrite();
	return &colorSettings;
}

void HighlightManager::rUnlock(){
	lock.leaveRead();
}

void HighlightManager::wUnlock(){
	lock.leaveWrite();
}

void HighlightManager::on(SettingsManagerListener::Load, SimpleXML* xml){
	load(xml);
}

void HighlightManager::on(SettingsManagerListener::Save, SimpleXML* xml){
	save(xml);
}

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
	if(aXml->findChild("Highlights")) {
		aXml->stepIn();
		while(aXml->findChild("Highlight")) {
			ColorSettings *cs = new ColorSettings;

			cs->setMatch( aXml->getChildAttrib("Match") );
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
			cs->setMatchType( aXml->getIntChildAttrib("MatchType") );
			cs->setHasFgColor( aXml->getBoolChildAttrib("HasFgColor") );
			cs->setHasBgColor( aXml->getBoolChildAttrib("HasBgColor") );
			cs->setBgColor( aXml->getLongLongChildAttrib("BgColor") );
			cs->setFgColor( aXml->getLongLongChildAttrib("FgColor") );
			cs->setSoundFile( aXml->getChildAttrib("SoundFile") );

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

		aXml->addChildAttrib("Match", (*iter)->getMatch());
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
		aXml->addChildAttrib("MatchType", (*iter)->getMatchType());
		aXml->addChildAttrib("HasFgColor", (*iter)->getHasFgColor());
		aXml->addChildAttrib("HasBgColor", (*iter)->getHasBgColor());
		aXml->addChildAttrib("FgColor", Util::toString((*iter)->getFgColor()));
		aXml->addChildAttrib("BgColor", Util::toString((*iter)->getBgColor()));
		aXml->addChildAttrib("SoundFile", (*iter)->getSoundFile());
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

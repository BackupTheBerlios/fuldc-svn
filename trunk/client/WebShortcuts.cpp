/* 
 * Copyright (C) 2003 Opera, opera@home.se
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
#include "StringTokenizer.h"
#include "oxmllib.h"
#include "SimpleXML.h"
#include "WebShortcuts.h"

WebShortcuts::WebShortcuts() {
	SettingsManager::getInstance()->addListener(this);

	tstring s = Text::toT(OXMLLib::decode("As URL&#1;u&#1;%s&#2;Google&#1;g&#1;http://www.google.com/search?q=%s&#2;IMDB&#1;i&#1;http://www.imdb.com/Find?select=All&amp;for=%s&#2;TV Tome&#1;t&#1;http://www.tvtome.com/tvtome/servlet/Search?searchType=all&amp;searchString=%s", false));

	StringTokenizer<tstring> st(s, _T('\x02'));
	for (TStringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		StringTokenizer<tstring> st_i(*i, _T('\x01'));
		dcassert(st_i.getTokens().size() == 3);
		list.push_back(new WebShortcut(st_i.getTokens()[0], st_i.getTokens()[1], st_i.getTokens()[2]));
	}
}

WebShortcuts::~WebShortcuts() {
	SettingsManager::getInstance()->removeListener(this);
}

void WebShortcuts::load(SimpleXML* xml) {
	xml->resetCurrentChild();

	if(xml->findChild("WebShortcuts")){
		xml->stepIn();
		WebShortcut* tmp = NULL;

		clear();

		while(xml->findChild("WebShortcut")){
			tmp = new WebShortcut();

			tmp->name  = Text::toT(xml->getChildAttrib("Name"));
			tmp->key   = Text::toT(xml->getChildAttrib("Key"));
			tmp->url   = Text::toT(OXMLLib::decode(xml->getChildAttrib("URL"), false));

			tmp->clean = xml->getBoolChildAttrib("Clean");
			list.push_back(tmp);
		}
		
		xml->stepOut();
		return;
	}
	
	xml->resetCurrentChild();
	if(xml->findChild("Settings")){
		xml->stepIn();
		if(xml->findChild("WebShortcuts")){

			tstring s = Text::toT(OXMLLib::decode( xml->getChildData(), false));

			clear();

			StringTokenizer<tstring> st(s, _T('\x02'));
			for (TStringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
				StringTokenizer<tstring> st_i(*i, _T('\x01'));
				dcassert(st_i.getTokens().size() == 3);
				list.push_back(new WebShortcut(st_i.getTokens()[0], st_i.getTokens()[1], st_i.getTokens()[2]));
			}
		}
		xml->stepOut();
	}
}

void WebShortcuts::save(SimpleXML* xml) {
	xml->addTag("WebShortcuts");
	xml->stepIn();
	for(WebShortcut::Iter i = list.begin(); i != list.end(); ++i){
		xml->addTag("WebShortcut");
#ifdef UNICODE
		xml->addChildAttrib("Name", Text::wideToUtf8((*i)->name));
		xml->addChildAttrib("Key", Text::wideToUtf8((*i)->key));
		xml->addChildAttrib("URL", Text::wideToUtf8((*i)->url));
#else
		xml->addChildAttrib("Name", (*i)->name);
		xml->addChildAttrib("Key", (*i)->key);
		xml->addChildAttrib("URL", (*i)->url);
#endif
		xml->addChildAttrib("Clean", (*i)->clean);
	}
	xml->stepOut();
}

WebShortcut* WebShortcuts::getShortcutByName(const tstring& name) {
	for (WebShortcut::Iter i = list.begin(); i != list.end(); ++i)
		if ( (*i)->name == name )
			return *i;
	return NULL;
}
WebShortcut* WebShortcuts::getShortcutByKey(const tstring& key) {
	for (WebShortcut::Iter i = list.begin(); i != list.end(); ++i)
		if ( (*i)->key == key )
			return *i;
	return NULL;
}

WebShortcut* WebShortcuts::getShortcutByName(WebShortcut::List& _list, const tstring& name) {
	for (WebShortcut::Iter i = _list.begin(); i != _list.end(); ++i)
		if ( (*i)->name == name )
			return *i;
	return NULL;
}
WebShortcut* WebShortcuts::getShortcutByKey(WebShortcut::List& _list, const tstring& key) {
	for (WebShortcut::Iter i = _list.begin(); i != _list.end(); ++i)
		if ( (*i)->key == key )
			return *i;
	return NULL;
}

WebShortcut::List WebShortcuts::copyList() {
	WebShortcut::List lst;
	for (WebShortcut::Iter i = list.begin(); i != list.end(); ++i)
		lst.push_back(new WebShortcut( (*i)->name, (*i)->key, (*i)->url, (*i)->clean ));
	return lst;
}
void WebShortcuts::replaceList(WebShortcut::List& new_list) {
	clear();
	for (WebShortcut::Iter i = new_list.begin(); i != new_list.end(); ++i)
		list.push_back(new WebShortcut( (*i)->name, (*i)->key, (*i)->url, (*i)->clean ));
}

void WebShortcuts::clear() {
	for (WebShortcut::Iter i = list.begin(); i != list.end(); ++i)
		delete *i;
	list.clear();
}

void WebShortcuts::on(SettingsManagerListener::Save, SimpleXML* xml) {
	save(xml);
}
void WebShortcuts::on(SettingsManagerListener::Load, SimpleXML* xml) {
	load(xml);
}

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

#ifndef _WEBSHORTCUTS_H
#define _WEBSHORTCUTS_H

#include "Singleton.h"
#include "Speaker.h"

class WebShortcutsListener {
public:
	typedef WebShortcutsListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	enum Types {
		CHANGED
	};

	virtual void onAction(Types) throw() {}
};

class WebShortcut {
public:
	typedef WebShortcut* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	WebShortcut(const string& _name, const string& _key, const string& _url) :
	name(_name), key(_key), url(_url) { }
	WebShortcut() {}

	string name;
	string key;
	string url;
};

class WebShortcuts : public Speaker<WebShortcutsListener>, public Singleton<WebShortcuts> {
public:
	void load();
	void save();

	void propagateChanges() {
		fire(WebShortcutsListener::CHANGED);
	}

	WebShortcut* getShortcutByName(const string& name);
	WebShortcut* getShortcutByKey(const string& key);
	static WebShortcut* getShortcutByName(WebShortcut::List& _list, const string& name);
	static WebShortcut* getShortcutByKey(WebShortcut::List& _list, const string& key);

	WebShortcut::List copyList();
	void replaceList(WebShortcut::List& new_list);

	WebShortcut::List list;
private:
	void clear();
};

#endif // _WEBSHORTCUTS_H

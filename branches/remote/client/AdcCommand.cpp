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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "AdcCommand.h"

void AdcCommand::parse(const string& aLine, bool nmdc /* = false */) throw(ParseException) {
	string::size_type i = 5;

	if(nmdc) {
		// "$ADCxxx ..."
		if(aLine.length() < 7)
			throw ParseException("Too short");
		type = TYPE_CLIENT;
		memcpy(cmd, &aLine[4], 3);
		i += 3;
	} else {
		// "yxxx cidcidcidcid..."
		if(aLine.length() < 5 + (8 * 8 + 7) / 5)
			throw ParseException("Too short");
		type = aLine[0];
		memcpy(cmd, &aLine[1], 3);
	}

	string::size_type len = aLine.length();
	const char* buf = aLine.c_str();
	string cur;
	cur.reserve(128);

	while(i < len) {
		switch(buf[i]) {
		case '\\':
			++i;
			if(i == len)
				throw ParseException("Escape at eol");
			if(buf[i] == 's')
				cur += ' ';
			else if(buf[i] == 'n')
				cur += '\n';
			else if(buf[i] == '\\')
				cur += '\\';
			else if(buf[i] == ' ' && nmdc)	// $ADCGET escaping, leftover from old specs
				cur += ' ';
			else
				throw ParseException("Unknown escape");
			break;
		case ' ':
			// New parameter...
			{
				parameters.push_back(cur);
				cur.clear();
			}
			break;
		default:
			cur += buf[i];
		}
		++i;
	}
	if(!cur.empty()) {
		parameters.push_back(cur);
	}
}

string AdcCommand::toString(bool nmdc /* = false */, bool old /* = false */) const {
	string tmp;
	if(nmdc) {
		tmp += "$ADC";
	} else {
		tmp += getType();
	}

	tmp += cmdChar;

	for(StringIterC i = getParameters().begin(); i != getParameters().end(); ++i) {
		tmp += ' ';
		tmp += escape(*i, old);
	}
	if(nmdc) {
		tmp += '|';
	} else {
		tmp += '\n';
	}
	return tmp;
}

bool AdcCommand::getParam(const char* name, size_t start, string& ret) const {
	for(string::size_type i = start; i < getParameters().size(); ++i) {
		if(toCode(name) == toCode(getParameters()[i].c_str())) {
			ret = getParameters()[i].substr(2);
			return true;
		}
	}
	return false;
}

bool AdcCommand::hasFlag(const char* name, size_t start) const {
	for(string::size_type i = start; i < getParameters().size(); ++i) {
		if(toCode(name) == toCode(getParameters()[i].c_str()) &&
			getParameters()[i][2] == '1' &&
			getParameters()[i].size() == 3)
		{
			return true;
		}
	}
	return false;
}

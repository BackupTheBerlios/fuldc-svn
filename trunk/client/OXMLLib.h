/* 
 * Copyright (C) 2002 Opera, opera@home.se
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

#ifndef _OXMLLIB_H
#define _OXMLLIB_H

#include "Util.h"

class OXMLLib {
public:
	static string encode(const string& s, bool bArguments = true) {
		string::size_type i = 0;
		string sChars = bArguments ? "<&>'\"" : "<&>";
		for (int c = 1; c < 32; ++c)
			sChars += (char)c;
		const char* chars = sChars.c_str();

		i = s.find_first_of(chars);
		if(i == string::npos) {
			return s;
		} else {
			string tmp = s;
			do {
				if (tmp[i] >= 0 && tmp[i] < 32) {
					string stmp = "&#" + Util::toString((int)tmp[i]) + ";";
					tmp.replace(i, 1, stmp.c_str());
					i += stmp.length();
					continue;
				} else {
					switch(tmp[i]) {
						case '<': tmp.replace(i, 1, "&lt;"); i+=4; break;
						case '&': tmp.replace(i, 1, "&amp;"); i+=5; break;
						case '>': tmp.replace(i, 1, "&gt;"); i+=4; break;
						case '\'': tmp.replace(i, 1, "&apos;"); i+=6; break;
						case '"': tmp.replace(i, 1, "&quot;"); i+=6; break;
						default: dcassert(0);
					}
				}
			} while( (i = tmp.find_first_of(chars, i)) != string::npos);
			return tmp;
		}
	}
	static string decode(const string& s, bool bArguments = true) {
		string::size_type i = 0;

		if(s.find('&') == string::npos) {
			return s;
		} else {
			string tmp = s;
			while( (i=tmp.find("&lt;")) != string::npos) {
				tmp.replace(i, 4, 1, '<');
			}
			while( (i=tmp.find("&gt;")) != string::npos) {
				tmp.replace(i, 4, 1, '>');
			}
			while( (i=tmp.find("&#")) != string::npos) {
				string::size_type j = tmp.find(';', i);
				if (j == string::npos)
					break;
				string sNumber = tmp.substr(i+2, j-i-2);
				int iNumber = Util::toInt(sNumber);
				dcassert(iNumber > 0 || iNumber < 256);
				tmp.replace(i, 3 + sNumber.length(), 1, (char)iNumber);
			}
			if(bArguments) {
				while( (i=tmp.find("&apos;")) != string::npos) {
					tmp.replace(i, 6, 1, '\'');
				}
				while( (i=tmp.find("&quot;")) != string::npos) {
					tmp.replace(i, 6, 1, '"');
				}
			}
			i = 0;
			while( (i=tmp.find("&amp;", i)) != string::npos) {
				tmp.replace(i, 5, 1, '&');
				i -= 4;
			}
			return tmp;
		}
	}
};

#endif // _OXMLLIB_H

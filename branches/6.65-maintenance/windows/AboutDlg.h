﻿/*
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

#if !defined(ABOUT_DLG_H)
#define ABOUT_DLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/HttpConnection.h"
#include "../client/SimpleXML.h"

static const TCHAR thanks[] = _T("Big thanks to all donators and people who have contributed with ideas ")
_T("and code! Thanks go out to sourceforge for hosting the project. This application uses bzip2 (www.bzip.org), ")
_T("thanks to Julian Seward and team for providing it. Thiz application uses zlib (www.zlib.net), ")
_T("thanks to Jean-loup Gailly and Mark Adler for providing it. ")
_T("This product includes GeoIP data created by MaxMind, available from http://maxmind.com/. ")
_T("This product uses yassl from www.yassl.com, thanks to Todd Ouska and Larry Stefonic.")
_T("The following people have contributed code to ")
_T("DC++ (I hope I haven't missed someone, they're in roughly chronological order...=):\r\n")
_T("geoff, carxor, luca rota, dan kline, mike, anton, zc, sarf, farcry, kyrre aalerud, opera, ")
_T("patbateman, xeroc, fusbar, vladimir marko, kenneth skovhede, ondrea, todd pederzani, who, ")
_T("sedulus, sandos, henrik engstr?m, dwomac, robert777, saurod, atomicjo, bzbetty, orkblutt, ")
_T("distiller, citruz, dan fulger, cologic, christer palm, twink, ilkka seppï¿½lï¿½, johnny, ciber, ")
_T("theparanoidone, gadget, naga, tremor, joakim tosteberg, pofis, psf8500, lauris ievins, ")
_T("defr, ullner, fleetcommand, liny, xan, olle svensson, mark gillespie, jeremy huddleston, ")
_T("bsod, sulan, jonathan stone, tim burton, izzzo, guitarm, paka, nils maier, jens oknelid, yoji, ")
_T("krzysztof tyszecki, poison, pothead, pur, bigmuscle, martin, jove, bart vullings, ")
_T("steven sheehy, tobias nygren, poy, dorian, stephan hohe, mafa_45, mikael eman. ")
_T("Keep it coming!");

static const TCHAR fulthanks[] = 
_T("Thanks to Snowflake/Summerfling/Goose for his work on the lovely fulDC website. ")
_T("Thanks to fnordpojk for 0xBADC0DE and to DCPortal.net for hosting the website. ")
_T("Thanks to fusbar for all his advice. ")
_T("Thanks to farmer for his advice on default colors. ")
_T("Thanks to manos for his nice icons. ")
_T("Thanks to Banten for his lovely icons. ")
_T("Thanks to Irean for her help with the graphics. ")
_T("Thanks to xAyiDe for his threaded file list code.");

class AboutDlg : public CDialogImpl<AboutDlg>, private HttpConnectionListener
{
public:
	enum { IDD = IDD_ABOUTBOX };
	enum { WM_VERSIONDATA = WM_APP + 53 };

	AboutDlg() { }
	virtual ~AboutDlg() { }

	BEGIN_MSG_MAP(AboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_VERSIONDATA, onVersionData)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		SetDlgItemText(IDC_VERSION, _T("DC++ v") _T(VERSIONSTRING) _T("\n(c) Copyright 2001-2006 Jacek Sieka\nCodeveloper: Per Lindén\nGraphics: Martin Skogevall\nDC++ is licenced under GPL\nhttp://dcplusplus.sourceforge.net/"));
		CEdit ctrl(GetDlgItem(IDC_THANKS));
		ctrl.FmtLines(TRUE);
		ctrl.AppendText(thanks, TRUE);
		ctrl.Detach();

		ctrl.Attach(GetDlgItem(IDC_FULTHANKS));
		ctrl.FmtLines(TRUE);
		ctrl.AppendText(fulthanks, TRUE);
		ctrl.Detach();

		SetDlgItemText(IDC_TTH, WinUtil::tth.c_str());
		SetDlgItemText(IDC_LATEST, CTSTRING(DOWNLOADING));
		SetDlgItemText(IDC_TOTALS, Text::toT("Upload: " + Util::formatBytes(SETTING(TOTAL_UPLOAD)) + ", Download: " +
			Util::formatBytes(SETTING(TOTAL_DOWNLOAD))).c_str());

		if(SETTING(TOTAL_DOWNLOAD) > 0) {
			TCHAR buf[64];
			_stprintf(buf, _T("Ratio (up/down): %.2f"), ((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)));
			SetDlgItemText(IDC_RATIO, buf);
		}

		tstring time = Util::formatTimeW(GET_TIME() - WinUtil::startTime);
		SetDlgItemText(IDC_UPTIME, time.c_str());

		CenterWindow(GetParent());
		c.addListener(this);
		c.downloadFile("http://dcplusplus.sourceforge.net/version.xml");
		return TRUE;
	}

	LRESULT onVersionData(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		tstring* x = (tstring*) wParam;
		SetDlgItemText(IDC_LATEST, x->c_str());
		delete x;
		return 0;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}

private:
	HttpConnection c;

	AboutDlg(const AboutDlg&) { dcassert(0); }

	virtual void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
		downBuf.append((char*)buf, len);
	}

	virtual void on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw() {
		if(!downBuf.empty()) {
			SimpleXML xml;
			xml.fromXML(downBuf);
			if(xml.findChild("DCUpdate")) {
				xml.stepIn();
				if(xml.findChild("Version")) {
					tstring* x = new tstring(Text::toT(xml.getChildData()));
					PostMessage(WM_VERSIONDATA, (WPARAM) x);
				}
			}
		}
		conn->removeListener(this);
	}

	virtual void on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw() {
		tstring* x = new tstring(Text::toT(aLine));
		PostMessage(WM_VERSIONDATA, (WPARAM) x);
		conn->removeListener(this);
	}

	string downBuf;
};

#endif // !defined(ABOUT_DLG_H)

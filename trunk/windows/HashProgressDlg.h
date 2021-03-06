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

#if !defined(HASH_PROGRESS_DLG_H)
#define HASH_PROGESS_DLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/HashManager.h"

class HashProgressDlg : public CDialogImpl<HashProgressDlg>
{
public:
	enum { IDD = IDD_HASH_PROGRESS };
	enum { WM_VERSIONDATA = WM_APP + 53 };

	HashProgressDlg(bool aAutoClose = false) : autoClose(aAutoClose), startTime(GET_TICK()), startBytes(0), startFiles(0) {

	}
	virtual ~HashProgressDlg() { }

	BEGIN_MSG_MAP(HashProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER, onTimer)
		MESSAGE_HANDLER(WM_DESTROY, onDestroy)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		// Translate static strings
		SetWindowText(CTSTRING(HASH_PROGRESS));
		SetDlgItemText(IDOK, CTSTRING(HASH_PROGRESS_BACKGROUND));
		SetDlgItemText(IDC_STATISTICS, CTSTRING(HASH_PROGRESS_STATS));
		SetDlgItemText(IDC_HASH_INDEXING, CTSTRING(HASH_PROGRESS_TEXT));

		string tmp;
		startTime = GET_TICK();
		HashManager::getInstance()->getStats(tmp, startBytes, startFiles);

		progress.Attach(GetDlgItem(IDC_HASH_PROGRESS));
		progress.SetRange(0, 10000);
		updateStats();

		SetTimer(1, 1000);
		return TRUE;
	}

	LRESULT onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		updateStats();
		return 0;
	}
	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		progress.Detach();
		KillTimer(1);
		return 0;
	}

	void updateStats() {
		string file;
		int64_t bytes = 0;
		size_t files = 0;
		u_int32_t tick = GET_TICK();

		HashManager::getInstance()->getStats(file, bytes, files);
		if(bytes > startBytes)
			startBytes = bytes;

		if(files > startFiles)
			startFiles = files;

		if(autoClose && files == 0) {
			PostMessage(WM_CLOSE);
			return;
		}
		double diff = tick - startTime;
		if(diff < 1000 || files == 0 || bytes == 0) {
			SetDlgItemText(IDC_FILES_PER_HOUR, Text::toT("-.-- " + STRING(FILES_PER_HOUR) + ", " + Util::toString((u_int32_t)files) + " " + STRING(FILES_LEFT)).c_str());
			SetDlgItemText(IDC_HASH_SPEED, Text::toT("-.-- B/s, " + Util::formatBytes(bytes) + " " + STRING(LEFT)).c_str());
			SetDlgItemText(IDC_TIME_LEFT, Text::toT("-:--:-- " + STRING(LEFT)).c_str());
			progress.SetPos(0);
		} else {
			double filestat = (((double)(startFiles - files)) * 60 * 60 * 1000) / diff;
			double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

			SetDlgItemText(IDC_FILES_PER_HOUR, Text::toT(Util::toString(filestat) + " " + STRING(FILES_PER_HOUR) + ", " + Util::toString((u_int32_t)files) + " " + STRING(FILES_LEFT)).c_str());
			SetDlgItemText(IDC_HASH_SPEED, Text::toT(Util::formatBytes((int64_t)speedStat) + "/s, " + Util::formatBytes(bytes) + " " + STRING(LEFT)).c_str());

			if(filestat == 0 || speedStat == 0) {
				SetDlgItemText(IDC_TIME_LEFT, Text::toT("-:--:-- " + STRING(LEFT)).c_str());
			} else {
				double fs = files * 60 * 60 / filestat;
				double ss = bytes / speedStat;

				SetDlgItemText(IDC_TIME_LEFT, Text::toT(Util::formatSeconds((int64_t)(fs + ss) / 2) + " " + STRING(LEFT)).c_str());
			}
		}

		if(files == 0) {
			SetDlgItemText(IDC_CURRENT_FILE, CTSTRING(DONE));
		} else {
			SetDlgItemText(IDC_CURRENT_FILE, Text::toT(file).c_str());
		}

		if(startFiles == 0 || startBytes == 0) {
			progress.SetPos(0);
		} else {
			progress.SetPos((int)(10000 * ((0.5 * (startFiles - files)/startFiles) + 0.5 * (startBytes - bytes) / startBytes)));
		}
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if( IsWindow() )
			DestroyWindow();
		return 0;
	}

private:

	bool autoClose;
	int64_t startBytes;
	size_t startFiles;
	u_int32_t startTime;
	CProgressBarCtrl progress;

};

#endif // !defined(HASH_PROGRESS_DLG_H)

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "FulIgnorePage.h"
#include "LineDlg.h"

#include "../client/IgnoreManager.h"
#include "../client/version.h"

PropPage::TextItem FulIgnorePage::texts[] = {
	{ IDC_ADD,		ResourceManager::NEW				},
	{ IDC_REMOVE,	ResourceManager::REMOVE				},
	{ 0,			ResourceManager::SETTINGS_AUTO_AWAY	}
};

LRESULT FulIgnorePage::onInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	PropPage::translate((HWND)(*this), texts);

	IgnoreManager::getInstance()->copyPatterns(patterns);

	patternList.Attach(GetDlgItem(IDC_PATTERNS));
	CRect rc;
	patternList.GetClientRect(rc);
	patternList.InsertColumn(0, _T("Dummy"), LVCFMT_LEFT, rc.Width(), 0);

	for(StringSetIter i = patterns.begin(); i != patterns.end(); ++i) {
		patternList.insert(0, Text::toT(*i));
	}

	return 0;
}

LRESULT FulIgnorePage::onAdd(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	LineDlg dlg;
	dlg.title = TSTRING(NEW_IGNORE_PATTERN_TITLE);
	dlg.description = TSTRING(NEW_IGNORE_PATTERN_DESCRIPTION);

	if(dlg.DoModal() == IDOK) {
		string pattern = Text::fromT(dlg.line);

		pair<StringSetIter, bool> p = patterns.insert(pattern);

		if(p.second) {
			patternList.insert(0, dlg.line);
		} else {
			MessageBox(CTSTRING(ALREADY_IGNORED), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_OK);
		}
	}

	return 0;
}

LRESULT FulIgnorePage::onRemove(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	int i = -1;
	
	//to account for enormous regexps =)
	TCHAR* buf = new TCHAR[1024];

	while((i = patternList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		patternList.GetItemText(i, 0, buf, 1024);
		string pattern = Text::fromT(buf);

		patterns.erase(pattern);
		patternList.DeleteItem(i);
	}

	delete[] buf;

	return 0;
}

void FulIgnorePage::write() {
	IgnoreManager::getInstance()->replacePatterns(patterns);
}

LRESULT FulIgnorePage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULIGNOREPAGE);
	return 0;
}

LRESULT FulIgnorePage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULIGNOREPAGE);
	return 0;
}

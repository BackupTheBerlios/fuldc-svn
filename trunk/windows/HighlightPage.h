#ifndef HIGHLIGHTPAGE_H
#define HIGHLIGHTPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "../client/SettingsManager.h"

class HighlightPage: public CPropertyPage<IDD_HIGHLIGHTPAGE>, public PropPage
{
	public:
	HighlightPage(SettingsManager *s) : PropPage(s) {
		SetTitle(CSTRING(SETTINGS_FUL_HIGHLIGHT));
	};
	virtual ~HighlightPage();

	BEGIN_MSG_MAP(HighlightPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_DELETE, onDelete)
		COMMAND_ID_HANDLER(IDC_UPDATE, onUpdate)
		COMMAND_ID_HANDLER(IDC_MOVEUP, onMove)
		COMMAND_ID_HANDLER(IDC_MOVEDOWN, onMove)
		COMMAND_ID_HANDLER(IDC_BGCOLOR, onBgColor)
		COMMAND_ID_HANDLER(IDC_FGCOLOR, onFgColor)
		COMMAND_ID_HANDLER(IDC_SELECT_SOUND, onSelSound)
		
		NOTIFY_HANDLER(IDC_ITEMS, LVN_ITEMCHANGED, onItemChanged)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	
	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onFgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSelSound(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	//virtual void setTitle(const string& t) { SetTitle(t.c_str()); };

	void getValues(ColorSettings* );
private:
	CComboBox ctrlMatchType;
	ExListViewCtrl ctrlStrings;

	COLORREF bgColor;
	COLORREF fgColor;

	string soundFile;

	//reset all controls except listview
	void clear();
	
};

#endif //HIGHLIGHTPAGE_H

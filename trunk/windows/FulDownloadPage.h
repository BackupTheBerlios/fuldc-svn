#ifndef FULPAGE2_H
#define FULPAGE2_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"
#include "ExListViewCtrl.h"

class FulDownloadPage : public CPropertyPage<IDD_FULDOWNLOADPAGE>, public PropPage
{
public:
	FulDownloadPage(SettingsManager *s) : PropPage(s) {
		SetTitle( CTSTRING(SETTINGS_FUL_DOWNLOAD) );
	};
	~FulDownloadPage() {
		ctrlDownload.Detach();
	};

	BEGIN_MSG_MAP(FulDownloadPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];

	//just save the copy here to avoid searching the list
	StringMap download;

	ExListViewCtrl ctrlDownload;
};

#endif
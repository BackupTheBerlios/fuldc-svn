#ifndef FULPAGE_H
#define FULPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class FulPage : public CPropertyPage<IDD_FULPAGE>, public PropPage
{
public:
	FulPage(SettingsManager *s) : PropPage(s) {	};
	~FulPage() {};

	BEGIN_MSG_MAP(FulPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
		
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	virtual void setTitle(const string& t) { SetTitle(t.c_str()); };
	
protected:
	static Item items[];
};

#endif //FULPAGE_H

/* 
 * Copyright (C) 2001-2005 Jacek Sieka, j_s at telia com
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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "PropertiesDlg.h"

#include "GeneralPage.h"
#include "DownloadPage.h"
#include "UploadPage.h"
#include "AppearancePage.h"
#include "AdvancedPage.h"
#include "LogPage.h"
#include "UCPage.h"
#include "Appearance2Page.h"
#include "Advanced3Page.h"
#include "NetworkPage.h"
#include "FulHighlightPage.h"
#include "FulSharePage.h"
#include "FulDownloadPage.h"
#include "FulAdvancedPage.h"
#include "FulAppearance.h"
#include "FulTabsPage.h"
#include "FulPopupsPage.h"

PropertiesDlg::PropertiesDlg(SettingsManager *s) : TreePropertySheet(CTSTRING(SETTINGS))
{
	pages[0]  = new GeneralPage(s);
	pages[1]  = new NetworkPage(s);
	pages[2]  = new DownloadPage(s);
	pages[3]  = new UploadPage(s);
	pages[4]  = new AppearancePage(s);
	pages[5]  = new Appearance2Page(s);
	pages[6]  = new LogPage(s);
	pages[7]  = new AdvancedPage(s);
	pages[8]  = new Advanced3Page(s);
	pages[9]  = new UCPage(s);
	pages[10] = new FulDownloadPage(s);
	pages[11] = new FulSharePage(s);
	pages[12] = new FulAdvancedPage(s);
	pages[13] = new FulAppearancePage(s);
	pages[14] = new FulHighlightPage(s);
	pages[15] = new FulPopupsPage(s);
	pages[16] = new FulTabsPage(s);

	for(int i=0; i<numPages; i++) {
		AddPage(pages[i]->getPSP());
	}

	// Hide "Apply" button
	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	m_psh.dwFlags &= ~PSH_HASHELP;
}

PropertiesDlg::~PropertiesDlg()
{
	for(int i=0; i<numPages; i++) {
		delete pages[i];
	}
}

void PropertiesDlg::write()
{
	for(int i=0; i<numPages; i++)
	{
		// Check HWND of page to see if it has been created
		const HWND page = PropSheet_IndexToHwnd((HWND)*this, i);

		if(page != NULL)
			pages[i]->write();
	}
}

LRESULT PropertiesDlg::onOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	write();
	bHandled = FALSE;
	return TRUE;
}

/**
 * @file
 * $Id: PropertiesDlg.cpp,v 1.3 2004/02/12 22:43:57 trem Exp $
 */


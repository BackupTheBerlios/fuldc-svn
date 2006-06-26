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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "MainFrm.h"
#include "AboutDlg.h"
#include "HubFrame.h"
#include "SearchFrm.h"
#include "PublicHubsFrm.h"
#include "PropertiesDlg.h"
#include "UsersFrame.h"
#include "DirectoryListingFrm.h"
#include "FavoritesFrm.h"
#include "NotepadFrame.h"
#include "QueueFrame.h"
#include "SpyFrame.h"
#include "FinishedDLFrame.h"
#include "ADLSearchFrame.h"
#include "FinishedULFrame.h"
#include "TextFrame.h"
#include "StatsFrame.h"
#include "WaitingUsersFrame.h"
#include "LineDlg.h"
#include "HashProgressDlg.h"
#include "UPnP.h"
#include "SystemFrame.h"
#include "PopupManager.h"
#include "PrivateFrame.h"

#include "../client/ConnectionManager.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/StringTokenizer.h"
#include "../client/SimpleXML.h"
#include "../client/ShareManager.h"
#include "../client/version.h"

MainFrame::MainFrame() : trayMessage(0), trayIcon(false), maximized(false), lastUpload(-1), lastUpdate(0), 
lastUp(0), lastDown(0), oldshutdown(false), stopperThread(NULL), c(new HttpConnection()), 
closing(false), missedAutoConnect(false), UPnP_TCPConnection(NULL), UPnP_UDPConnection(NULL),
 hashProgress(false), timerTime(0)
{ 
	memset(statusSizes, 0, sizeof(statusSizes));
	
	links.homepage = _T("http://dcpp.net/");
	links.downloads = links.homepage + _T("download/");
	//links.geoipfile = _T("http://www.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip");
	links.translations = _T("http://sourceforge.net/tracker/?atid=460289&group_id=40287");
	links.faq = links.homepage + _T("faq/");
	links.help = links.homepage + _T("forum/");
	links.discuss = links.homepage + _T("forum/");
	links.features = links.homepage + _T("bugzilla/");
	links.bugs = links.homepage + _T("bugzilla/");
}

MainFrame::~MainFrame() {
	m_CmdBar.m_hImageList = NULL;

	images.Destroy();
	largeImages.Destroy();
	largeImagesHot.Destroy();

	WinUtil::uninit();
}

DWORD WINAPI MainFrame::stopper(void* p) {
	MainFrame* mf = (MainFrame*)p;
	HWND wnd, wnd2 = NULL;

	while( (wnd=::GetWindow(mf->m_hWndMDIClient, GW_CHILD)) != NULL) {
		if(wnd == wnd2) 
			Sleep(100);
		else { 
			::PostMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}

	mf->PostMessage(WM_CLOSE);	
	return 0;
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	TimerManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);

	WinUtil::init(m_hWnd);

	trayMessage = RegisterWindowMessage(_T("TaskbarCreated"));

	TimerManager::getInstance()->start();

	// Set window name
	SetWindowText(_T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" ") _T(FULDC) _T(" ") _T(FULVERSIONSTRING));

	// Load images
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);

	m_hMenu = WinUtil::mainMenu;

	// attach menu
	m_CmdBar.AttachMenu(m_hMenu);
	// load command bar images
	images.CreateFromImage(WinUtil::getIconPath(_T("toolbar.bmp")).c_str(), 16, 33, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	m_CmdBar.m_hImageList = images;
	if(images.GetImageCount() > 22) {
		//File
		m_CmdBar.m_arrCommand.Add(IDC_OPEN_FILE_LIST);
		m_CmdBar.m_arrCommand.Add(IDC_OPEN_OWN_LIST);
		m_CmdBar.m_arrCommand.Add(IDC_REFRESH_FILE_LIST);
		m_CmdBar.m_arrCommand.Add(IDC_OPEN_DOWNLOADS);
		m_CmdBar.m_arrCommand.Add(ID_FILE_QUICK_CONNECT);
		m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
		m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
		m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);

		//View
		m_CmdBar.m_arrCommand.Add(ID_FILE_CONNECT);
		m_CmdBar.m_arrCommand.Add(IDC_QUEUE);
		m_CmdBar.m_arrCommand.Add(IDC_WAITING_USERS);
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED_DL);
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL);
		m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
		m_CmdBar.m_arrCommand.Add(IDC_FAVUSERS);
		m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_SEARCH_SPY);
		m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
		m_CmdBar.m_arrCommand.Add(IDC_SYSTEM_LOG);
		m_CmdBar.m_arrCommand.Add(IDC_NET_STATS);
		m_CmdBar.m_arrCommand.Add(IDC_HASH_PROGRESS);
		
		//Window
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_CASCADE);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_HORZ);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_VERT);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_ARRANGE);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_MINIMIZE_ALL);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_RESTORE_ALL);
		m_CmdBar.m_arrCommand.Add(IDC_CLOSE_DISCONNECTED);
		m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_PM);
		m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_OFFLINE_PM);
		m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_DIR_LIST);
		m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_SEARCH_FRAME);
	} else if(images.GetImageCount() == 22) {
		m_CmdBar.m_arrCommand.Add(IDC_OPEN_FILE_LIST);
		m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
		m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
		m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);
		m_CmdBar.m_arrCommand.Add(ID_FILE_CONNECT);
		m_CmdBar.m_arrCommand.Add(IDC_QUEUE);
		m_CmdBar.m_arrCommand.Add(IDC_WAITING_USERS);
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED_DL);
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL);
		m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
		m_CmdBar.m_arrCommand.Add(IDC_FAVUSERS);
		m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_SEARCH_SPY);
		m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
		m_CmdBar.m_arrCommand.Add(IDC_SYSTEM_LOG);	
		m_CmdBar.m_arrCommand.Add(IDC_NET_STATS);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_CASCADE);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_HORZ);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_VERT);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_MINIMIZE_ALL);
		m_CmdBar.m_arrCommand.Add(ID_WINDOW_RESTORE_ALL);
	}

	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = createToolbar();

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	CreateSimpleStatusBar();

	ctrlStatus.Attach(m_hWndStatusBar);
	ctrlStatus.SetSimple(FALSE);
	int w[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	ctrlStatus.SetParts(9, w);
	statusSizes[0] = WinUtil::getTextWidth(TSTRING(AWAY), ::GetDC(ctrlStatus.m_hWnd)); // for "AWAY" segment
	CToolInfo ti(TTF_SUBCLASS, ctrlStatus.m_hWnd);

	ctrlLastLines.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);
	ctrlLastLines.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ctrlLastLines.AddTool(&ti);
	ctrlLastLines.SetDelayTime(TTDT_AUTOPOP, 30000);

	CreateMDIClient();
	m_CmdBar.SetMDIClient(m_hWndMDIClient);
	WinUtil::mdiClient = m_hWndMDIClient;

	ctrlTab.Create(m_hWnd, rcDefault);
	WinUtil::tabCtrl = &ctrlTab;

	transferView.Create(m_hWnd);

	SetSplitterPanes(m_hWndMDIClient, transferView.m_hWnd);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 8000;

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_VIEW_TRANSFER_VIEW, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	c->addListener(this);
	c->downloadFile("http://www.fuldc.net/version.xml");

	if(BOOLSETTING(OPEN_SYSTEM_LOG)) PostMessage(WM_COMMAND, IDC_SYSTEM_LOG);
	if(BOOLSETTING(OPEN_PUBLIC)) PostMessage(WM_COMMAND, ID_FILE_CONNECT);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS)) PostMessage(WM_COMMAND, IDC_FAVORITES);
	if(BOOLSETTING(OPEN_FAVORITE_USERS)) PostMessage(WM_COMMAND, IDC_FAVUSERS);
	if(BOOLSETTING(OPEN_QUEUE)) PostMessage(WM_COMMAND, IDC_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED_DL);
	if(BOOLSETTING(OPEN_WAITING_USERS)) PostMessage(WM_COMMAND, IDC_WAITING_USERS);
	if(BOOLSETTING(OPEN_FINISHED_UPLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED_UL);
	if(BOOLSETTING(OPEN_SEARCH_SPY)) PostMessage(WM_COMMAND, IDC_SEARCH_SPY);
	if(BOOLSETTING(OPEN_NETWORK_STATISTICS)) PostMessage(WM_COMMAND, IDC_NET_STATS);
	if(BOOLSETTING(OPEN_NOTEPAD)) PostMessage(WM_COMMAND, IDC_NOTEPAD);
	
	if(!BOOLSETTING(SHOW_STATUSBAR)) PostMessage(WM_COMMAND, ID_VIEW_STATUS_BAR);
	if(!BOOLSETTING(SHOW_TOOLBAR)) PostMessage(WM_COMMAND, ID_VIEW_TOOLBAR);
	if(!BOOLSETTING(SHOW_TRANSFERVIEW)) PostMessage(WM_COMMAND, ID_VIEW_TRANSFER_VIEW);

	if(!WinUtil::isShift())
		PostMessage(WM_SPEAKER, AUTO_CONNECT);

	PostMessage(WM_SPEAKER, PARSE_COMMAND_LINE);

	try {
		File::ensureDirectory(SETTING(LOG_DIRECTORY));
	} catch (const FileException) {	}

	startSocket();
	
	if(SETTING(NICK).empty()) {
		HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_GENERALPAGE);
		PostMessage(WM_COMMAND, ID_FILE_SETTINGS);
	}

	WinUtil::SetIcon(m_hWnd, _T("DCPlusPlus.ico"), true);

	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
}

void MainFrame::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if(ClientManager::getInstance()->isActive()) {
		try {
			ConnectionManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
		try {
			SearchManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}

	startUPnP();
}

void MainFrame::startUPnP() {
	stopUPnP();

	if( SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP ) {
		UPnP_TCPConnection = new UPnP( Util::getLocalIp(), "TCP", APPNAME " Download Port (" + Util::toString(ConnectionManager::getInstance()->getPort()) + " TCP)", ConnectionManager::getInstance()->getPort() );
		UPnP_UDPConnection = new UPnP( Util::getLocalIp(), "UDP", APPNAME " Search Port (" + Util::toString(SearchManager::getInstance()->getPort()) + " UDP)", SearchManager::getInstance()->getPort() );

		if ( FAILED(UPnP_UDPConnection->OpenPorts()) || FAILED(UPnP_TCPConnection->OpenPorts()) )
		{
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_CREATE_MAPPINGS));
			MessageBox(CTSTRING(UPNP_FAILED_TO_CREATE_MAPPINGS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);

			// We failed! thus reset the objects
			delete UPnP_TCPConnection;
			delete UPnP_UDPConnection;
			UPnP_TCPConnection = UPnP_UDPConnection = NULL;
		}
		else
		{
			if(!BOOLSETTING(NO_IP_OVERRIDE)) {
				// now lets configure the external IP (connect to me) address
				string ExternalIP = UPnP_TCPConnection->GetExternalIP();
				if ( !ExternalIP.empty() ) {
					// woohoo, we got the external IP from the UPnP framework
					SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, ExternalIP );
				} else {
					//:-(  Looks like we have to rely on the user setting the external IP manually
					// no need to do cleanup here because the mappings work
					LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_GET_EXTERNAL_IP));
					MessageBox(CTSTRING(UPNP_FAILED_TO_GET_EXTERNAL_IP), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);
				}
			}
		}
	}
}

void MainFrame::stopUPnP() {
	// Just check if the port mapping objects are initialized (NOT NULL)
	if ( UPnP_TCPConnection != NULL )
	{
		if (FAILED(UPnP_TCPConnection->ClosePorts()) )
		{
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_REMOVE_MAPPINGS));
		}
		delete UPnP_TCPConnection;
	}
	if ( UPnP_UDPConnection != NULL )
	{
		if (FAILED(UPnP_UDPConnection->ClosePorts()) )
		{
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_REMOVE_MAPPINGS));
		}
		delete UPnP_UDPConnection;
	}
	// Not sure this is required (i.e. Objects are checked later in execution)
	// But its better being on the save side :P
	UPnP_TCPConnection = UPnP_UDPConnection = NULL;
}

HWND MainFrame::createToolbar() {
	largeImages.CreateFromImage(WinUtil::getIconPath(_T("toolbar20.bmp")).c_str(), 0, 15, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	largeImagesHot.CreateFromImage(WinUtil::getIconPath(_T("toolbar20-highlight.bmp")).c_str(), 0, 15, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	
	ctrlToolBar.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, ATL_IDW_TOOLBAR);
	ctrlToolBar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	ctrlToolBar.SetImageList(largeImages);
	ctrlToolBar.SetHotImageList(largeImagesHot);

	const int numButtons = 24;


	TBBUTTON tb[numButtons];
	memset(tb, 0, sizeof(tb));
	int n = 0, bitmap = 0;

	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_FILE_CONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_FILE_RECONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FOLLOW;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FAVORITES;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FAVUSERS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_QUEUE;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_WAITING_USERS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FINISHED_DL;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FINISHED_UL;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_FILE_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FILE_ADL_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_SEARCH_SPY;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_OPEN_FILE_LIST;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_REFRESH_FILE_LIST;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_DROPDOWN;
		
	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_FILE_SETTINGS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_SYSTEM_LOG;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_NOTEPAD;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_CHECK;

	ctrlToolBar.SetButtonStructSize();
	ctrlToolBar.AddButtons(numButtons, tb);
	ctrlToolBar.AutoSize();

	return ctrlToolBar.m_hWnd;
}

LRESULT MainFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		
	if(wParam == DOWNLOAD_LISTING) {
		auto_ptr<DirectoryListInfo> i(reinterpret_cast<DirectoryListInfo*>(lParam));
		DirectoryListingFrame::openWindow(i->file, i->user);
	} else if(wParam == BROWSE_LISTING) {
		auto_ptr<DirectoryBrowseInfo> i(reinterpret_cast<DirectoryBrowseInfo*>(lParam));
		DirectoryListingFrame::openWindow(i->user, i->text);
	} else if(wParam == VIEW_FILE_AND_DELETE) {
		auto_ptr<tstring> file(reinterpret_cast<tstring*>(lParam));
		TextFrame::openWindow(*file);
		File::deleteFile(Text::fromT(*file));
	} else if(wParam == STATS) {
		auto_ptr<TStringList> pstr(reinterpret_cast<TStringList*>(lParam));
		const TStringList& str = *pstr;
		if(ctrlStatus.IsWindow()) {
			HDC dc = ::GetDC(ctrlStatus.m_hWnd);
			bool u = false;
			ctrlStatus.SetText(1, str[0].c_str());
			for(int i = 1; i < 8; i++) {
				int w = WinUtil::getTextWidth(str[i], dc);
				
				if(statusSizes[i] < w) {
					statusSizes[i] = w;
					u = true;
				}
				ctrlStatus.SetText(i+1, str[i].c_str());
			}
			::ReleaseDC(ctrlStatus.m_hWnd, dc);
			if(u)
				UpdateLayout(TRUE);
		}
	} else if(wParam == AUTO_CONNECT) {
		autoConnect(FavoriteManager::getInstance()->getFavoriteHubs());
	} else if(wParam == PARSE_COMMAND_LINE) {
		parseCommandLine(GetCommandLine());
	} else if(wParam == STATUS_MESSAGE) {
		auto_ptr<pair<time_t, tstring> > msg((pair<time_t, tstring>*)lParam);
		if(ctrlStatus.IsWindow()) {
			tstring line = _T("[") + Util::getShortTimeString(msg->first) + _T("] ") + msg->second;

			ctrlStatus.SetText(0, line.c_str());
			while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
				lastLinesList.erase(lastLinesList.begin());
			if (line.find(_T('\r')) == tstring::npos) {
				lastLinesList.push_back(line);
			} else {
				lastLinesList.push_back(line.substr(0, line.find(_T('\r'))));
			}
		}
	} else if(wParam == DOWNLOAD_COMPLETE) {
		PopupManager::getInstance()->ShowDownloadComplete((tstring*)lParam);
	} else if(wParam == WM_CLOSE) {
		PopupManager::getInstance()->Remove((int)lParam, true);
	} else if(wParam == REMOVE_POPUP){
		PopupManager::getInstance()->AutoRemove();
	} else if(wParam == START_TIMER) {
		auto_ptr<pair<u_int32_t, tstring> > msg(reinterpret_cast<pair<u_int32_t, tstring>* > (lParam));
		timerTime = msg->first;
		timerMsg = msg->second;
	} else if(wParam == STOP_TIMER) {
		timerTime = 0;
		timerMsg = _T("");
	}

	return 0;
}

void MainFrame::parseCommandLine(const tstring& cmdLine)
{
	string::size_type i = 0;
	string::size_type j;

	if( (j = cmdLine.find(_T("dchub://"), i)) != string::npos) {
		WinUtil::parseDchubUrl(cmdLine.substr(j));
	}
	if( (j = cmdLine.find(_T("adc://"), i)) != string::npos) {
		WinUtil::parseADChubUrl(cmdLine.substr(j));
	}
}

LRESULT MainFrame::onCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	tstring cmdLine = (LPCTSTR) (((COPYDATASTRUCT *)lParam)->lpData);
	parseCommandLine(Text::toT(Util::getAppName() + " ") + cmdLine);
	return true;
}

LRESULT MainFrame::onHashProgress(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if( !hashProgress.IsWindow() ){
		hashProgress.Create( m_hWnd );
		hashProgress.ShowWindow( SW_SHOW );
	}
	
	return 0;
}


LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	AboutDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::onOpenWindows(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID) {
		case ID_FILE_SEARCH: SearchFrame::openWindow(); break;
		case ID_FILE_CONNECT: PublicHubsFrame::openWindow(); break;
		case IDC_FAVORITES: FavoriteHubsFrame::openWindow(); break;
		case IDC_FAVUSERS: UsersFrame::openWindow(); break;
		case IDC_NOTEPAD: NotepadFrame::openWindow(); break;
		case IDC_QUEUE: QueueFrame::openWindow(); break;
		case IDC_SEARCH_SPY: SpyFrame::openWindow(); break;
		case IDC_FILE_ADL_SEARCH: ADLSearchFrame::openWindow(); break;
		case IDC_NET_STATS: StatsFrame::openWindow(); break; 
		case IDC_FINISHED_DL: FinishedDLFrame::openWindow(); break;
		case IDC_FINISHED_UL: FinishedULFrame::openWindow(); break;
		case IDC_WAITING_USERS: WaitingUsersFrame::openWindow(); break;
		case IDC_SYSTEM_LOG: SystemFrame::openWindow(); break;
		default: dcassert(0); break;
	}
	return 0;
}

LRESULT MainFrame::OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PropertiesDlg dlg(m_hWnd, SettingsManager::getInstance());

	unsigned short lastPort = (unsigned short)SETTING(TCP_PORT);
	unsigned short lastUDP = (unsigned short)SETTING(UDP_PORT);
	int lastConn = SETTING(INCOMING_CONNECTIONS);

	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		SettingsManager::getInstance()->save();
		if(missedAutoConnect && !SETTING(NICK).empty()) {
			PostMessage(WM_SPEAKER, AUTO_CONNECT);
		}
		if(SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != lastPort || SETTING(UDP_PORT) != lastUDP) {
			startSocket();
		}
		ClientManager::getInstance()->infoUpdated();

		if(BOOLSETTING(URL_HANDLER)) {
			WinUtil::registerDchubHandler();
			WinUtil::registerADChubHandler();
			WinUtil::urlDcADCRegistered = true;
		} else if(WinUtil::urlDcADCRegistered) {
			WinUtil::unRegisterDchubHandler();
			WinUtil::unRegisterADChubHandler();
			WinUtil::urlDcADCRegistered = false;
		}
	}
	return 0;
}

void MainFrame::on(HttpConnectionListener::Complete, HttpConnection* /*aConn*/, const string&) throw() {
	try {
		SimpleXML xml;
		xml.fromXML(versionInfo);
		xml.stepIn();

		if(xml.findChild("Beta") && BOOLSETTING(NOTIFY_BETA_UPDATES)) {
			xml.stepIn();
			string url = Text::fromT(links.homepage);

			if(xml.findChild("URL")) {
				url = xml.getChildData();
			}

			xml.resetCurrentChild();
			if(xml.findChild("MajorVersion")) {
				float major, minor;
				major = Util::toDouble(xml.getChildData());
				xml.resetCurrentChild();
				if(xml.findChild("MinorVersion")) {
					minor = Util::toDouble(xml.getChildData());
                    if( major > MAJORVERSIONFLOAT || ( major >= MAJORVERSIONFLOAT && minor > MINORVERSIONFLOAT )) {
                        if(xml.findChild("Title")) {
                            const string& title = xml.getChildData();
                            xml.resetCurrentChild();
                            if(xml.findChild("Message")) {
                                if(url.empty()) {
                                    const string& msg = xml.getChildData();
                                    MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_OK);
                                } else {
                                    string msg = xml.getChildData() + "\r\n" + STRING(OPEN_DOWNLOAD_PAGE);
                                    if(MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_YESNO | MB_DEFBUTTON1) == IDYES) {
                                        WinUtil::openLink(Text::toT(url));
                                    }
                                }
                            }
                        }
                    }
				}
			}
			xml.stepOut();
			xml.resetCurrentChild();
		}
		
		xml.resetCurrentChild();
		if(xml.findChild("Stable") && BOOLSETTING(NOTIFY_UPDATES)) {
			xml.stepIn();
			string url = Text::fromT(links.homepage);

			if(xml.findChild("URL")) {
				url = xml.getChildData();
			}

			xml.resetCurrentChild();
			if(xml.findChild("MajorVersion")) {
				double major, minor;
				major = Util::toDouble(xml.getChildData());
				xml.resetCurrentChild();
				if(xml.findChild("MinorVersion")) {
					minor = Util::toDouble(xml.getChildData());
                    if( major > MAJORVERSIONFLOAT || ( major >= MAJORVERSIONFLOAT && minor > MINORVERSIONFLOAT )) {
                        if(xml.findChild("Title")) {
                            const string& title = xml.getChildData();
                            xml.resetCurrentChild();
                            if(xml.findChild("Message")) {
                                if(url.empty()) {
                                    const string& msg = xml.getChildData();
                                    MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_OK);
                                } else {
                                    string msg = xml.getChildData() + "\r\n" + STRING(OPEN_DOWNLOAD_PAGE);
                                    if(MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_YESNO | MB_DEFBUTTON1) == IDYES) {
                                        WinUtil::openLink(Text::toT(url));
                                    }
                                }
                            }
                        }
                    }
				}
			}
			xml.stepOut();
			xml.resetCurrentChild();
		}
		xml.stepOut();
	} catch (const Exception&) {
		// ...
	}
}

LRESULT MainFrame::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_DISPLAY_TOC, NULL);
	bHandled = TRUE;
	return 0;	
}

LRESULT MainFrame::onMenuHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
	UINT action = (wID == IDC_HELP_CONTENTS) ? HH_DISPLAY_TOC : HH_HELP_CONTEXT;
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), action, wID);
	bHandled = TRUE;
	return 0;	
}

LRESULT MainFrame::onGetToolTip(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)pnmh;
	pDispInfo->szText[0] = 0;

	if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
	{
		int stringId = -1;
		switch(idCtrl) {
			case ID_FILE_CONNECT: stringId = ResourceManager::MENU_PUBLIC_HUBS; break;
			case ID_FILE_RECONNECT: stringId = ResourceManager::MENU_RECONNECT; break;
			case IDC_FOLLOW: stringId = ResourceManager::MENU_FOLLOW_REDIRECT; break;
			case IDC_FAVORITES: stringId = ResourceManager::MENU_FAVORITE_HUBS; break;
			case IDC_FAVUSERS: stringId = ResourceManager::MENU_FAVORITE_USERS; break;
			case IDC_QUEUE: stringId = ResourceManager::MENU_DOWNLOAD_QUEUE; break;
			case IDC_FINISHED_DL: stringId = ResourceManager::FINISHED_DOWNLOADS; break;
			case IDC_FINISHED_UL: stringId = ResourceManager::FINISHED_UPLOADS; break;
			case ID_FILE_SEARCH: stringId = ResourceManager::MENU_SEARCH; break;
			case IDC_FILE_ADL_SEARCH: stringId = ResourceManager::MENU_ADL_SEARCH; break;
			case IDC_WAITING_USERS: stringId = ResourceManager::WAITING_USERS; break;
			case IDC_SEARCH_SPY: stringId = ResourceManager::MENU_SEARCH_SPY; break;
			case IDC_OPEN_FILE_LIST: stringId = ResourceManager::MENU_OPEN_FILE_LIST; break;
			case IDC_REFRESH_FILE_LIST: stringId = ResourceManager::MENU_REFRESH_FILE_LIST; break;
			case ID_FILE_SETTINGS: stringId = ResourceManager::MENU_SETTINGS; break;
			case IDC_NET_STATS: stringId = ResourceManager::MENU_NETWORK_STATISTICS; break;
			case IDC_NOTEPAD: stringId = ResourceManager::MENU_NOTEPAD; break;
			case IDC_SYSTEM_LOG: stringId = ResourceManager::MENU_SYSTEM_LOG; break;
			default: dcassert(false); break;
		}
		if(stringId != -1) {
			_tcsncpy(pDispInfo->lpszText, CTSTRING_I((ResourceManager::Strings)stringId), 79);
			pDispInfo->uFlags |= TTF_DI_SETITEM;
		}
	} else { // if we're really in the status bar, this should be detected intelligently
		lastLines.clear();
		for(TStringIter i = lastLinesList.begin(); i != lastLinesList.end(); ++i) {
			lastLines += *i;
			lastLines += _T("\r\n");
		}
		if(lastLines.size() > 2) {
			lastLines.erase(lastLines.size() - 2);
		}
		pDispInfo->lpszText = const_cast<TCHAR*>(lastLines.c_str());
	}
	return 0;
}

void MainFrame::autoConnect(const FavoriteHubEntry::List& fl) {
	missedAutoConnect = false;
	for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		FavoriteHubEntry* entry = *i;
		if(entry->getConnect()) {
			if(!entry->getNick().empty() || !SETTING(NICK).empty())
				HubFrame::openWindow(Text::toT(entry->getServer()));
			else
				missedAutoConnect = true;
		}
	}
}

void MainFrame::updateTray(bool add /* = true */) {
	if(add) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		nid.uCallbackMessage = WM_APP + 242;
		nid.hIcon = (HICON)::LoadImage(NULL, WinUtil::getIconPath(_T("dcplusplus.ico")).c_str(), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
		_tcscpy(nid.szTip, _T("DC++"));
		nid.szTip[63] = '\0';
		lastMove = GET_TICK() - 1000;
		::Shell_NotifyIcon(NIM_ADD, &nid);
		trayIcon = true;
	} else {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = 0;
		::Shell_NotifyIcon(NIM_DELETE, &nid);
		ShowWindow(SW_SHOW);
		trayIcon = false;		
	}
}

/**
 * @todo Fix so that the away mode is not reset if it was set manually...
 */
LRESULT MainFrame::onSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if(wParam == SIZE_MINIMIZED) {
		if(BOOLSETTING(AUTO_AWAY) && !Util::getManualAway()) {
			Util::setAway(true);
		}
		if(BOOLSETTING(MINIMIZE_TRAY) != WinUtil::isShift()) {
			updateTray(true);
			ShowWindow(SW_HIDE);
			PopupManager::getInstance()->Minimized(true);
		}
		maximized = IsZoomed() > 0;

	} else if( (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) ) {
		if(BOOLSETTING(AUTO_AWAY) && !Util::getManualAway()) {
			Util::setAway(false);
		}
		if(trayIcon) {
			updateTray(false);
		}

		PopupManager::getInstance()->Minimized(false);
	}
	
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::onEndSession(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(c != NULL) {
		//c->removeListener(this);
		delete c;
		c = NULL;
	}

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	
	CRect rc;
	GetWindowRect(rc);
	
	if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
	}
	if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);
	
	QueueManager::getInstance()->saveQueue();
	SettingsManager::getInstance()->save();
	
	return 0;
}

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	
	if(c != NULL) {
		//c->removeListener(this);
		delete c;
		c = NULL;
	}
	
	if(!closing) {
		if( oldshutdown ||(!BOOLSETTING(CONFIRM_EXIT)) || (MessageBox(CTSTRING(REALLY_EXIT), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) ) {
			string tmp1;
			string tmp2;

			HubFrame::setClosing();

			if( hashProgress.IsWindow() )
				hashProgress.DestroyWindow();

			WINDOWPLACEMENT wp;
			wp.length = sizeof(wp);
			GetWindowPlacement(&wp);

			CRect rc;
			GetWindowRect(rc);
			if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
			}
			if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);

			ShowWindow(SW_HIDE);
			transferView.prepareClose();

			SearchManager::getInstance()->disconnect();
			ConnectionManager::getInstance()->disconnect();

			stopUPnP();

			DWORD id;
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
			closing = true;
		}
		bHandled = TRUE;
	} else {
		// This should end immediately, as it only should be the stopper that sends another WM_CLOSE
		WaitForSingleObject(stopperThread, 60*1000);
		CloseHandle(stopperThread);
		stopperThread = NULL;
		bHandled = FALSE;
	}

	return 0;
}

LRESULT MainFrame::onLink(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	tstring site;
	switch(wID) {
	case IDC_HELP_HOMEPAGE: site = links.homepage; break;
	case IDC_HELP_DOWNLOADS: site = links.downloads; break;
	//case IDC_HELP_GEOIPFILE: site = links.geoipfile; break;
	case IDC_HELP_TRANSLATIONS: site = links.translations; break;
	case IDC_HELP_FAQ: site = links.faq; break;
	case IDC_HELP_HELP_FORUM: site = links.help; break;
	case IDC_HELP_DISCUSS: site = links.discuss; break;
	case IDC_HELP_REQUEST_FEATURE: site = links.features; break;
	case IDC_HELP_REPORT_BUG: site = links.bugs; break;
	case IDC_HELP_DONATE: site = Text::toT("https://www.paypal.com/xclick/business=arnetheduck%40gmail.com&item_name=DCPlusPlus&no_shipping=1&return=http%3A//dcplusplus.sf.net&cn=Greeting+%28and+forum+nick%3F%29&currency_code=EUR"); break;
	case IDC_HELP_FULPAGE: site = _T("http://www.fuldc.net"); break;
	default: dcassert(0);
	}

	WinUtil::openLink(site);

	return 0;
}

void MainFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */)
{
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow() && ctrlLastLines.IsWindow()) {
		CRect sr;
		int w[9];
		ctrlStatus.GetClientRect(sr);
		w[8] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(7); setw(6); setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(9, w);
		ctrlLastLines.SetMaxTipWidth(w[0]);
	}
	CRect rc = rect;
	rc.top = rc.bottom - ctrlTab.getHeight();
	if(ctrlTab.IsWindow())
		ctrlTab.MoveWindow(rc);
	
	CRect rc2 = rect;
	rc2.bottom = rc.top;
	SetSplitterRect(rc2);
}

static const TCHAR types[] = _T("File Lists\0*.DcLst;*.xml.bz2\0All Files\0*.*\0");

LRESULT MainFrame::onOpenFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring file;
	if(WinUtil::browseFile(file, m_hWnd, false, Text::toT(Util::getAppPath() + "FileLists\\"), types)) {
		tstring username;
		if(file.rfind('\\') != string::npos) {
			username = file.substr(file.rfind('\\') + 1);
			if(username.rfind('.') != string::npos) {
				username.erase(username.rfind('.'));
			}
			if(username.length() > 4 && Util::stricmp(username.c_str() + username.length() - 4, _T(".xml")) == 0)
				username.erase(username.length()-4);
			DirectoryListingFrame::openWindow(file, ClientManager::getInstance()->getUser(Text::fromT(username)));
		}
	}
	return 0;
}

LRESULT MainFrame::onOpenOwnList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ShareManager::getInstance()->generateXmlList(true);
	if(!ShareManager::getInstance()->getOwnListFile().empty()){
		DirectoryListingFrame::openWindow(Text::toT(ShareManager::getInstance()->getOwnListFile()), ClientManager::getInstance()->getUser(SETTING(NICK)));
	}
	return 0;
}

LRESULT MainFrame::onRefreshFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true);
	return 0;
}

LRESULT MainFrame::onTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (lParam == WM_LBUTTONUP) {
		ShowWindow(SW_SHOW);
		ShowWindow(maximized ? SW_MAXIMIZE : SW_RESTORE);
	}else if(lParam == WM_RBUTTONDOWN || lParam == WM_CONTEXTMENU){ 		
		CPoint pt;
		CMenu mnuTrayMenu;
		mnuTrayMenu.CreatePopupMenu();
		mnuTrayMenu.AppendMenu(MF_STRING, IDC_TRAY_SHOW, CTSTRING(MENU_SHOW));
		mnuTrayMenu.AppendMenu(MF_STRING, IDC_TRAY_QUIT, CTSTRING(MENU_EXIT));
		mnuTrayMenu.AppendMenu(MF_STRING, IDC_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR));
		mnuTrayMenu.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CTSTRING(MENU_SETTINGS));
		GetCursorPos(&pt);
		SetForegroundWindow(m_hWnd); 
		mnuTrayMenu.TrackPopupMenu(TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,pt.x,pt.y,m_hWnd);
		PostMessage(WM_NULL, 0, 0);
		mnuTrayMenu.SetMenuDefaultItem(0,TRUE);
	} else if(lParam == WM_MOUSEMOVE && ((lastMove + 1000) < GET_TICK()) ) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_TIP;
		_tcsncpy(nid.szTip, Text::toT("D: " + Util::formatBytes(DownloadManager::getInstance()->getAverageSpeed()) + "/s (" + 
			Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")\r\nU: " +
			Util::formatBytes(UploadManager::getInstance()->getAverageSpeed()) + "/s (" + 
			Util::toString(UploadManager::getInstance()->getUploadCount()) + ")").c_str(), 64);
		
		::Shell_NotifyIcon(NIM_MODIFY, &nid);
		lastMove = GET_TICK();
	}
	return 0;
}

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_TOOLBAR, bVisible);
	return 0;
}

LRESULT MainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_STATUSBAR, bVisible);
	return 0;
}

LRESULT MainFrame::OnViewTransferView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !transferView.IsWindowVisible();
	if(!bVisible) {	
		if(GetSinglePaneMode() == SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_TOP);
	} else { 
		if(GetSinglePaneMode() != SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	UISetCheck(ID_VIEW_TRANSFER_VIEW, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_TRANSFERVIEW, bVisible);
	return 0;
}

LRESULT MainFrame::onCloseWindows(WORD , WORD wID, HWND , BOOL& ) {
	switch(wID) {
	case IDC_CLOSE_DISCONNECTED:		HubFrame::closeDisconnected();		break;
	case IDC_CLOSE_ALL_PM:				PrivateFrame::closeAll();			break;
	case IDC_CLOSE_ALL_OFFLINE_PM:		PrivateFrame::closeAllOffline();	break;
	case IDC_CLOSE_ALL_DIR_LIST:		DirectoryListingFrame::closeAll();	break;
	case IDC_CLOSE_ALL_SEARCH_FRAME:	SearchFrame::closeAll();			break;
	}
	return 0;
}

LRESULT MainFrame::onSwitchWindow(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlTab.SwitchWindow(wID - IDC_SWITCH_WINDOW_1);
	return 0;
}

LRESULT MainFrame::onQuickConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	LineDlg dlg;
	dlg.description = TSTRING(HUB_ADDRESS);
	dlg.title = TSTRING(QUICK_CONNECT);
	if(dlg.DoModal(m_hWnd) == IDOK){
		if(SETTING(NICK).empty())
			return 0;

		tstring tmp = dlg.line;
		// Strip out all the spaces
		string::size_type i;
		while((i = tmp.find(' ')) != string::npos)
			tmp.erase(i, 1);

		HubFrame::openWindow(tmp);
	}
	return 0;
}

void MainFrame::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	int64_t diff = (int64_t)((lastUpdate == 0) ? aTick - 1000 : aTick - lastUpdate);
	int64_t updiff = Socket::getTotalUp() - lastUp;
	int64_t downdiff = Socket::getTotalDown() - lastDown;

	TStringList* str = new TStringList();
	str->push_back(Util::getAway() ? TSTRING(AWAY) : _T(""));
	str->push_back(TSTRING(STATUS_QUEUE) + Util::formatBytesW(QueueManager::getInstance()->getQueueSize()));
	str->push_back(Text::toT("H: " + Client::getCounts()));
	str->push_back(Text::toT(STRING(SLOTS) + ": " + Util::toString(SETTING(SLOTS) - static_cast<int>(UploadManager::getInstance()->getUploadCount())) + '/' + Util::toString(SETTING(SLOTS))));
	str->push_back(Text::toT("D: " + Util::formatBytes(Socket::getTotalDown())));
	str->push_back(Text::toT("U: " + Util::formatBytes(Socket::getTotalUp())));
	str->push_back(Text::toT("D: " + Util::formatBytes(downdiff*1000I64/diff) + "/s (" + Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")"));
	str->push_back(Text::toT("U: " + Util::formatBytes(updiff*1000I64/diff) + "/s (" + Util::toString(UploadManager::getInstance()->getUploadCount()) + ")"));
	PostMessage(WM_SPEAKER, STATS, (LPARAM)str);
	SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + updiff);
	SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downdiff);
	lastUpdate = aTick;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	if(timerTime > 0) {
		if(aTick > timerTime) {
			WinUtil::flashWindow();
			::MessageBox(WinUtil::mainWnd, timerMsg.c_str(), CTSTRING(TIMER), MB_OK);
			timerTime = 0;
			timerMsg = _T("");
		}
	}
}

void MainFrame::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const u_int8_t* buf, size_t len) throw() {
	versionInfo += string((const char*)buf, len);
}

void MainFrame::on(PartialList, const User::Ptr& aUser, const string& text) throw() {
	PostMessage(WM_SPEAKER, BROWSE_LISTING, (LPARAM)new DirectoryBrowseInfo(aUser, text));
}

void MainFrame::on(QueueManagerListener::Finished, QueueItem* qi, int64_t) throw() {
	if(qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
		if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
			// This is a file listing, show it...

			DirectoryListInfo* i = new DirectoryListInfo();
			i->file = Text::toT(qi->getListName());
			i->user = qi->getCurrent()->getUser();

			PostMessage(WM_SPEAKER, DOWNLOAD_LISTING, (LPARAM)i);
		} else if(qi->isSet(QueueItem::FLAG_TEXT)) {
			PostMessage(WM_SPEAKER, VIEW_FILE_AND_DELETE, (LPARAM) new tstring(Text::toT(qi->getTarget())));
		}
	}
}


LRESULT MainFrame::onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	LogManager::getInstance()->removeListener(this);
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);

	if(trayIcon) {
		updateTray(false);
	}
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::onDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMTOOLBAR tb = (LPNMTOOLBAR)pnmh;
	CMenu dropMenu;
	dropMenu.CreatePopupMenu();

	//Set the MNS_NOTIFYBYPOS flag to receive WM_MENUCOMMAND
	MENUINFO inf;
	inf.cbSize = sizeof(MENUINFO);
	inf.fMask = MIM_STYLE;
	inf.dwStyle = MNS_NOTIFYBYPOS;
	dropMenu.SetMenuInfo(&inf);

	StringList l = ShareManager::getInstance()->getVirtualDirectories();
	
	dropMenu.AppendMenu(MF_STRING, IDC_REFRESH_MENU, CTSTRING(SETTINGS_ST_REFRESH_INCOMING));
	dropMenu.AppendMenu(MF_SEPARATOR);
	int j = 1;
	for(StringIter i = l.begin(); i != l.end(); ++i, ++j)
		dropMenu.AppendMenu(MF_STRING, IDC_REFRESH_MENU, Text::toT( *i ).c_str());
	
	POINT pt;
	pt.x = tb->rcButton.right;
	pt.y = tb->rcButton.bottom;
	ClientToScreen(&pt);
	dropMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, m_hWnd);

	return TBDDRET_DEFAULT;
}

LRESULT MainFrame::onRefreshMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	try
	{
		ShareManager::getInstance()->setDirty();
		if(wParam == 0){
			ShareManager::getInstance()->refresh( false, true, false, true );
		} else if(wParam > 1){
			int id = wParam - 2;
			StringList l = ShareManager::getInstance()->getVirtualDirectories();
			ShareManager::getInstance()->refresh( l[id] );
		}
	} catch(ShareException) {
		//...
	}

	return 0;
}

LRESULT MainFrame::onAppCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled){
	bHandled = TRUE;
	int ret = TRUE;
	HWND wnd;

	switch(GET_APPCOMMAND_LPARAM(lParam)){
		case APPCOMMAND_BROWSER_FORWARD:
			ctrlTab.SwitchTo();
			break;
		case APPCOMMAND_BROWSER_BACKWARD:
			ctrlTab.SwitchTo(false);
			break;
		case APPCOMMAND_BROWSER_SEARCH:
			::PostMessage(m_hWnd, WM_COMMAND, ID_FILE_SEARCH, 0);
			break;
		case APPCOMMAND_FIND:
			wnd = (HWND)::SendMessage(m_hWnd, WM_MDIGETACTIVE, 0, 0);
			::PostMessage(wnd, WM_COMMAND, IDC_FIND, 0);
			break;
		case APPCOMMAND_CLOSE:
			wnd = (HWND)::SendMessage(m_hWnd, WM_MDIGETACTIVE, 0, 0);
			::PostMessage(wnd, WM_CLOSE, 0, 0);
		default:
			bHandled = FALSE;
			ret = FALSE;
			break;
	}

	return ret;
}

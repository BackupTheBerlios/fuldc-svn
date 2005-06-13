/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "FinishedFrame.h"
#include "ADLSearchFrame.h"
#include "FinishedULFrame.h"
#include "TextFrame.h"
#include "StatsFrame.h"
#include "LineDlg.h"
#include "HashProgressDlg.h"
#include "UPnP.h"
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
closing(false), missedAutoConnect(false), hashProgress(false)
{ 
	memset(statusSizes, 0, sizeof(statusSizes));
	
	links.homepage = _T("http://dcplusplus.sourceforge.net/");
	links.downloads = links.homepage + _T("download/");
	//links.geoipfile = _T("http://www.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip");
	links.translations = _T("http://sourceforge.net/tracker/?atid=460289&group_id=40287");
	links.faq = links.homepage + _T("faq/faq.php?list=all&prog=1&lang=en");
	links.help = links.homepage + _T("forum/");
	links.discuss = links.homepage + _T("forum/");
	links.features = links.homepage + _T("bugs/");
	links.bugs = links.homepage + _T("bugs/");

};

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

	shutdown();
	
	mf->PostMessage(WM_CLOSE);	
	return 0;
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	TimerManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);

	WinUtil::init(m_hWnd);

	// Register server socket message
	WSAAsyncSelect(ConnectionManager::getInstance()->getServerSocket().getSocket(),
		m_hWnd, SERVER_SOCKET_MESSAGE, FD_ACCEPT);

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
	images.CreateFromImage(_T("icons\\toolbar.bmp"), 16, 30, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	m_CmdBar.m_hImageList = images;
	if(images.GetImageCount() > 15) {
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
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED);
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL);
		m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
		m_CmdBar.m_arrCommand.Add(IDC_FAVUSERS);
		m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_SEARCH_SPY);
		m_CmdBar.m_arrCommand.Add(IDC_NET_STATS);
		m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
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
		m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_DIR_LIST);
		m_CmdBar.m_arrCommand.Add(IDC_CLOSE_ALL_SEARCH_FRAME);
	} else if(images.GetImageCount() == 15) {
		m_CmdBar.m_arrCommand.Add(ID_FILE_CONNECT);
		m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
		m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
		m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
		m_CmdBar.m_arrCommand.Add(IDC_FAVUSERS);
		m_CmdBar.m_arrCommand.Add(IDC_QUEUE);
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED);
		m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL);
		m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
		m_CmdBar.m_arrCommand.Add(IDC_SEARCH_SPY);
		m_CmdBar.m_arrCommand.Add(IDC_OPEN_FILE_LIST);
		m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);
		m_CmdBar.m_arrCommand.Add(IDC_NET_STATS);
		m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
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
	int w[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	ctrlStatus.SetParts(8, w);
	statusSizes[0] = WinUtil::getTextWidth(TSTRING(AWAY), ::GetDC(ctrlStatus.m_hWnd)); // for "AWAY" segment
	CToolInfo ti(TTF_SUBCLASS, ctrlStatus.m_hWnd);

	ctrlLastLines.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);
	ctrlLastLines.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ctrlLastLines.AddTool(&ti);

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

	//c->addListener(this);
	//c->downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	if(BOOLSETTING(OPEN_PUBLIC)) PostMessage(WM_COMMAND, ID_FILE_CONNECT);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS)) PostMessage(WM_COMMAND, IDC_FAVORITES);
	if(BOOLSETTING(OPEN_FAVORITE_USERS)) PostMessage(WM_COMMAND, IDC_FAVUSERS);
	if(BOOLSETTING(OPEN_QUEUE)) PostMessage(WM_COMMAND, IDC_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED);
	if(BOOLSETTING(OPEN_FINISHED_UPLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED_UL);
	if(BOOLSETTING(OPEN_SEARCH_SPY)) PostMessage(WM_COMMAND, IDC_SEARCH_SPY);
	if(BOOLSETTING(OPEN_NETWORK_STATISTICS)) PostMessage(WM_COMMAND, IDC_NET_STATS);
	if(BOOLSETTING(OPEN_NOTEPAD)) PostMessage(WM_COMMAND, IDC_NOTEPAD);
	
	if(!BOOLSETTING(SHOW_STATUSBAR)) PostMessage(WM_COMMAND, ID_VIEW_STATUS_BAR);
	if(!BOOLSETTING(SHOW_TOOLBAR)) PostMessage(WM_COMMAND, ID_VIEW_TOOLBAR);
	if(!BOOLSETTING(SHOW_TRANSFERVIEW)) PostMessage(WM_COMMAND, ID_VIEW_TRANSFER_VIEW);

	if(!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
		PostMessage(WM_SPEAKER, AUTO_CONNECT);

	PostMessage(WM_SPEAKER, PARSE_COMMAND_LINE);

	File::ensureDirectory(SETTING(LOG_DIRECTORY));

	startSocket();
	// we should have decided what ports we are using by now
	// so if we are using UPnP lets open the ports.
	
	// Initialize (no more need for all these elses)
	UPnP_TCPConnection = UPnP_UDPConnection = NULL;
	
	if( BOOLSETTING( SETTINGS_USE_UPNP ) )
	{
		 if (
			 ( WinUtil::getOsMajor() >= 5 && WinUtil::getOsMinor() >= 1 ) //WinXP & WinSvr2003
			 || WinUtil::getOsMajor() >= 6 )  //Longhorn
		 {
			UPnP_TCPConnection = new UPnP( Util::getLocalIp(), "TCP", APPNAME " Download Port (" + Util::toString(ConnectionManager::getInstance()->getPort()) + " TCP)", ConnectionManager::getInstance()->getPort() );
			UPnP_UDPConnection = new UPnP( Util::getLocalIp(), "UDP", APPNAME " Search Port (" + Util::toString(SearchManager::getInstance()->getPort()) + " UDP)", SearchManager::getInstance()->getPort() );
		
			if ( FAILED(UPnP_UDPConnection->OpenPorts()) || FAILED(UPnP_TCPConnection->OpenPorts()) )
			{
				LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_CREATE_MAPPINGS));
				MessageBox(CTSTRING(UPNP_FAILED_TO_CREATE_MAPPINGS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);
				
				// fall-back to passive mode
				SettingsManager::getInstance()->set(SettingsManager::CONNECTION_TYPE, SettingsManager::CONNECTION_PASSIVE);

				// We failed! thus reset the objects
				delete UPnP_TCPConnection;
				delete UPnP_UDPConnection;
				UPnP_TCPConnection = UPnP_UDPConnection = NULL;
			}
			else
			{
				// now lets configure the external IP (connect to me) address
				string ExternalIP = UPnP_TCPConnection->GetExternalIP();
				if ( ExternalIP.empty() == false )
				{
					// woohoo, we got the external IP from the UPnP framework
					SettingsManager::getInstance()->set(SettingsManager::SERVER, ExternalIP );
					SettingsManager::getInstance()->set(SettingsManager::CONNECTION_TYPE, SettingsManager::CONNECTION_ACTIVE);
				}
				else
				{
					//:-(  Looks like we have to rely on the user setting the external IP manually
					// no need to do cleanup here because the mappings work
					LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_GET_EXTERNAL_IP));
					MessageBox(CTSTRING(UPNP_FAILED_TO_GET_EXTERNAL_IP), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);
				}
			}
		}
		else
		{
			LogManager::getInstance()->message(STRING(OPERATING_SYSTEM_NOT_COMPATIBLE));
		}
	}

	if(SETTING(NICK).empty()) {
		PostMessage(WM_COMMAND, IDC_HELP_README);
		PostMessage(WM_COMMAND, ID_FILE_SETTINGS);
	}

	WinUtil::SetIcon(m_hWnd, _T("DCPlusPlus.ico"), true);

	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
}
/**
 * @todo Fix this, it's dead ugly...
 */
void MainFrame::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {

		short lastPort = (short)SETTING(IN_PORT);
		short firstPort = lastPort;

		while(true) {
			try {
				ConnectionManager::getInstance()->setPort(lastPort);
				WSAAsyncSelect(ConnectionManager::getInstance()->getServerSocket().getSocket(), m_hWnd, SERVER_SOCKET_MESSAGE, FD_ACCEPT);
				break;
			} catch(const Exception& e) {
				dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
				short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
				SettingsManager::getInstance()->setDefault(SettingsManager::IN_PORT, newPort);
				if(SETTING(IN_PORT) == lastPort || (firstPort == newPort)) {
					// Changing default didn't change port, a fixed port must be in use...(or we
					// tried all ports
					AutoArray<TCHAR> buf(STRING(PORT_IS_BUSY).size() + 8);
					_stprintf(buf, CTSTRING(PORT_IS_BUSY), SETTING(IN_PORT));
					MessageBox(buf, _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_ICONSTOP | MB_OK);
					break;
				}
				lastPort = newPort;
			}
		}

		lastPort = (short)SETTING(UDP_PORT);
		firstPort = lastPort;

		while(true) {
			try {
				SearchManager::getInstance()->setPort(lastPort);
				break;
			} catch(const Exception& e) {
				dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
				short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
				SettingsManager::getInstance()->setDefault(SettingsManager::UDP_PORT, newPort);
				if(SETTING(UDP_PORT) == lastPort || (firstPort == newPort)) {
					// Changing default didn't change port, a fixed port must be in use...(or we
					// tried all ports
					AutoArray<TCHAR> buf(STRING(PORT_IS_BUSY).size() + 8);
					_stprintf(buf, CTSTRING(PORT_IS_BUSY), SETTING(UDP_PORT));
					MessageBox(buf, _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
					break;
				}
				lastPort = newPort;
			}
		}

	}
}

HWND MainFrame::createToolbar() {
	largeImages.CreateFromImage(_T("icons\\toolbar20.bmp"), 0, 15, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	largeImagesHot.CreateFromImage(_T("icons\\toolbar20-highlight.bmp"), 0, 15, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	
	ctrlToolBar.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, ATL_IDW_TOOLBAR);
	ctrlToolBar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	ctrlToolBar.SetImageList(largeImages);
	ctrlToolBar.SetHotImageList(largeImagesHot);

	const int numButtons = 22;


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
	tb[n].idCommand = IDC_FINISHED;
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
		DirectoryListInfo* i = (DirectoryListInfo*)lParam;
		DirectoryListingFrame::openWindow(i->file, i->user);
		delete i;
	} else if(wParam == BROWSE_LISTING) {
		DirectoryBrowseInfo* i = (DirectoryBrowseInfo*)lParam;
		DirectoryListingFrame::openWindow(i->user, i->text);
		delete i;
	} else if(wParam == VIEW_FILE_AND_DELETE) {
		tstring* file = (tstring*)lParam;
		TextFrame::openWindow(*file);
		File::deleteFile(Text::fromT(*file));
		delete file;
	} else if(wParam == STATS) {
		TStringList& str = *(TStringList*)lParam;
		if(ctrlStatus.IsWindow()) {
			HDC dc = ::GetDC(ctrlStatus.m_hWnd);
			bool u = false;
			ctrlStatus.SetText(1, str[0].c_str());
			for(int i = 1; i < 7; i++) {
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
		delete &str;
	} else if(wParam == AUTO_CONNECT) {
		autoConnect(HubManager::getInstance()->getFavoriteHubs());
	} else if(wParam == PARSE_COMMAND_LINE) {
		parseCommandLine(GetCommandLine());
	} else if(wParam == STATUS_MESSAGE) {
		tstring* msg = (tstring*)lParam;
		if(ctrlStatus.IsWindow()) {
			tstring line = _T("[") + Util::getShortTimeString() + _T("] ") + *msg;

			ctrlStatus.SetText(0, line.c_str());
			while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
				lastLinesList.erase(lastLinesList.begin());
			if (line.find(_T('\r')) == tstring::npos) {
				lastLinesList.push_back(line);
			} else {
				lastLinesList.push_back(line.substr(0, line.find(_T('\r'))));
			}
		}
		delete msg;
	} else if(wParam == DOWNLOAD_COMPLETE) {
		PopupManager::getInstance()->ShowDownloadComplete((tstring*)lParam);
	} else if(wParam == WM_CLOSE) {
		PopupManager::getInstance()->Remove((int)lParam, true);
	} else if(wParam == REMOVE_POPUP){
		PopupManager::getInstance()->AutoRemove();
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

LRESULT MainFrame::onStaticFrame(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID){
		case ID_FILE_CONNECT:		PublicHubsFrame::openWindow(); break;
		case IDC_FAVORITES:			FavoriteHubsFrame::openWindow(); break;
		case IDC_FAVUSERS:			UsersFrame::openWindow(); break;
		case IDC_QUEUE:				QueueFrame::openWindow(); break;
		case IDC_FINISHED:			FinishedFrame::openWindow(); break;
		case IDC_FINISHED_UL:		FinishedULFrame::openWindow(); break;
		case ID_FILE_SEARCH:		SearchFrame::openWindow(); break;
		case IDC_FILE_ADL_SEARCH:	ADLSearchFrame::openWindow(); break;
		case IDC_SEARCH_SPY:		SpyFrame::openWindow(); break;
		case IDC_NOTEPAD:			NotepadFrame::openWindow(); break;
		case IDC_NET_STATS:			StatsFrame::openWindow(); break;
	}
	return 0;
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

LRESULT MainFrame::OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PropertiesDlg dlg(SettingsManager::getInstance());

	short lastPort = (short)SETTING(IN_PORT);
	short lastUDP = (short)SETTING(UDP_PORT);
	int lastConn = SETTING(CONNECTION_TYPE);

	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		SettingsManager::getInstance()->save();
		if(missedAutoConnect && !SETTING(NICK).empty()) {
			PostMessage(WM_SPEAKER, AUTO_CONNECT);
		}
		if(SETTING(CONNECTION_TYPE) != lastConn || SETTING(IN_PORT) != lastPort || SETTING(UDP_PORT) != lastUDP) {
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

		string url = Text::fromT(links.homepage);

		if(xml.findChild("URL")) {
			url = xml.getChildData();
		}

		xml.resetCurrentChild();
		if(xml.findChild("Version")) {
			if(atof(xml.getChildData().c_str()) > VERSIONFLOAT) {
				xml.resetCurrentChild();
				xml.resetCurrentChild();
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
			} else {
				xml.resetCurrentChild();
				if(xml.findChild("VeryOldVersion")) {
					if(atof(xml.getChildData().c_str()) >= VERSIONFLOAT) {
						string msg = xml.getChildAttrib("Message", "Your version of DC++ contains a serious bug that affects all users of the DC network or the security of your computer.");
						MessageBox(Text::toT(msg + "\r\nPlease get a new one at " + url).c_str());
						oldshutdown = true;
						PostMessage(WM_CLOSE);
					}
				}
			}

			xml.resetCurrentChild();
			if(xml.findChild("BadVersions")) {
				xml.stepIn();
				while(xml.findChild("BadVersion")) {
					double v = atof(xml.getChildAttrib("Version").c_str());
					if(v == VERSIONFLOAT) {
						string msg = xml.getChildAttrib("Message", "Your version of DC++ contains a serious bug that affects all users of the DC network or the security of your computer.");
						MessageBox(Text::toT(msg + "\r\nPlease get a new one at " + url).c_str(), _T("Bad DC++ version"), MB_OK | MB_ICONEXCLAMATION);
						oldshutdown = true;
						PostMessage(WM_CLOSE);
					}
				}
				xml.stepOut();
			}

			xml.resetCurrentChild();
			if(xml.findChild("Links")) {
				xml.stepIn();
				if(xml.findChild("Homepage")) {
					links.homepage = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Downloads")) {
					links.downloads = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("GeoIP database update")) {
					links.geoipfile = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Translations")) {
					links.translations = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Faq")) {
					links.faq = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Bugs")) {
					links.bugs = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Features")) {
					links.features = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Help")) {
					links.help = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Forum")) {
					links.discuss = Text::toT(xml.getChildData());
				}
				xml.stepOut();
			}
		}
		xml.stepOut();
	} catch (const Exception&) {
		// ...
	}
}

LRESULT MainFrame::onServerSocket(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ConnectionManager::getInstance()->getServerSocket().incoming();
	return 0;
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
			case IDC_FINISHED: stringId = ResourceManager::FINISHED_DOWNLOADS; break;
			case IDC_FINISHED_UL: stringId = ResourceManager::FINISHED_UPLOADS; break;
			case ID_FILE_SEARCH: stringId = ResourceManager::MENU_SEARCH; break;
			case IDC_FILE_ADL_SEARCH: stringId = ResourceManager::MENU_ADL_SEARCH; break;
			case IDC_SEARCH_SPY: stringId = ResourceManager::MENU_SEARCH_SPY; break;
			case IDC_OPEN_FILE_LIST: stringId = ResourceManager::MENU_OPEN_FILE_LIST; break;
			case IDC_REFRESH_FILE_LIST: stringId = ResourceManager::MENU_REFRESH_FILE_LIST; break;
			case ID_FILE_SETTINGS: stringId = ResourceManager::MENU_SETTINGS; break;
			case IDC_NET_STATS: stringId = ResourceManager::MENU_NETWORK_STATISTICS; break;
			case IDC_NOTEPAD: stringId = ResourceManager::MENU_NOTEPAD; break;
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
		nid.hIcon = (HICON)::LoadImage(NULL, _T("icons\\dcplusplus.ico"), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
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
		if(BOOLSETTING(AUTO_AWAY)) {
			Util::setAway(true);
		}
		if(BOOLSETTING(MINIMIZE_TRAY)) {
			updateTray(true);
			ShowWindow(SW_HIDE);
			PopupManager::getInstance()->Minimized(true);
		}
		maximized = IsZoomed() > 0;

	} else if( (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) ) {
		if(BOOLSETTING(AUTO_AWAY)) {
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
	bool isFile = false;
	switch(wID) {
	case IDC_HELP_README: site = Text::toT(Util::getAppPath() + "README.txt"); isFile = true; break;
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
	case IDC_HELP_FULPAGE: site = _T("http://ful.dcportal.net"); break;
	default: dcassert(0);
	}

	if(isFile)
		WinUtil::openFile(site);
	else
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
		int w[8];
		ctrlStatus.GetClientRect(sr);
		w[7] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(6); setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(8, w);
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
	case IDC_CLOSE_DISCONNECTED:		HubFrame::closeDisconnected(); break;
	case IDC_CLOSE_ALL_PM:				PrivateFrame::closeAll(); break;
	case IDC_CLOSE_ALL_DIR_LIST:		DirectoryListingFrame::closeAll(); break;
	case IDC_CLOSE_ALL_SEARCH_FRAME:	SearchFrame::closeAll(); break;
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
}

void MainFrame::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const u_int8_t* buf, size_t len) throw() {
	versionInfo += string((const char*)buf, len);
}

void MainFrame::on(PartialList, const User::Ptr& aUser, const string& text) throw() {
	PostMessage(WM_SPEAKER, BROWSE_LISTING, (LPARAM)new DirectoryBrowseInfo(aUser, text));
}

void MainFrame::on(QueueManagerListener::Finished, QueueItem* qi) throw() {
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
/**
 * @file
 * $Id: MainFrm.cpp,v 1.13 2004/02/22 01:24:11 trem Exp $
 */
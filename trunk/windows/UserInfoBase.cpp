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
#include "UserInfoBase.h"
#include "PrivateFrame.h"

#include "../client/FavoriteManager.h"
#include "../client/QueueManager.h"
#include "../client/UploadManager.h"
#include "../client/User.h"
#include "../client/IgnoreManager.h"
#include "../client/LogManager.h"


void UserInfoBase::matchQueue() {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());
	}
}
void UserInfoBase::getList() {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());
	}
}
void UserInfoBase::browseList() {
	if(user->getCID().isZero())
		return;
	try {
		QueueManager::getInstance()->addPfs(user, "");
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());
	}
}

void UserInfoBase::addFav() {
	FavoriteManager::getInstance()->addFavoriteUser(user);
}
void UserInfoBase::pm() {
	PrivateFrame::openWindow(user);
}

void UserInfoBase::pm(const tstring& aMsg) {
	PrivateFrame::openWindow(user, aMsg);
}
void UserInfoBase::grant() {
	UploadManager::getInstance()->reserveSlot(user);
}
void UserInfoBase::removeAll() {
	QueueManager::getInstance()->removeUserFromQueue(user, QueueItem::Source::FLAG_REMOVED);
}

void UserInfoBase::ignore() {
	IgnoreManager::getInstance()->ignore(Text::fromT(WinUtil::getNicks(user)));
}
void UserInfoBase::unignore() {
	IgnoreManager::getInstance()->unignore(Text::fromT(WinUtil::getNicks(user)));
}

void UserInfoBase::showLog() {
	StringMap params;
	params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(user->getCID()));
	params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(user->getCID()));
	params["userCID"] = user->getCID().toBase32(); 
	params["userNI"] = user->getFirstNick();
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();

	tstring path = Text::toT(LogManager::getInstance()->getLogFilename(LogManager::PM, params));
	if(!path.empty())
		ShellExecute(NULL, _T("open"), path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

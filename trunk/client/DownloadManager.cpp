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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "DownloadManager.h"

#include "ResourceManager.h"
#include "ConnectionManager.h"
#include "QueueManager.h"
#include "CryptoManager.h"
#include "HashManager.h"

#include "LogManager.h"
#include "SFVReader.h"
#include "User.h"
#include "File.h"
#include "FilteredFile.h"
#include "MerkleCheckOutputStream.h"
#include "ClientManager.h"

#include <limits>

// some strange mac definition
#ifdef ff
#undef ff
#endif

const string DownloadManager::USER_LIST_NAME = "files.xml";
const string DownloadManager::USER_LIST_NAME_BZ = "files.xml.bz2";

Download::Download() throw() : file(NULL),
crcCalc(NULL), treeValid(false) {
}

Download::Download(QueueItem* qi) throw() : 
	target(qi->getTarget()), tempTarget(qi->getTempTarget()), file(NULL),
	crcCalc(NULL), tth(qi->getTTH()), treeValid(false) {

	setSize(qi->getSize());
	if(qi->isSet(QueueItem::FLAG_USER_LIST))
		setFlag(Download::FLAG_USER_LIST);
	if(qi->isSet(QueueItem::FLAG_RESUME))
		setFlag(Download::FLAG_RESUME);
}

AdcCommand Download::getCommand(bool zlib) {
	AdcCommand cmd(AdcCommand::CMD_GET);
	if(isSet(FLAG_TREE_DOWNLOAD)) {
		cmd.addParam("tthl");
	} else if(isSet(FLAG_PARTIAL_LIST)) {
		cmd.addParam("list");
	} else {
		cmd.addParam("file");
	}
	if(isSet(FLAG_PARTIAL_LIST) || isSet(FLAG_USER_LIST)) {
		cmd.addParam(Util::toAdcFile(getSource()));
	} else {
		cmd.addParam("TTH/" + getTTH().toBase32());
	}
	cmd.addParam(Util::toString(getPos()));
	cmd.addParam(Util::toString(getSize() - getPos()));

	if(zlib && BOOLSETTING(COMPRESS_TRANSFERS)) {
		cmd.addParam("ZL1");
	}

	return cmd;
}

int64_t Download::getTotalSecondsLeft() {
	updateRunningAverage();
	int64_t avg = DownloadManager::getInstance()->getAverageSpeed(target);
	u_int64_t pos = DownloadManager::getInstance()->getAveragePos(target);
	u_int64_t size = QueueManager::getInstance()->getTotalSize(target);
	if(size == 0)
		size = pos;
	return (avg > 0) ? ((size - pos) / avg) : 0;
}

DownloadManager::DownloadManager() {
	TimerManager::getInstance()->addListener(this);
}

DownloadManager::~DownloadManager() throw() {
	TimerManager::getInstance()->removeListener(this);
	while(true) {
		{
			Lock l(cs);
			if(downloads.empty())
				break;
		}
		Thread::sleep(100);
	}
}

void DownloadManager::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	Lock l(cs);

	averageSpeedMap.clear();
	averagePosMap.clear();

	Download::List tickList;
	// Tick each ongoing download
	for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
		if((*i)->getTotal() > 0) {
			tickList.push_back(*i);
		}
	
		Download* d = *i;
		string tmp = d->getTarget().substr(0, d->getTarget().rfind("\\"));
		StringIntIter j = averageSpeedMap.find(tmp);
		if(j != averageSpeedMap.end())
			j->second += d->getRunningAverage();
		else
			averageSpeedMap.insert(StringIntPair(tmp, d->getRunningAverage()));
		
		j = averagePosMap.find(tmp);
		if(j != averagePosMap.end())
			j->second += d->getPos();
		else
			averagePosMap.insert(StringIntPair(tmp, d->getPos()));
	}

	if(tickList.size() > 0)
		fire(DownloadManagerListener::Tick(), tickList);
}

void DownloadManager::FileMover::moveFile(const string& source, const string& target) {
	Lock l(cs);
	files.push_back(make_pair(source, target));
	if(!active) {
		active = true;
		start();
	}
}

int DownloadManager::FileMover::run() {
	for(;;) {
		FilePair next;
		{
			Lock l(cs);
			if(files.empty()) {
				active = false;
				return 0;
			}
			next = files.back();
			files.pop_back();
		}
		try {
			File::renameFile(next.first, next.second);
		} catch(const FileException&) {
			try {
				// Try to just rename it to the correct name at least
				string newTarget = Util::getFilePath(next.first) + Util::getFileName(next.second);
				File::renameFile(next.first, newTarget);
				LogManager::getInstance()->message(next.first + STRING(RENAMED_TO) + newTarget);
			} catch(const FileException& e) {
				LogManager::getInstance()->message(STRING(UNABLE_TO_RENAME) + next.first + ": " + e.getError());
			}
		}
		if(File::getSize(next.second) != -1) {
			StringPair sp = SettingsManager::getInstance()->getFileEvent(SettingsManager::ON_FILE_COMPLETE);
			if(sp.first.length() > 0) {
				STARTUPINFO si = { sizeof(si), 0 };
				PROCESS_INFORMATION pi = { 0 };
				StringMap params;
				params["file"] = next.second;
				wstring cmdLine = Text::toT(Util::formatParams(sp.second, params, false));
				wstring cmd = Text::toT(sp.first);

				AutoArray<TCHAR> cmdLineBuf(cmdLine.length() + 1);
				_tcscpy(cmdLineBuf, cmdLine.c_str());

				AutoArray<TCHAR> cmdBuf(cmd.length() + 1);
				_tcscpy(cmdBuf, cmd.c_str());

				if(::CreateProcess(cmdBuf, cmdLineBuf, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
					::CloseHandle(pi.hThread);
					::CloseHandle(pi.hProcess);
				}
			}
		}
	}
}

void DownloadManager::removeConnection(UserConnection::Ptr aConn) {
	dcassert(aConn->getDownload() == NULL);
	aConn->removeListener(this);
	aConn->disconnect();

	Lock l(cs);
	idlers.erase(remove(idlers.begin(), idlers.end(), aConn), idlers.end());
}

class TreeOutputStream : public OutputStream {
public:
	TreeOutputStream(TigerTree& aTree) : tree(aTree), bufPos(0) {
	}

	virtual size_t write(const void* xbuf, size_t len) throw(Exception) {
		size_t pos = 0;
		u_int8_t* b = (u_int8_t*)xbuf;
		while(pos < len) {
			size_t left = len - pos;
			if(bufPos == 0 && left >= TigerTree::HASH_SIZE) {
				tree.getLeaves().push_back(TTHValue(b + pos));
				pos += TigerTree::HASH_SIZE;
			} else {
				size_t bytes = min(TigerTree::HASH_SIZE - bufPos, left);
				memcpy(buf + bufPos, b + pos, bytes);
				bufPos += bytes;
				pos += bytes;
				if(bufPos == TigerTree::HASH_SIZE) {
					tree.getLeaves().push_back(TTHValue(buf));
					bufPos = 0;
				}
			}
		}
		return len;
	}

	virtual size_t flush() throw(Exception) {
		return 0;
	}
private:
	TigerTree& tree;
	u_int8_t buf[TigerTree::HASH_SIZE];
	size_t bufPos;
};

void DownloadManager::checkIdle(const User::Ptr& user) {
	Lock l(cs);
	for(UserConnection::Iter i = idlers.begin(); i != idlers.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == user) {
			idlers.erase(i);
			checkDownloads(uc);
			return;
		}
	}
}

void DownloadManager::addConnection(UserConnection::Ptr conn) {
	if(!conn->isSet(UserConnection::FLAG_SUPPORTS_TTHF) || !conn->isSet(UserConnection::FLAG_SUPPORTS_ADCGET)) {
		// Can't download from these...
		conn->getUser()->setFlag(User::OLD_CLIENT);
		QueueManager::getInstance()->removeUserFromQueue(conn->getUser(), QueueItem::Source::FLAG_NO_TTHF);
		removeConnection(conn);
	}

	conn->addListener(this);
	checkDownloads(conn);
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	dcassert(aConn->getDownload() == NULL);

	bool slotsFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (getDownloadCount() >= (size_t)SETTING(DOWNLOAD_SLOTS));
	bool speedFull = (SETTING(MAX_DOWNLOAD_SPEED) != 0) && (getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024));

	if( slotsFull || speedFull ) {
		bool extraFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (getDownloadCount() >= (size_t)(SETTING(DOWNLOAD_SLOTS)+3));
		if(extraFull || !QueueManager::getInstance()->hasDownload(aConn->getUser(), QueueItem::HIGHEST)) {
			removeConnection(aConn);
			return;
		}
	}

	Download* d = QueueManager::getInstance()->getDownload(aConn->getUser(), aConn->isSet(UserConnection::FLAG_SUPPORTS_TTHL));

	if(!d) {
		Lock l(cs);
		idlers.push_back(aConn);
		return;
	}

	d->setUserConnection(aConn);
	aConn->setDownload(d);

	aConn->setState(UserConnection::STATE_FILELENGTH);

	if(d->isSet(Download::FLAG_RESUME)) {
		dcassert(d->getSize() != -1);

		const string& target = (d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget());
		int64_t start = File::getSize(target);

		// Only use antifrag if we don't have a previous non-antifrag part
		if( BOOLSETTING(ANTI_FRAG) && (start == -1) && (d->getSize() != -1) ) {
			int64_t aSize = File::getSize(target + Util::ANTI_FRAG_EXT);

			if(aSize == d->getSize())
				start = d->getPos();
			else
				start = 0;

			d->setFlag(Download::FLAG_ANTI_FRAG);
		}

		if(d->getTreeValid() && start > 0 &&
		(d->getTigerTree().getLeaves().size() > 1 || aConn->isSet(UserConnection::FLAG_SUPPORTS_TTHL))) {
			d->setStartPos(getResumePos(d->getDownloadTarget(), d->getTigerTree(), start));
		} else {
			int rollback = SETTING(ROLLBACK);
			if(rollback > start) {
				d->setStartPos(0);
			} else {
				d->setStartPos(start - rollback);
				d->setFlag(Download::FLAG_ROLLBACK);
			}
		}

	} else {
		d->setStartPos(0);
	}

	if(d->isSet(Download::FLAG_USER_LIST)) {
		if(aConn->isSet(UserConnection::FLAG_SUPPORTS_XML_BZLIST)) {
			d->setSource(USER_LIST_NAME_BZ);
		} else {
			d->setSource(USER_LIST_NAME);
		}
	}

	{
		Lock l(cs);
		downloads.push_back(d);
	}

	aConn->send(d->getCommand(aConn->isSet(UserConnection::FLAG_SUPPORTS_ZLIB_GET)));
}

class DummyOutputStream : public OutputStream {
public:
	virtual size_t write(const void*, size_t n) throw(Exception) { return n; }
	virtual size_t flush() throw(Exception) { return 0; }
};

int64_t DownloadManager::getResumePos(const string& file, const TigerTree& tt, int64_t startPos) {
	// Always discard data until the last block
	if(startPos < tt.getBlockSize())
		return 0;

	startPos -= (startPos % tt.getBlockSize());

	DummyOutputStream dummy;

	vector<u_int8_t> buf((size_t)min((int64_t)1024*1024, tt.getBlockSize()));

	do {
		int64_t blockPos = startPos - tt.getBlockSize();

		try {
			MerkleCheckOutputStream<TigerTree, false> check(tt, &dummy, blockPos);

			File inFile(file, File::READ, File::OPEN);
			inFile.setPos(blockPos);
			int64_t bytesLeft = tt.getBlockSize();
			while(bytesLeft > 0) {
				size_t n = (size_t)min((int64_t)buf.size(), bytesLeft);
				size_t nr = inFile.read(&buf[0], n);
				check.write(&buf[0], nr);
				bytesLeft -= nr;
				if(bytesLeft > 0 && nr == 0) {
					// Huh??
					throw Exception();
				}
			}
			check.flush();
			break;
		} catch(const Exception&) {
			dcdebug("Removed bad block at " I64_FMT "\n", blockPos);
		}
		startPos = blockPos;
	} while(startPos > 0);
	return startPos;
}

void DownloadManager::on(AdcCommand::SND, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	const string& type = cmd.getParam(0);
	int64_t bytes = Util::toInt64(cmd.getParam(3));

	if(!(type == "file" || (type == "tthl" && aSource->getDownload()->isSet(Download::FLAG_TREE_DOWNLOAD)) ||
		(type == "list" && aSource->getDownload()->isSet(Download::FLAG_PARTIAL_LIST))) )
	{
		// Uhh??? We didn't ask for this?
		aSource->disconnect();
		return;
	}

	if(prepareFile(aSource, (bytes == -1) ? -1 : aSource->getDownload()->getPos() + bytes, cmd.hasFlag("ZL", 4))) {
		aSource->setDataMode();
	}
}

class RollbackException : public FileException {
public:
	RollbackException (const string& aError) : FileException(aError) { }
};

template<bool managed>
class RollbackOutputStream : public OutputStream {
public:
	RollbackOutputStream(File* f, OutputStream* aStream, size_t bytes) : s(aStream), pos(0), bufSize(bytes), buf(new u_int8_t[bytes]) {
		size_t n = bytes;
		f->read(buf, n);
		f->movePos(-((int64_t)bytes));
	}
	virtual ~RollbackOutputStream() throw() { delete[] buf; if(managed) delete s; }

	virtual size_t flush() throw(FileException) {
		return s->flush();
	}

	virtual size_t write(const void* b, size_t len) throw(FileException) {
		if(buf != NULL) {
			size_t n = min(len, bufSize - pos);

			u_int8_t* wb = (u_int8_t*)b;
			if(memcmp(buf + pos, wb, n) != 0) {
				throw RollbackException(STRING(ROLLBACK_INCONSISTENCY));
			}
			pos += n;
			if(pos == bufSize) {
				delete buf;
				buf = NULL;
			}
		}
		return s->write(b, len);
	}

private:
	OutputStream* s;
	size_t pos;
	size_t bufSize;
	u_int8_t* buf;
};


bool DownloadManager::prepareFile(UserConnection* aSource, int64_t newSize, bool z) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(newSize != -1) {
		d->setSize(newSize);
	}
	if(d->getPos() >= d->getSize()) {
		// Already finished?
		aSource->setDownload(NULL);
		removeDownload(d);
		QueueManager::getInstance()->putDownload(d, true);
		removeConnection(aSource);
		return false;
	}

	dcassert(d->getSize() != -1);

	if(d->isSet(Download::FLAG_PARTIAL_LIST)) {
		d->setFile(new StringOutputStream(d->getPFS()));
	} else if(d->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		d->setFile(new TreeOutputStream(d->getTigerTree()));
	} else {
		string target = d->getDownloadTarget();
		File::ensureDirectory(target);
		if(d->isSet(Download::FLAG_USER_LIST)) {
			if(aSource->isSet(UserConnection::FLAG_SUPPORTS_XML_BZLIST)) {
				target += ".xml.bz2";
			} else {
				target += ".xml";
			}
		}

		File* file = NULL;
		try {
			// Let's check if we can find this file in a any .SFV...
			int trunc = d->isSet(Download::FLAG_RESUME) ? 0 : File::TRUNCATE;
			file = new File(target, File::RW, File::OPEN | File::CREATE | trunc);
			if(d->isSet(Download::FLAG_ANTI_FRAG)) {
				file->setSize(d->getSize());
			}
			file->setPos(d->getPos());
		} catch(const FileException& e) {
			delete file;
			failDownload(aSource, STRING(COULD_NOT_OPEN_TARGET_FILE) + e.getError());
			return false;
		} catch(const Exception& e) {
			delete file;
			failDownload(aSource, e.getError());
			return false;
		}

		d->setFile(file);


		if(SETTING(BUFFER_SIZE) > 0 ) {
			d->setFile(new BufferedOutputStream<true>(d->getFile()));
		}

		bool sfvcheck = BOOLSETTING(SFV_CHECK) && (d->getPos() == 0) && (SFVReader(d->getTarget()).hasCRC());

		if(sfvcheck) {
			d->setFlag(Download::FLAG_CALC_CRC32);
			Download::CrcOS* crc = new Download::CrcOS(d->getFile());
			d->setCrcCalc(crc);
			d->setFile(crc);
		}

		/** @todo check the rest of the file when resuming? */
		if(d->getTreeValid()) {
			if((d->getPos() % d->getTigerTree().getBlockSize()) == 0) {
				d->setFile(new MerkleCheckOutputStream<TigerTree, true>(d->getTigerTree(), d->getFile(), d->getPos()));
				d->setFlag(Download::FLAG_TTH_CHECK);
			}
		}
		if(d->isSet(Download::FLAG_ROLLBACK)) {
			d->setFile(new RollbackOutputStream<true>(file, d->getFile(), (size_t)min((int64_t)SETTING(ROLLBACK), d->getSize() - d->getPos())));
		}

	}

	if(z) {
		d->setFlag(Download::FLAG_ZDOWNLOAD);
		d->setFile(new FilteredOutputStream<UnZFilter, true>(d->getFile()));
	}

	dcassert(d->getPos() != -1);
	d->setStart(GET_TICK());
	aSource->setState(UserConnection::STATE_DONE);

	fire(DownloadManagerListener::Starting(), d);

	return true;
}

void DownloadManager::on(UserConnectionListener::Data, UserConnection* aSource, const u_int8_t* aData, size_t aLen) throw() {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	try {
		d->addPos(d->getFile()->write(aData, aLen), aLen);

		if(d->getPos() > d->getSize()) {
			throw Exception(STRING(TOO_MUCH_DATA));
		} else if(d->getPos() == d->getSize()) {
			handleEndData(aSource);
			aSource->setLineMode(0);
		}
	} catch(const RollbackException& e) {
		QueueManager::getInstance()->removeUserFromFile(d->getTarget(), aSource->getUser(), QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY);
		d->resetPos();
		failDownload(aSource, e.getError());
	} catch(const FileException& e) {
		d->resetPos();
		failDownload(aSource, e.getError());
	} catch(const Exception& e) {
		// Nuke the bytes we have written, this is probably a compression error
		d->resetPos();
		failDownload(aSource, e.getError());
	}
}

/** Download finished! */
void DownloadManager::handleEndData(UserConnection* aSource) {

	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(d->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		d->getFile()->flush();
		delete d->getFile();
		d->setFile(NULL);

		int64_t bl = 1024;
		while(bl * (int64_t)d->getTigerTree().getLeaves().size() < d->getTigerTree().getFileSize())
			bl *= 2;
		d->getTigerTree().setBlockSize(bl);
		d->getTigerTree().calcRoot();

		if(!(d->getTTH() == d->getTigerTree().getRoot())) {
			// This tree is for a different file, remove from queue...
			removeDownload(d);
			fire(DownloadManagerListener::Failed(), d, STRING(INVALID_TREE));

			QueueManager::getInstance()->removeUserFromFile(d->getTarget(), aSource->getUser(), QueueItem::Source::FLAG_BAD_TREE, false);

			aSource->setDownload(NULL);
			QueueManager::getInstance()->putDownload(d, false);

			checkDownloads(aSource);
			return;
		}
		d->setTreeValid(true);
	} else {

		// Hm, if the real crc == 0, we'll get a file reread extra, but what the heck...
		u_int32_t crc = 0;

		// First, finish writing the file (flushing the buffers and closing the file...)
		try {
			d->getFile()->flush();
			if(d->getCrcCalc() != NULL)
				crc = d->getCrcCalc()->getFilter().getValue();
			delete d->getFile();
			d->setFile(NULL);
			d->setCrcCalc(NULL);

			// Check if we're anti-fragging...
			if(d->isSet(Download::FLAG_ANTI_FRAG)) {
				// Ok, rename the file to what we expect it to be...
				try {
					const string& tgt = d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget();
					File::renameFile(d->getDownloadTarget(), tgt);
					d->unsetFlag(Download::FLAG_ANTI_FRAG);
				} catch(const FileException& e) {
					dcdebug("AntiFrag: %s\n", e.getError().c_str());
					// Now what?
				}
			}
		} catch(const FileException& e) {
			failDownload(aSource, e.getError());
			return;
		}

		dcassert(d->getPos() == d->getSize());
		dcdebug("Download finished: %s, size " I64_FMT ", downloaded " I64_FMT "\n", d->getTarget().c_str(), d->getSize(), d->getTotal());

		// Check if we have some crc:s...
		if(BOOLSETTING(SFV_CHECK)) {
			if(!checkSfv(aSource, d, crc))
				return;
		}

		if(BOOLSETTING(LOG_DOWNLOADS) && (BOOLSETTING(LOG_FILELIST_TRANSFERS) || !d->isSet(Download::FLAG_USER_LIST)) && !d->isSet(Download::FLAG_TREE_DOWNLOAD)) {
			logDownload(aSource, d);
		}

		// Check if we need to move the file
		if( !d->getTempTarget().empty() && (Util::stricmp(d->getTarget().c_str(), d->getTempTarget().c_str()) != 0) ) {
			moveFile(d->getTempTarget(), d->getTarget());
		}
	}

	removeDownload(d);
	fire(DownloadManagerListener::Complete(), d);

	aSource->setDownload(NULL);
	QueueManager::getInstance()->putDownload(d, true);
	checkDownloads(aSource);
}

u_int32_t DownloadManager::calcCrc32(const string& file) throw(FileException) {
	File ff(file, File::READ, File::OPEN);
	CalcInputStream<CRC32Filter, false> f(&ff);

	const size_t BUF_SIZE = 1024*1024;
	AutoArray<u_int8_t> b(BUF_SIZE);
	size_t n = BUF_SIZE;
	while(f.read((u_int8_t*)b, n) > 0)
		;		// Keep on looping...

	return f.getFilter().getValue();
}

bool DownloadManager::checkSfv(UserConnection* aSource, Download* d, u_int32_t crc) {
	SFVReader sfv(d->getTarget());
	if(sfv.hasCRC()) {
		bool crcMatch = (crc == sfv.getCRC());
		if(!crcMatch && crc == 0) {
			// Blah. We have to reread the file...
			try {
				crcMatch = (calcCrc32(d->getDownloadTarget()) == sfv.getCRC());
			} catch(const FileException& ) {
				// Couldn't read the file to get the CRC(!!!)
				crcMatch = false;
			}
		}

		if(!crcMatch) {
			File::deleteFile(d->getDownloadTarget());
			dcdebug("DownloadManager: CRC32 mismatch for %s\n", d->getTarget().c_str());
			LogManager::getInstance()->message(STRING(SFV_INCONSISTENCY) + " (" + STRING(FILE) + ": " + d->getTarget() + ")");
			removeDownload(d);
			fire(DownloadManagerListener::Failed(), d, STRING(SFV_INCONSISTENCY));

			QueueManager::getInstance()->removeUserFromFile(d->getTarget(), aSource->getUser(), QueueItem::Source::FLAG_CRC_WARN, false);
			aSource->setDownload(NULL);
			QueueManager::getInstance()->putDownload(d, false);

			checkDownloads(aSource);
			return false;
		}

		d->setFlag(Download::FLAG_CRC32_OK);

		dcdebug("DownloadManager: CRC32 match for %s\n", d->getTarget().c_str());
	}
	return true;
}

void DownloadManager::logDownload(UserConnection* aSource, Download* d) {
	StringMap params;
	params["target"] = d->getTarget();
	params["userNI"] = Util::toString(ClientManager::getInstance()->getNicks(aSource->getUser()->getCID()));
	params["userI4"] = aSource->getRemoteIp();
	StringList hubNames = ClientManager::getInstance()->getHubNames(aSource->getUser()->getCID());
	if(hubNames.empty())
		hubNames.push_back(STRING(OFFLINE));
	params["hub"] = Util::toString(hubNames);
	StringList hubs = ClientManager::getInstance()->getHubs(aSource->getUser()->getCID());
	if(hubs.empty())
		hubs.push_back(STRING(OFFLINE));
	params["hubURL"] = Util::toString(hubs);
	params["fileSI"] = Util::toString(d->getSize());
	params["fileSIshort"] = Util::formatBytes(d->getSize());
	params["fileSIchunk"] = Util::toString(d->getTotal());
	params["fileSIchunkshort"] = Util::formatBytes(d->getTotal());
	params["fileSIactual"] = Util::toString(d->getActual());
	params["fileSIactualshort"] = Util::formatBytes(d->getActual());
	params["speed"] = Util::formatBytes(d->getAverageSpeed()) + "/s";
	params["time"] = Util::formatSeconds((GET_TICK() - d->getStart()) / 1000);
	params["sfv"] = Util::toString(d->isSet(Download::FLAG_CRC32_OK) ? 1 : 0);
	params["fileTR"] = d->getTTH().toBase32();
	LOG(LogManager::DOWNLOAD, params);
}

void DownloadManager::moveFile(const string& source, const string& target) {
	try {
		File::ensureDirectory(target);
		if(File::getSize(source) > MOVER_LIMIT) {
			mover.moveFile(source, target);
		} else {
			File::renameFile(source, target);
		}
	} catch(const FileException&) {
		try {
			if(!SETTING(DOWNLOAD_DIRECTORY).empty()) {
				File::renameFile(source, SETTING(DOWNLOAD_DIRECTORY) + Util::getFileName(target));
			} else {
				File::renameFile(source, Util::getFilePath(source) + Util::getFileName(target));
			}
		} catch(const FileException&) {
			try {
				File::renameFile(source, Util::getFilePath(source) + Util::getFileName(target));
			} catch(const FileException&) {
				// Ignore...
			}
		}
	}

	int64_t size = File::getSize(target);
	if(size > -1 && size <= MOVER_LIMIT) {
		StringPair sp = SettingsManager::getInstance()->getFileEvent(SettingsManager::ON_FILE_COMPLETE);
		if(sp.first.length() > 0) {
			STARTUPINFO si = { sizeof(si), 0 };
			PROCESS_INFORMATION pi = { 0 };
			StringMap params;
			params["file"] = target;
			wstring cmdLine = Text::toT(Util::formatParams(sp.second, params, false));
			wstring cmd = Text::toT(sp.first);

			AutoArray<TCHAR> cmdLineBuf(cmdLine.length() + 1);
			_tcscpy(cmdLineBuf, cmdLine.c_str());

			AutoArray<TCHAR> cmdBuf(cmd.length() + 1);
			_tcscpy(cmdBuf, cmd.c_str());

			if(::CreateProcess(cmdBuf, cmdLineBuf, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
				::CloseHandle(pi.hThread);
				::CloseHandle(pi.hProcess);
			}
		}
	}
}
void DownloadManager::noSlots(UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_FILELENGTH && aSource->getState() != UserConnection::STATE_TREE) {
		dcdebug("DM::onMaxedOut Bad state, ignoring\n");
		return;
	}

	failDownload(aSource, STRING(NO_SLOTS_AVAILABLE));
}

void DownloadManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) throw() {
	failDownload(aSource, aError);
}

void DownloadManager::failDownload(UserConnection* aSource, const string& reason) {
	Download* d = aSource->getDownload();

	if(d) {
		removeDownload(d);
		fire(DownloadManagerListener::Failed(), d, reason);

		aSource->setDownload(0);
		QueueManager::getInstance()->putDownload(d, false);
	}
	removeConnection(aSource);
}

void DownloadManager::removeDownload(Download* d) {
	if(d->getFile()) {
		if(d->getActual() > 0) {
			try {
				d->getFile()->flush();
			} catch(const Exception&) {
			}
		}
		delete d->getFile();
		d->setFile(NULL);
		d->setCrcCalc(NULL);

		if(d->isSet(Download::FLAG_ANTI_FRAG)) {
			d->unsetFlag(Download::FLAG_ANTI_FRAG);
		}
	}

	{
		Lock l(cs);
		// Either I'm stupid or the msvc7 optimizer is doing something _very_ strange here...
		// STL-port -D_STL_DEBUG complains that .begin() and .end() don't have the same owner (!),
		// but only in release build

		dcassert(find(downloads.begin(), downloads.end(), d) != downloads.end());

		downloads.erase(remove(downloads.begin(), downloads.end(), d), downloads.end());

/*		for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
			if(*i == d) {
				downloads.erase(i);
				break;
			}
		}*/
	}
}

/** @todo Handle errors better */
void DownloadManager::on(AdcCommand::STA, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(cmd.getParameters().size() < 2) {
		aSource->disconnect();
		return;
	}

	const string& err = cmd.getParameters()[0];
	if(err.length() < 3) {
		aSource->disconnect();
		return;
	}

	switch(Util::toInt(err.substr(0, 1))) {
	case AdcCommand::SEV_FATAL:
		aSource->disconnect();
		return;
	case AdcCommand::SEV_RECOVERABLE:
		switch(Util::toInt(err.substr(1))) {
		case AdcCommand::ERROR_FILE_NOT_AVAILABLE:
			fileNotAvailable(aSource);
			return;
		case AdcCommand::ERROR_SLOTS_FULL:
			noSlots(aSource);
			return;
		}
	}
	aSource->disconnect();
}

void DownloadManager::fileNotAvailable(UserConnection* aSource) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);
	dcdebug("File Not Available: %s\n", d->getTarget().c_str());

	removeDownload(d);
	fire(DownloadManagerListener::Failed(), d, d->getTargetFileName() + ": " + STRING(FILE_NOT_AVAILABLE));

	aSource->setDownload(NULL);

	QueueManager::getInstance()->removeUserFromFile(d->getTarget(), aSource->getUser(), d->isSet(Download::FLAG_TREE_DOWNLOAD) ? QueueItem::Source::FLAG_NO_TREE : QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);

	QueueManager::getInstance()->putDownload(d, false);
	checkDownloads(aSource);
}

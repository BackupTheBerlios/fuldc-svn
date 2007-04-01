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

#include "ShareManager.h"
#include "ResourceManager.h"

#include "CryptoManager.h"
#include "ClientManager.h"
#include "LogManager.h"
#include "HashManager.h"

#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "File.h"
#include "FilteredFile.h"
#include "BZUtils.h"
#include "Wildcards.h"

#if !(defined(_WIN32) || defined(_WIN64))
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#endif

#include <limits>

ShareManager::ShareManager() : hits(0), bzXmlListLen(0),
	xmlDirty(true), listN(0), 
	lastXmlUpdate(0), lastFullUpdate(GET_TICK()), bloom(1<<20), refreshing(0),
	lastIncomingUpdate(GET_TICK()), shareXmlDirty(false)
{ 
	SettingsManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	HashManager::getInstance()->addListener(this);
}

ShareManager::~ShareManager() {
	SettingsManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	HashManager::getInstance()->removeListener(this);

	join();

	StringList lists = File::findFiles(Util::getConfigPath(), "files?*.xml.bz2");
	for_each(lists.begin(), lists.end(), File::deleteFile);

	for(Directory::MapIter j = directories.begin(); j != directories.end(); ++j) {
		delete j->second;
	}
}

ShareManager::Directory::~Directory() {
	for(MapIter i = directories.begin(); i != directories.end(); ++i)
		delete i->second;
}

string ShareManager::Directory::getADCPath() const throw() {
	if(!getParent())
		return '/' + name + '/';
	return getParent()->getADCPath() + name + '/';
}

string ShareManager::Directory::getFullName() const throw() {
	if(!getParent())
		return getName() + '\\';
	return getParent()->getFullName() + getName() + '\\';
}

void ShareManager::Directory::addType(uint32_t type) throw() {
	if(!hasType(type)) {
		fileTypes |= (1 << type);
		if(getParent())
			getParent()->addType(type);
	}
}

string ShareManager::Directory::getRealPath() const throw() {
	if(getParent()) {
		return getParent()->getRealPath() + getName() + PATH_SEPARATOR_STR;
	} else {
		return ShareManager::getInstance()->virtualMap[const_cast<Directory*>(this)];
	}
}

int64_t ShareManager::Directory::getSize() const throw() {
	int64_t tmp = size;
	for(Map::const_iterator i = directories.begin(); i != directories.end(); ++i)
		tmp+=i->second->getSize();
	return tmp;
}

size_t ShareManager::Directory::countFiles() const throw() {
	size_t tmp = files.size();
	for(Map::const_iterator i = directories.begin(); i != directories.end(); ++i)
		tmp+=i->second->countFiles();
	return tmp;
}

string ShareManager::toVirtual(const TTHValue& tth) const throw(ShareException) {
	Lock l(cs);
	if(tth == bzXmlRoot) {
		return Transfer::USER_LIST_NAME_BZ;
	} else if(tth == xmlRoot) {
		return Transfer::USER_LIST_NAME;
	}

	HashFileMap::const_iterator i = tthIndex.find(tth);
	if(i != tthIndex.end()) {
		return i->second->getADCPath();
	} else {
		throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
	}
}

string ShareManager::toReal(const string& virtualFile) throw(ShareException) {
	if(virtualFile == "MyList.DcLst") {
		throw ShareException("NMDC-style lists no longer supported, please upgrade your client");
	} else if(virtualFile == Transfer::USER_LIST_NAME_BZ || virtualFile == Transfer::USER_LIST_NAME) {
		generateXmlList();
		return getBZXmlFile();
	} else {
		Lock l(cs);

		return findFile(virtualFile)->getRealPath();
	}
}

TTHValue ShareManager::getTTH(const string& virtualFile) const throw(ShareException) {
	Lock l(cs);
	if(virtualFile == Transfer::USER_LIST_NAME_BZ) {
		return bzXmlRoot;
	} else if(virtualFile == Transfer::USER_LIST_NAME) {
		return xmlRoot;
	}

	return findFile(virtualFile)->getTTH();
}

MemoryInputStream* ShareManager::getTree(const string& virtualFile) const {
	TigerTree tree;
	if(virtualFile.compare(0, 4, "TTH/") == 0) {
		if(!HashManager::getInstance()->getTree(TTHValue(virtualFile.substr(4)), tree))
			return 0;
	} else {
		try {
			TTHValue tth = getTTH(virtualFile);
			HashManager::getInstance()->getTree(tth, tree);
		} catch(const Exception&) {
			return 0;
		}
	}

	vector<uint8_t> buf = tree.getLeafData();
	return new MemoryInputStream(&buf[0], buf.size());
}

AdcCommand ShareManager::getFileInfo(const string& aFile) throw(ShareException) {
	if(aFile == Transfer::USER_LIST_NAME) {
		generateXmlList();
		AdcCommand cmd(AdcCommand::CMD_RES);
		cmd.addParam("FN", aFile);
		return cmd;
	} else if(aFile == Transfer::USER_LIST_NAME_BZ) {
		generateXmlList();

		AdcCommand cmd(AdcCommand::CMD_RES);
		cmd.addParam("FN", aFile);
		cmd.addParam("SI", Util::toString(bzXmlListLen));
		cmd.addParam("TR", bzXmlRoot.toBase32());
		return cmd;
	}

	if(aFile.compare(0, 4, "TTH/") != 0)
		throw ShareException(UserConnection::FILE_NOT_AVAILABLE);

	Lock l(cs);
	TTHValue val(aFile.substr(4));
	HashFileIter i = tthIndex.find(val);
	if(i == tthIndex.end()) {
		throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
	}

	const Directory::File& f = *i->second;
	AdcCommand cmd(AdcCommand::CMD_RES);
	cmd.addParam("FN", f.getADCPath());
	cmd.addParam("SI", Util::toString(f.getSize()));
	cmd.addParam("TR", f.getTTH().toBase32());
	return cmd;
}

ShareManager::Directory::File::Set::const_iterator ShareManager::findFile(const string& virtualFile) const throw(ShareException) {
	if(virtualFile.compare(0, 4, "TTH/") == 0) {
		HashFileMap::const_iterator i = tthIndex.find(TTHValue(virtualFile.substr(4)));
		if(i == tthIndex.end()) {
			throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
		}
		return i->second;
	}
	throw ShareException(UserConnection::FILE_NOT_AVAILABLE);
}

string ShareManager::validateVirtual(const string& aVirt) const throw() {
	string tmp = aVirt;
	string::size_type idx = 0;

	while( (idx = tmp.find_first_of("$|:\\/"), idx) != string::npos) {
		tmp[idx] = '_';
	}
	return tmp;
}

void ShareManager::load(SimpleXML& aXml) {
	Lock l(cs);

	if(aXml.findChild("Share")) {
		aXml.stepIn();
		while(aXml.findChild("Directory")) {
			const string& realPath = aXml.getChildData();
			if(realPath.empty()) {
				continue;
			}

			const string& virtualName = aXml.getChildAttrib("Virtual");
			string vName = validateVirtual(virtualName.empty() ? Util::getLastDir(realPath) : virtualName);

			directories[realPath] = new Directory(virtualName, 0);
			directories[realPath]->setIncoming(aXml.getBoolChildAttrib("Incoming"));

			virtualMap[directories[realPath]] = realPath;
		}
		aXml.stepOut();
	}
}

void ShareManager::save(SimpleXML& aXml) {
	Lock l(cs);

	aXml.addTag("Share");
	aXml.stepIn();
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		aXml.addTag("Directory", i->first);
		aXml.addChildAttrib("Virtual", i->second->getName());
		aXml.addChildAttrib("Incoming", i->second->getIncoming());
	}
	aXml.stepOut();
}

void ShareManager::addDirectory(const string& realPath, const string& virtualName) throw(ShareException) {
	if(realPath.empty() || virtualName.empty()) {
		throw ShareException(STRING(NO_DIRECTORY_SPECIFIED));
	}

	if(Util::stricmp(SETTING(TEMP_DOWNLOAD_DIRECTORY), realPath) == 0) {
		throw ShareException(STRING(DONT_SHARE_TEMP_DIRECTORY));
	}

	string vName = validateVirtual(virtualName);

	{
		Lock l(cs);

		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			if(Util::strnicmp(realPath, i->first, i->first.length()) == 0) {
				// Trying to share an already shared directory
				throw ShareException(STRING(DIRECTORY_ALREADY_SHARED));
			} else if(Util::strnicmp(realPath, i->first, realPath.length()) == 0) {
				// Trying to share a parent directory
				throw ShareException(STRING(REMOVE_ALL_SUBDIRECTORIES));
			}
		}

	}

	Directory* dp = buildTree(realPath, 0);
	dp->setName(vName);

	{
		Lock l(cs);
		addTree(*dp);

		directories[realPath] = dp;
		virtualMap[dp] = realPath;
		setDirty();
	}
}

void ShareManager::removeDirectory(const string& realPath) {
	if(realPath.empty())
		return;

	{
		Lock l(cs);

		Directory::MapIter i = directories.find(realPath);
		if(i != directories.end()) {
			virtualMap.erase(i->second);
			delete i->second;
			directories.erase(i);
		}

		rebuildIndices();
		setDirty();
	}

	HashManager::getInstance()->stopHashing(realPath);
}

void ShareManager::renameDirectory(const string& realPath, const string& virtualName) throw(ShareException) {
	string vName = validateVirtual(virtualName);

	Lock l(cs);
	
	Directory::MapIter j = directories.find(realPath);
	if(j == directories.end())
		return;

	j->second->setName(vName);
}

int64_t ShareManager::getShareSize(const string& realPath) const throw() {
	Lock l(cs);
	dcassert(realPath.size()>0);
	Directory::Map::const_iterator i = directories.find(realPath);

	if(i != directories.end()) {
		return i->second->getSize();
	}

	return -1;
}

int64_t ShareManager::getShareSize() const throw() {
	Lock l(cs);
	int64_t tmp = 0;
	for(Directory::Map::const_iterator i = directories.begin(); i != directories.end(); ++i) {
		tmp += i->second->getSize();
	}
	return tmp;
}

size_t ShareManager::getSharedFiles() const throw() {
	Lock l(cs);
	size_t tmp = 0;
	for(Directory::Map::const_iterator i = directories.begin(); i != directories.end(); ++i) {
		tmp += i->second->countFiles();
	}
	return tmp;
}

class FileFindIter {
#if defined(_WIN32) || defined(_WIN64)
public:
	/** End iterator constructor */
	FileFindIter() : handle(INVALID_HANDLE_VALUE) { }
	/** Begin iterator constructor, path in utf-8 */
	FileFindIter(const string& path) : handle(INVALID_HANDLE_VALUE) {
		handle = ::FindFirstFile(Text::toT(path).c_str(), &data);
	}

	~FileFindIter() {
		if(handle != INVALID_HANDLE_VALUE) {
			::FindClose(handle);
		}
	}

	FileFindIter& operator++() {
		if(!::FindNextFile(handle, &data)) {
			::FindClose(handle);
			handle = INVALID_HANDLE_VALUE;
		}
		return *this;
	}
	bool operator!=(const FileFindIter& rhs) const { return handle != rhs.handle; }

	struct DirData : public WIN32_FIND_DATA {
		string getFileName() {
			return Text::fromT(cFileName);
		}

		bool isDirectory() {
			return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
		}

		bool isHidden() {
			return ((dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (cFileName[0] == L'.'));
		}

		int64_t getSize() {
			return (int64_t)nFileSizeLow | ((int64_t)nFileSizeHigh)<<32;
		}

		uint32_t getLastWriteTime() {
			return File::convertTime(&ftLastWriteTime);
		}
	};


private:
	HANDLE handle;
#else
// This code has been cleaned up/fixed a little.
public:
	FileFindIter() {
		dir = NULL;
		data.ent = NULL;
	}

	~FileFindIter() {
		if (dir) closedir(dir);
	}

	FileFindIter(const string& name) {
		dir = opendir(name.c_str());
		if (!dir)
			return;
		data.base = name;
		data.ent = readdir(dir);
		if (!data.ent) {
			closedir(dir);
			dir = NULL;
		}
	}

	FileFindIter& operator++() {
		if (!dir)
			return *this;
		data.ent = readdir(dir);
		if (!data.ent) {
			closedir(dir);
			dir = NULL;
		}
		return *this;
	}

	// good enough to to say if it's null
	bool operator !=(const FileFindIter& rhs) const {
		return dir != rhs.dir;
	}

	struct DirData {
		DirData() : ent(NULL) {}
		string getFileName() {
			if (!ent) return Util::emptyString;
			return string(ent->d_name);
		}
		bool isDirectory() {
			struct stat inode;
			if (!ent) return false;
			if (stat((base + PATH_SEPARATOR + ent->d_name).c_str(), &inode) == -1) return false;
			return S_ISDIR(inode.st_mode);
		}
		bool isHidden() {
			if (!ent) return false;
			return ent->d_name[0] == '.';
		}
		int64_t getSize() {
			struct stat inode;
			if (!ent) return false;
			if (stat((base + PATH_SEPARATOR + ent->d_name).c_str(), &inode) == -1) return 0;
			return inode.st_size;
		}
		uint32_t getLastWriteTime() {
			struct stat inode;
			if (!ent) return false;
			if (stat((base + PATH_SEPARATOR + ent->d_name).c_str(), &inode) == -1) return 0;
			return inode.st_mtime;
		}
		struct dirent* ent;
		string base;
	};
private:
	DIR* dir;
#endif

public:

	DirData& operator*() { return data; }
	DirData* operator->() { return &data; }

private:
	DirData data;

};

ShareManager::Directory* ShareManager::buildTree(const string& aName, Directory* aParent) {
	Directory* dir = new Directory(Util::getLastDir(aName), aParent);
	dir->addType(SearchManager::TYPE_DIRECTORY); // needed since we match our own name in directory searches

	Directory::File::Iter lastFileIter = dir->files.begin();

	FileFindIter end;
#if defined(_WIN32) || defined(_WIN64)
		for(FileFindIter i(aName + "*"); i != end; ++i) {
#else
	//the fileiter just searches directorys for now, not sure if more
	//will be needed later
	//for(FileFindIter i(aName + "*"); i != end; ++i) {
	for(FileFindIter i(aName); i != end; ++i) {
#endif
		string name = i->getFileName();

		if(name == "." || name == "..")
			continue;
		if(name.find('$') != string::npos) {
			LogManager::getInstance()->message(STRING(FORBIDDEN_DOLLAR_FILE) + name + " (" + STRING(SIZE) + ": " + Util::toString(File::getSize(name)) + " " + STRING(B) + ") (" + STRING(DIRECTORY) + ": \"" + aName + "\")");
			continue;
		}

		if( Wildcard::patternMatch( name, SETTING(SKIPLIST_SHARE), '|' ) )
			continue;

		if(!BOOLSETTING(SHARE_HIDDEN) && i->isHidden() )
			continue;

		if(i->isDirectory()) {
			string newName = aName + name + PATH_SEPARATOR;
			if(Util::stricmp(newName + PATH_SEPARATOR, SETTING(TEMP_DOWNLOAD_DIRECTORY)) != 0) {
				dir->directories[name] = buildTree(newName, dir);
			}
		} else {
			// Not a directory, assume it's a file...make sure we're not sharing the settings file...
			if( (Util::stricmp(name.c_str(), "DCPlusPlus.xml") != 0) && 
				(Util::stricmp(name.c_str(), "Favorites.xml") != 0) &&
				(Util::stricmp(Util::getFileExt(name).c_str(), Util::TEMP_EXT) != 0) &&
				(Util::stricmp(Util::getFileExt(name).c_str(), Util::ANTI_FRAG_EXT) != 0) ){

				int64_t size = i->getSize();
				string fileName = aName + name;

				try {
					HashManager::getInstance()->checkTTH(fileName, size, i->getLastWriteTime());
					lastFileIter = dir->files.insert(lastFileIter, Directory::File(name, size, dir, HashManager::getInstance()->getTTH(fileName, size)));
				} catch(const HashException&) {
				}
			}
		}
	}

	return dir;
}

void ShareManager::addTree(Directory& dir) {
	bloom.add(Text::toLower(dir.getName()));
	//reset the size to avoid increasing the share size
	//on every refresh.
	dir.size = 0;

	for(Directory::MapIter i = dir.directories.begin(); i != dir.directories.end(); ++i) {
		addTree(*i->second);
	}

	for(Directory::File::Iter i = dir.files.begin(); i != dir.files.end(); ) {
		addFile(dir, i++);
	}
}

void ShareManager::rebuildIndices() {
	tthIndex.clear();
	bloom.clear();

	for(Directory::Map::const_iterator i = directories.begin(); i != directories.end(); ++i) {
		addTree(*i->second);
	}
}

void ShareManager::addFile(Directory& dir, Directory::File::Iter i) {
	const Directory::File& f = *i;

	dir.size+=f.getSize();
	
	dir.addType(getType(f.getName()));

	tthIndex.insert(make_pair(f.getTTH(), i));
	bloom.add(Text::toLower(f.getName()));
}

int ShareManager::refresh(int aRefreshOptions) throw(ShareException) 
{
	if(Thread::safeInc(refreshing) > 1) {
		Thread::safeDec(refreshing);
		LogManager::getInstance()->message(STRING(FILE_LIST_REFRRESH_IN_PROGRESS));
		return REFRESH_IN_PROGRESS;
	}

	refreshOptions = aRefreshOptions;

	join();
	try {
		start();
		if(refreshOptions & REFRESH_BLOCKING) {
			join();
		} else {
			setThreadPriority(Thread::LOW);
		}
	} catch(const ThreadException& e) {
		LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_FAILED) + e.getError());
	}

	return REFRESH_STARTED;
}

ShareManager::DirectoryInfoList ShareManager::getDirectories(int refreshOptions) const throw() {
	Lock l(cs);
	DirectoryInfoList result;
	if(refreshOptions & REFRESH_ALL) {
		for(Directory::Map::const_iterator i = directories.begin(); i != directories.end(); ++i) {
			result.push_back(DirectoryInfo(i->first, i->second->getName(), i->second->getIncoming()));
		}
	} else if(refreshOptions & REFRESH_DIRECTORY) {
		for(StringIterC j = refreshPaths.begin(); j != refreshPaths.end(); ++j) {
			string bla = *j;
			result.push_back(DirectoryInfo(bla, directories.find(bla)->second->getName(), directories.find(bla)->second->getIncoming()));
		}
	} else if(refreshOptions & REFRESH_INCOMING) {
		for(Directory::Map::const_iterator i = directories.begin(); i != directories.end(); ++i) {
			if(i->second->getIncoming())
				result.push_back(DirectoryInfo(i->first, i->second->getName(), true));
		}
	}
	
	return result;
}

int ShareManager::run() {
	//cache the paused state of hashmanager, if it's paused when we start, don't resume it when we're finished.
	bool pause = HashManager::getInstance()->isPaused();
	if(!pause)
		HashManager::getInstance()->pause();
	LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_INITIATED));
	{
		if(refreshOptions & REFRESH_ALL)
			lastFullUpdate = GET_TICK();
		if(refreshOptions & REFRESH_INCOMING)
			lastIncomingUpdate = GET_TICK();

		DirectoryInfoList dirs = getDirectories(refreshOptions);

		Directory::Map newDirs;
		for(DirectoryInfoList::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
			Directory* dp = buildTree(i->realPath, 0);
			dp->setName(i->virtualName);
			dp->setIncoming(i->incoming);
			newDirs.insert(make_pair(i->realPath, dp));
		}

		{
			Lock l(cs);
			for(DirectoryInfoList::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
				delete &directories[i->realPath];
				directories.erase(i->realPath);
			}
			directories.insert(newDirs.begin(), newDirs.end());
			virtualMap.clear();
			for(Directory::Map::const_iterator i = directories.begin(); i != directories.end(); ++i) {
				virtualMap[i->second] = i->first;
			}

			rebuildIndices();
		}
	}

	Thread::safeDec(refreshing);

	LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_FINISHED));
	if(!pause)
		HashManager::getInstance()->resume();
	if(refreshOptions & REFRESH_UPDATE) {
		ClientManager::getInstance()->infoUpdated();
	}
	refreshing = 0;
	return 0;
}

void ShareManager::generateXmlList(bool force /* = false */ ) {
	Lock l(cs);
	if(xmlDirty && (lastXmlUpdate + 15 * 60 * 1000 < GET_TICK() || lastXmlUpdate < lastFullUpdate || force ) ) {
		listN++;

		try {
			SimpleXML xml;

			xml.addTag("FileListing");
			xml.addChildAttrib("Version", 1);
			xml.addChildAttrib("CID", SETTING(CLIENT_ID));
			xml.addChildAttrib("Base", string("/"));
			xml.addChildAttrib("Generator", string(APPNAME " " VERSIONSTRING));
			xml.stepIn();

			for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
				i->second->toXml(xml, true);
			}
			
			string newXmlName = Util::getConfigPath() + "files" + Util::toString(listN) + ".xml.bz2";
			{
				FilteredOutputStream<BZFilter, true> newXmlFile(new File(newXmlName, File::WRITE, File::TRUNCATE | File::CREATE));
				xml.stepOut();
				newXmlFile.write(SimpleXML::utf8Header);
				xml.toXML(&newXmlFile);
				newXmlFile.flush();
			}

			if(bzXmlRef.get()) {
				bzXmlRef.reset();
				File::deleteFile(getBZXmlFile());
			}

			try {
				File::renameFile(newXmlName, Util::getConfigPath() + "files.xml.bz2");
				newXmlName = Util::getConfigPath() + "files.xml.bz2";
			} catch(const FileException&) {
				// Ignore, this is for caching only...
			}
			bzXmlRef = auto_ptr<File>(new File(newXmlName, File::READ, File::OPEN));
			setBZXmlFile(newXmlName);
			bzXmlListLen = File::getSize(newXmlName);
		} catch(const Exception&) {
			// No new file lists...
		}

		xmlDirty = false;
		lastXmlUpdate = GET_TICK();
	}
}

static const string& escaper(const string& n, string& tmp) {
	if(SimpleXML::needsEscape(n, true, false)) {
		tmp.clear();
		tmp.append(n);
		return SimpleXML::escape(tmp, true, false);
	}
	return n;
}

#define LITERAL(n) n, sizeof(n)-1

void ShareManager::Directory::toXml(SimpleXML& aXml, bool recurse) {
	bool create = true;
	aXml.resetCurrentChild();

	while( aXml.findChild("Directory") ){
		if( Util::stricmp(aXml.getChildAttrib("Name"), name) == 0 ){
			create = false;
			break;	
		}
	}

	if(create) {
		aXml.addTag("Directory");
		aXml.forceEndTag();
		aXml.addChildAttrib("Name", name);
		if(!recurse && (!directories.empty() || !files.empty()))
			aXml.addChildAttrib("Incomplete", 1);
	}

	if(recurse) {
		aXml.stepIn();

		for(MapIter i = directories.begin(); i != directories.end(); ++i) {
			i->second->toXml(aXml, recurse);
		}

		filesToXml(aXml);

		aXml.stepOut();
	}
}

void ShareManager::Directory::filesToXml(SimpleXML& aXml) {
	for(Directory::File::Iter i = files.begin(); i != files.end(); ++i) {
		const Directory::File& f = *i;

		aXml.addTag("File");
		aXml.addChildAttrib("Name", f.getName() );
		aXml.addChildAttrib("Size", f.getSize() );
		aXml.addChildAttrib("TTH", f.getTTH().toBase32() );
	}
}

// These ones we can look up as ints (4 bytes...)...

static const char* typeAudio[] = { ".mp3", ".mp2", ".mid", ".wav", ".ogg", ".wma" };
static const char* typeCompressed[] = { ".zip", ".ace", ".rar" };
static const char* typeDocument[] = { ".htm", ".doc", ".txt", ".nfo" };
static const char* typeExecutable[] = { ".exe" };
static const char* typePicture[] = { ".jpg", ".gif", ".png", ".eps", ".img", ".pct", ".psp", ".pic", ".tif", ".rle", ".bmp", ".pcx" };
static const char* typeVideo[] = { ".mpg", ".mov", ".asf", ".avi", ".pxp", ".wmv", ".ogm", ".mkv" };

static const string type2Audio[] = { ".au", ".aiff", ".flac" };
static const string type2Picture[] = { ".ai", ".ps", ".pict" };
static const string type2Video[] = { ".rm", ".divx", ".mpeg" };

#define IS_TYPE(x) ( type == (*((uint32_t*)x)) )
#define IS_TYPE2(x) (Util::stricmp(aString.c_str() + aString.length() - x.length(), x.c_str()) == 0)

static bool checkType(const string& aString, int aType) {
	if(aType == SearchManager::TYPE_ANY)
		return true;

	if(aString.length() < 5)
		return false;

	const char* c = aString.c_str() + aString.length() - 3;
	if(!Text::isAscii(c))
		return false;

	uint32_t type = '.' | (Text::asciiToLower(c[0]) << 8) | (Text::asciiToLower(c[1]) << 16) | (((uint32_t)Text::asciiToLower(c[2])) << 24);

	switch(aType) {
	case SearchManager::TYPE_AUDIO:
		{
			for(size_t i = 0; i < (sizeof(typeAudio) / sizeof(typeAudio[0])); i++) {
				if(IS_TYPE(typeAudio[i])) {
					return true;
				}
			}
			if( IS_TYPE2(type2Audio[0]) || IS_TYPE2(type2Audio[1]) ) {
				return true;
			}
		}
		break;
	case SearchManager::TYPE_COMPRESSED:
		if( IS_TYPE(typeCompressed[0]) || IS_TYPE(typeCompressed[1]) || IS_TYPE(typeCompressed[2]) ) {
			return true;
		}
		break;
	case SearchManager::TYPE_DOCUMENT:
		if( IS_TYPE(typeDocument[0]) || IS_TYPE(typeDocument[1]) ||
			IS_TYPE(typeDocument[2]) || IS_TYPE(typeDocument[3]) ) {
			return true;
		}
		break;
	case SearchManager::TYPE_EXECUTABLE:
		if(IS_TYPE(typeExecutable[0]) ) {
			return true;
		}
		break;
	case SearchManager::TYPE_PICTURE:
		{
			for(size_t i = 0; i < (sizeof(typePicture) / sizeof(typePicture[0])); i++) {
				if(IS_TYPE(typePicture[i])) {
					return true;
				}
			}
			if( IS_TYPE2(type2Picture[0]) || IS_TYPE2(type2Picture[1]) || IS_TYPE2(type2Picture[2]) ) {
				return true;
			}
		}
		break;
	case SearchManager::TYPE_VIDEO:
		{
			for(size_t i = 0; i < (sizeof(typeVideo) / sizeof(typeVideo[0])); i++) {
				if(IS_TYPE(typeVideo[i])) {
					return true;
				}
			}
			if( IS_TYPE2(type2Video[0]) || IS_TYPE2(type2Video[1]) || IS_TYPE2(type2Video[2]) ) {
				return true;
			}
		}
		break;
	default:
		dcasserta(0);
		break;
	}
	return false;
}

SearchManager::TypeModes ShareManager::getType(const string& aFileName) const throw() {
	if(aFileName[aFileName.length() - 1] == PATH_SEPARATOR) {
		return SearchManager::TYPE_DIRECTORY;
	}

	if(checkType(aFileName, SearchManager::TYPE_VIDEO))
		return SearchManager::TYPE_VIDEO;
	else if(checkType(aFileName, SearchManager::TYPE_AUDIO))
		return SearchManager::TYPE_AUDIO;
	else if(checkType(aFileName, SearchManager::TYPE_COMPRESSED))
		return SearchManager::TYPE_COMPRESSED;
	else if(checkType(aFileName, SearchManager::TYPE_DOCUMENT))
		return SearchManager::TYPE_DOCUMENT;
	else if(checkType(aFileName, SearchManager::TYPE_EXECUTABLE))
		return SearchManager::TYPE_EXECUTABLE;
	else if(checkType(aFileName, SearchManager::TYPE_PICTURE))
		return SearchManager::TYPE_PICTURE;

	return SearchManager::TYPE_ANY;
}

/**
 * Alright, the main point here is that when searching, a search string is most often found in
 * the filename, not directory name, so we want to make that case faster. Also, we want to
 * avoid changing StringLists unless we absolutely have to --> this should only be done if a string
 * has been matched in the directory name. This new stringlist should also be used in all descendants,
 * but not the parents...
 */
void ShareManager::Directory::search(SearchResult::List& aResults, StringSearchList& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) throw() {
	// Skip everything if there's nothing to find here (doh! =)
	if(!hasType(aFileType))
		return;

	StringSearchList* cur = &aStrings;
	auto_ptr<StringSearchList> newStr;

	// Find any matches in the directory name
	for(StringSearchListIter k = aStrings.begin(); k != aStrings.end(); ++k) {
		if(k->match(name)) {
			if(!newStr.get()) {
				newStr = auto_ptr<StringSearchList>(new StringSearchList(aStrings));
			}
			newStr->erase(remove(newStr->begin(), newStr->end(), *k), newStr->end());
		}
	}

	if(newStr.get() != 0) {
		cur = newStr.get();
	}

	bool sizeOk = (aSearchType != SearchManager::SIZE_ATLEAST) || (aSize == 0);
	if( (cur->empty()) &&
		(((aFileType == SearchManager::TYPE_ANY) && sizeOk) || (aFileType == SearchManager::TYPE_DIRECTORY)) ) {
		// We satisfied all the search words! Add the directory...(NMDC searches don't support directory size)
		SearchResult* sr = new SearchResult(SearchResult::TYPE_DIRECTORY, 0, getFullName(), TTHValue());
		aResults.push_back(sr);
		ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
	}

	if(aFileType != SearchManager::TYPE_DIRECTORY) {
		for(File::Set::const_iterator i = files.begin(); i != files.end(); ++i) {

			if(aSearchType == SearchManager::SIZE_ATLEAST && aSize > i->getSize()) {
				continue;
			} else if(aSearchType == SearchManager::SIZE_ATMOST && aSize < i->getSize()) {
				continue;
			}
			StringSearchListIter j = cur->begin();
			for(; j != cur->end() && j->match(i->getName()); ++j)
				;	// Empty

			if(j != cur->end())
				continue;

			// Check file type...
			if(checkType(i->getName(), aFileType)) {
				SearchResult* sr = new SearchResult(SearchResult::TYPE_FILE, i->getSize(), getFullName() + i->getName(), i->getTTH());
				aResults.push_back(sr);
				ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
				if(aResults.size() >= maxResults) {
					break;
				}
			}
		}
	}

	for(Directory::Map::const_iterator l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, *cur, aSearchType, aSize, aFileType, aClient, maxResults);
	}
}

void ShareManager::search(SearchResult::List& results, const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) throw() {
	Lock l(cs);
	if(aFileType == SearchManager::TYPE_TTH) {
		if(aString.compare(0, 4, "TTH:") == 0) {
			TTHValue tth(aString.substr(4));
			HashFileMap::const_iterator i = tthIndex.find(tth);
			if(i != tthIndex.end()) {
				SearchResult* sr = new SearchResult(SearchResult::TYPE_FILE, i->second->getSize(),
					i->second->getParent()->getFullName() + i->second->getName(), i->second->getTTH());

				results.push_back(sr);
				ShareManager::getInstance()->addHits(1);
			}
		}
		return;
	}
	StringTokenizer<string> t(Text::toLower(aString), '$');
	StringList& sl = t.getTokens();
	if(!bloom.match(sl))
		return;

	StringSearchList ssl;
	for(StringList::iterator i = sl.begin(); i != sl.end(); ++i) {
		if(!i->empty()) {
			ssl.push_back(StringSearch(*i));
		}
	}
	if(ssl.empty())
		return;

	for(Directory::Map::const_iterator j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
		j->second->search(results, ssl, aSearchType, aSize, aFileType, aClient, maxResults);
	}
}

namespace {
	inline uint16_t toCode(char a, char b) { return (uint16_t)a | ((uint16_t)b)<<8; }
}

ShareManager::AdcSearch::AdcSearch(const StringList& params) : include(&includeX), gt(0),
	lt(numeric_limits<int64_t>::max()), hasRoot(false), isDirectory(false)
{
	for(StringIterC i = params.begin(); i != params.end(); ++i) {
		const string& p = *i;
		if(p.length() <= 2)
			continue;

		uint16_t cmd = toCode(p[0], p[1]);
		if(toCode('T', 'R') == cmd) {
			hasRoot = true;
			root = TTHValue(p.substr(2));
			return;
		} else if(toCode('A', 'N') == cmd) {
			includeX.push_back(StringSearch(p.substr(2)));
		} else if(toCode('N', 'O') == cmd) {
			exclude.push_back(StringSearch(p.substr(2)));
		} else if(toCode('E', 'X') == cmd) {
			ext.push_back(p.substr(2));
		} else if(toCode('G', 'E') == cmd) {
			gt = Util::toInt64(p.substr(2));
		} else if(toCode('L', 'E') == cmd) {
			lt = Util::toInt64(p.substr(2));
		} else if(toCode('E', 'Q') == cmd) {
			lt = gt = Util::toInt64(p.substr(2));
		} else if(toCode('T', 'Y') == cmd) {
			isDirectory = (p[2] == '2');
		}
	}
}

void ShareManager::Directory::search(SearchResult::List& aResults, AdcSearch& aStrings, StringList::size_type maxResults) const throw() {
	StringSearchList* cur = aStrings.include;
	StringSearchList* old = aStrings.include;

	auto_ptr<StringSearchList> newStr;

	// Find any matches in the directory name
	for(StringSearchListIter k = cur->begin(); k != cur->end(); ++k) {
		if(k->match(name) && !aStrings.isExcluded(name)) {
			if(!newStr.get()) {
				newStr = auto_ptr<StringSearchList>(new StringSearchList(*cur));
			}
			newStr->erase(remove(newStr->begin(), newStr->end(), *k), newStr->end());
		}
	}

	if(newStr.get() != 0) {
		cur = newStr.get();
	}

	bool sizeOk = (aStrings.gt == 0);
	if( cur->empty() && aStrings.ext.empty() && sizeOk ) {
		// We satisfied all the search words! Add the directory...
		SearchResult* sr = new SearchResult(SearchResult::TYPE_DIRECTORY, getSize(), getFullName(), TTHValue());
		aResults.push_back(sr);
		ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
	}

	if(!aStrings.isDirectory) {
		for(File::Set::const_iterator i = files.begin(); i != files.end(); ++i) {

			if(!(i->getSize() >= aStrings.gt)) {
				continue;
			} else if(!(i->getSize() <= aStrings.lt)) {
				continue;
			}

			if(aStrings.isExcluded(i->getName()))
				continue;

			StringSearchListIter j = cur->begin();
			for(; j != cur->end() && j->match(i->getName()); ++j)
				;	// Empty

			if(j != cur->end())
				continue;

			// Check file type...
			if(aStrings.hasExt(i->getName())) {

				SearchResult* sr = new SearchResult(SearchResult::TYPE_FILE,
					i->getSize(), getFullName() + i->getName(), i->getTTH());
				aResults.push_back(sr);
				ShareManager::getInstance()->addHits(1);
				if(aResults.size() >= maxResults) {
					return;
				}
			}
		}
	}

	for(Directory::Map::const_iterator l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, aStrings, maxResults);
	}
	aStrings.include = old;
}

void ShareManager::search(SearchResult::List& results, const StringList& params, StringList::size_type maxResults) throw() {
	AdcSearch srch(params);

	Lock l(cs);

	if(srch.hasRoot) {
		HashFileMap::const_iterator i = tthIndex.find(srch.root);
		if(i != tthIndex.end()) {
			SearchResult* sr = new SearchResult(SearchResult::TYPE_FILE,
				i->second->getSize(), i->second->getParent()->getFullName() + i->second->getName(),
				i->second->getTTH());
			results.push_back(sr);
			addHits(1);
		}
		return;
	}

	for(StringSearchListIter i = srch.includeX.begin(); i != srch.includeX.end(); ++i) {
		if(!bloom.match(i->getPattern()))
			return;
	}

	for(Directory::Map::const_iterator j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
		j->second->search(results, srch, maxResults);
	}
}

ShareManager::Directory* ShareManager::getDirectory(const string& fname) {
	for(Directory::MapIter mi = directories.begin(); mi != directories.end(); ++mi) {
		if(Util::strnicmp(fname, mi->first, mi->first.length()) == 0) {
			Directory* d = mi->second;

			string::size_type i;
			string::size_type j = mi->first.length();
			while( (i = fname.find(PATH_SEPARATOR, j)) != string::npos) {
				mi = d->directories.find(fname.substr(j, i-j));
				j = i + 1;
				if(mi == d->directories.end())
					return NULL;
				d = mi->second;
			}
			return d;
		}
	}
	return NULL;
}

void ShareManager::on(DownloadManagerListener::Complete, Download* d) throw() {
	if(BOOLSETTING(ADD_FINISHED_INSTANTLY)) {
		// Check if finished download is supposed to be shared
		Lock l(cs);
		const string& n = d->getTarget();
		for(Directory::MapIter i = directories.begin(); i != directories.end(); i++) {
			if(Util::strnicmp(i->first, n, i->first.size()) == 0 && n[i->first.size()] == PATH_SEPARATOR) {
				string s = n.substr(i->first.size()+1);
				try {
					// Schedule for hashing, it'll be added automatically later on...
					HashManager::getInstance()->checkTTH(n, d->getSize(), 0);
				} catch(const Exception&) {
					// Not a vital feature...
				}
				break;
			}
		}
	}
}

void ShareManager::on(HashManagerListener::TTHDone, const string& fname, const TTHValue& root) throw() {
	Lock l(cs);
	Directory* d = getDirectory(fname);
	if(d) {
		Directory::File::Set::const_iterator i = d->findFile(Util::getFileName(fname));
		if(i != d->files.end()) {
			if(root != i->getTTH())
				tthIndex.erase(i->getTTH());
			// Get rid of false constness...
			Directory::File* f = const_cast<Directory::File*>(&(*i));
			f->setTTH(root);
			tthIndex.insert(make_pair(f->getTTH(), i));
		} else {
			string name = Util::getFileName(fname);
			int64_t size = File::getSize(fname);
			Directory::File::Iter it = d->files.insert(Directory::File(name, size, d, root)).first;
			addFile(*d, it);
		}
		setDirty();
	}
}

void ShareManager::on(TimerManagerListener::Minute, time_t tick) throw() {
	bool r;
	if(BOOLSETTING(AUTO_UPDATE_LIST)) {
		r = true;
		if(lastFullUpdate + SETTING(SHARE_REFRESH_TIME) * 60 * 1000 < tick) {
			if(BOOLSETTING(REFRESH_SHARE_BETWEEN)) {
				time_t _tt = time(NULL);
				tm* _tm = localtime(&_tt);

				if(!(_tm->tm_hour >= SETTING(REFRESH_SHARE_BEGIN) &&
					_tm->tm_hour <= SETTING(REFRESH_SHARE_END))) {
					r = false;
				}
			}

			if(r) {
				try {
					refresh(ShareManager::REFRESH_ALL | ShareManager::REFRESH_UPDATE);
				} catch(const ShareException&) {
				}
			}
		}
	}
	if(BOOLSETTING(AUTO_UPDATE_INCOMING)) {
		r = true;
		if(lastIncomingUpdate + SETTING(INCOMING_REFRESH_TIME) * 60 * 1000 < tick) {
			if(BOOLSETTING(REFRESH_INCOMING_BETWEEN)) {
				time_t _tt = time(NULL);
				tm* _tm = localtime(&_tt);

				if(!(_tm->tm_hour >= SETTING(REFRESH_INCOMING_BEGIN) &&
					_tm->tm_hour <= SETTING(REFRESH_INCOMING_END))) {
					r = false;
				}
			}

			if(r) {
				try{
					refresh(ShareManager::REFRESH_INCOMING | ShareManager::REFRESH_UPDATE);
				} catch(const ShareException&){
				}
			}
		}
	}
}


bool ShareManager::loadXmlList(){
	string* xmlString = new string;
	const size_t BUF_SIZE = 64*1024;
	char *buf = new char[BUF_SIZE];
	uint32_t pos = 0;

	//try to read the xml from the file
	try{
		::File f(Util::getConfigPath() + "Share.xml.bz2", File::READ, File::OPEN);
		xmlString->reserve( static_cast<string::size_type>(f.getSize()) );
		FilteredInputStream<UnBZFilter, false> xmlFile(&f);
		for(;;) {
			size_t tmp = BUF_SIZE;
			pos = xmlFile.read(buf, tmp);
			xmlString->append(buf, pos);
			if(pos < BUF_SIZE)
				break;
		}
		f.close();
	}catch (Exception&) { 
        delete[] buf;
		delete xmlString;
		//if we for some reason failed, return false to indicate that a refresh is needed
		return false;
	}


	//cleanup
	if(buf) {
		delete[] buf;
		buf = NULL;
	}

	//same here =)
	if(xmlString->empty()){
		delete xmlString;
		return false;
	}

	bool result = true;

	SimpleXML xml;

	try{
		xml.fromXML(*xmlString);

		//step inside <Share>
		xml.resetCurrentChild();
		xml.findChild("Share");
		xml.stepIn();

		Lock l(cs);

		while (xml.findChild("Directory")) {
			string name = xml.getChildAttrib("Name");
			string path = xml.getChildAttrib("Path");
			if(path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;
			
			Directory *d = addDirectoryFromXml(xml, NULL, name, path); 
			d->setIncoming(directories[path]->getIncoming());
			virtualMap.erase(directories[path]);
			delete directories[path];
			directories[path] = d;
			virtualMap[d] = path;
			addTree(*d);
		}
		setDirty();
	} catch(SimpleXMLException &e){
		LogManager::getInstance()->message(e.getError());
		result = false;
	}

	delete xmlString;
	
	return result;
}

ShareManager::Directory* ShareManager::addDirectoryFromXml(SimpleXML& xml, Directory *aParent, string & aName, string & aPath){
	Directory * dir = new Directory(aName, aParent);
	dir->addType(SearchManager::TYPE_DIRECTORY);
	bloom.add(Text::toLower(dir->getName()));

	xml.stepIn();

	Directory::File::Iter lastFileIter = dir->files.begin();
	while (xml.findChild("File")) {
		string name = xml.getChildAttrib("Name");
		uint64_t size = xml.getLongLongChildAttrib("Size");

		string path;
		if( aPath[ aPath.length() -1 ] == PATH_SEPARATOR )
			path = aPath + name;
		else
			path = aPath + PATH_SEPARATOR + name;
		
		try{
			lastFileIter = dir->files.insert(lastFileIter, Directory::File(name, size, dir, HashManager::getInstance()->getTTH(path, size)));
		} catch (HashException&){
		}
	}

	xml.resetCurrentChild();

	while (xml.findChild("Directory")) {
		string name = xml.getChildAttrib("Name");
		string path = xml.getChildAttrib("Path");
		
		dir->directories[name] = addDirectoryFromXml(xml, dir, name, path);
	}


	xml.stepOut();

	return dir;
}

void ShareManager::saveXmlList(){
	Lock l(cs);
	string indent;
	FilteredOutputStream<BZFilter, true> *xmlFile = new FilteredOutputStream<BZFilter, true>(new File(Util::getConfigPath() + "Share.xml.bz2", File::WRITE, File::TRUNCATE | File::CREATE));
	try{
		xmlFile->write(SimpleXML::utf8Header);
		xmlFile->write("<Share>\r\n");

		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			i->second->toXmlList(xmlFile, indent, i->first);
		}
		xmlFile->write("</Share>");
		xmlFile->flush();
	}catch(Exception&){}

	delete xmlFile;
}

void ShareManager::Directory::toXmlList(OutputStream* xmlFile, string& indent, const string& path){
	string tmp, tmp2;

	xmlFile->write(indent);
	xmlFile->write(LITERAL("<Directory Name=\""));
	xmlFile->write(escaper(name, tmp));
	xmlFile->write(LITERAL("\" Path=\""));
	xmlFile->write(escaper(path, tmp));
	xmlFile->write(LITERAL("\">\r\n"));

	indent += '\t';
	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		if(path[ path.length() -1 ] == PATH_SEPARATOR )
			i->second->toXmlList(xmlFile, indent, path + i->first);
		else
			i->second->toXmlList(xmlFile, indent, path + PATH_SEPARATOR + i->first);
	}

	Directory::File::Iter j = files.begin();
	while(j != files.end()) {
		xmlFile->write(indent);
		xmlFile->write(LITERAL("<File Name=\""));
		xmlFile->write(escaper(j->getName(), tmp));
		xmlFile->write(LITERAL("\" Size=\""));
		xmlFile->write(Util::toString(j->getSize()));
		xmlFile->write(LITERAL("\"/>\r\n"));

		++j;
	}

	indent.erase(indent.length()-1);
	xmlFile->write(indent);
	xmlFile->write(LITERAL("</Directory>\r\n"));
}

bool ShareManager::isIncoming(const string& realPath) {
	string tmp = realPath;
	if( realPath[ realPath.length() -1 ] != PATH_SEPARATOR )
		tmp += PATH_SEPARATOR;

	return directories[tmp]->getIncoming();
}

void ShareManager::setIncoming( const string& realPath, bool incoming /*= true*/ ) {
	string tmp = realPath;
	if( realPath[ realPath.length() -1 ] != PATH_SEPARATOR )
		tmp += PATH_SEPARATOR;

	directories[tmp]->setIncoming(incoming);
}

int ShareManager::refresh( const string& aDir ){
	int result = REFRESH_PATH_NOT_FOUND;

	if(Thread::safeInc(refreshing) == 1) {
		string path = Text::toLower(aDir);

		if(path[ path.length() -1 ] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;

		{
			Lock l(cs);
			refreshPaths.clear();

			Directory::MapIter i = directories.find(path);

			if( i == directories.end() ) {
				for(Directory::Map::const_iterator j = directories.begin(); j != directories.end(); ++j) {
					if( Util::stricmp( j->second->getName(), aDir ) == 0 ) {
						refreshPaths.push_back( j->first );
						result = REFRESH_STARTED;
					}
				}
			} else {
				refreshPaths.push_back( path );
				result = REFRESH_STARTED;
			}
		}

		Thread::safeDec(refreshing);
		if(result == REFRESH_STARTED)
			result = refresh(ShareManager::REFRESH_DIRECTORY | ShareManager::REFRESH_UPDATE);

		return result;
	}

	Thread::safeDec(refreshing);
	LogManager::getInstance()->message(STRING(FILE_LIST_REFRRESH_IN_PROGRESS));
	return REFRESH_IN_PROGRESS;
}

StringList ShareManager::getVirtualDirectories() {
	StringList result;

	Lock l(cs);

	for(Directory::Map::const_iterator i = directories.begin(); i != directories.end(); ++i){
		bool exists = false;

		for(StringIter j = result.begin(); j != result.end(); ++j) {
			if( Util::stricmp( *j, i->second->getName() ) == 0 ){
				exists = true;
				break;
			}
		}

		if( !exists )
			result.push_back( i->second->getName() );
	}

	sort( result.begin(), result.end() );

	return result;
}

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

#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#endif

#include <limits>

ShareManager::ShareManager() : hits(0), listLen(0), bzXmlListLen(0),
	xmlDirty(true), nmdcDirty(false), refreshDirs(false), update(false), listN(0), lFile(NULL), 
	xFile(NULL), lastXmlUpdate(0), lastNmdcUpdate(0), lastFullUpdate(GET_TICK()), bloom(1<<20),
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

	delete lFile;
	lFile = NULL;
	delete xFile;
	xFile = NULL;

#ifdef _WIN32
	WIN32_FIND_DATA data;
	HANDLE hFind;

	hFind = FindFirstFile(Text::toT(Util::getAppPath() + "files*.xml.bz2").c_str(), &data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			if(_tcslen(data.cFileName) > 13) // length of "files.xml.bz2"
				File::deleteFile(Util::getAppPath() + Text::fromT(data.cFileName));			
		} while(FindNextFile(hFind, &data));

		FindClose(hFind);
	}

	hFind = FindFirstFile(Text::toT(Util::getAppPath() + "MyList*.DcLst").c_str(), &data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			File::deleteFile(Util::getAppPath() + Text::fromT(data.cFileName));			
		} while(FindNextFile(hFind, &data));

		FindClose(hFind);
	}

#else
	DIR* dir = opendir(Util::getAppName().c_str());
	if (dir) {
		while (struct dirent* ent = readdir(dir)) {
			if (fnmatch("files*.xml.bz2", ent->d_name, 0) == 0 ||
				fnmatch("MyList*.DcLst", ent->d_name, 0) == 0) {
					File::deleteFile(Util::getAppPath() + ent->d_name);	
				}
		}
		closedir(dir);
	}		
#endif

	for(Directory::MapIter j = directories.begin(); j != directories.end(); ++j) {
		delete j->second;
	}
}

ShareManager::Directory::~Directory() {
	for(MapIter i = directories.begin(); i != directories.end(); ++i)
		delete i->second;
	
#ifdef USE_TTH
	for(File::Iter i = files.begin(); i != files.end(); ++i) {
		ShareManager::getInstance()->removeTTH(i->getTTH(), i);
	}
#endif
}

string ShareManager::translateFileName(const string& aFile) throw(ShareException) {
	RLock<> l(cs);
	if(aFile == "MyList.DcLst") {
		generateNmdcList();
		return getListFile();
	} else if(aFile == "files.xml.bz2") {
		generateXmlList();
		return getBZXmlFile();
	} else {
		if(aFile.length() < 3)
			throw ShareException("File Not Available");

		string file;

		// Check for tth root identifier
		if(aFile.compare(0, 4, "TTH/") == 0) {
			TTHValue v(aFile.substr(4));
			HashFileIter i = tthIndex.find(&v);
			if(i != tthIndex.end()) {
				file = i->second->getADCPath();
			} else {
				throw ShareException("File Not Available");
			}
		} else if(aFile[0] != '/') {
			throw ShareException("File Not Available");
		}  else {
			file = aFile;
		}

		string::size_type i = file.find('/', 1);
		if(i == string::npos)
			throw ShareException("File Not Available");
		
		string aDir = file.substr(1, i-1);
		file = file.substr(i+1);
		bool found = false;
		Directory::File::Iter it;

		StringPairIter j = virtualMap.begin();
		for(; j != virtualMap.end(); ++j) {
			if( Util::stricmp(aDir, j->first) == 0 ){
				if( checkFile(j->second, file, it) ){
					found = true;
					break;
				}
			}
		}

		if( !found )
			throw ShareException("File Not Available");

		i = 0;

		while((i = file.find('/', i)) != string::npos) {
			file[i] = '\\';
		}
		
		return j->second + file;
	}
}

/** @todo Fix for file list */
AdcCommand ShareManager::getFileInfo(const string& aFile) throw(ShareException) {
	if(aFile.compare(0, 4, "TTH/") != 0)
		throw ShareException("File Not Available");

	RLock<> l(cs);
	TTHValue val(aFile.substr(4));
	HashFileIter i = tthIndex.find(&val);
	if(i == tthIndex.end()) {
		throw ShareException("File Not Available");
	}

	Directory::File::Iter f = i->second;
	AdcCommand cmd(AdcCommand::CMD_RES);
	cmd.addParam("FN", f->getADCPath());
	cmd.addParam("SI", Util::toString(f->getSize()));
	cmd.addParam("TR", f->getTTH().toBase32());
	return cmd;
}

//this method takes the full path instead of the virtual name
//since there can be several entries with the same virtual name
StringPairIter ShareManager::findVirtual(const string& name) {
	for(StringPairIter i = virtualMap.begin(); i != virtualMap.	end(); ++i) {
		if(Util::stricmp(name, i->second) == 0)
			return i;
	}
	return virtualMap.end();
}

StringPairIter ShareManager::lookupVirtual(const string& name) {
	for(StringPairIter i = virtualMap.begin(); i != virtualMap.	end(); ++i) {
		if(Util::stricmp(name, i->first) == 0)
			return i;
	}
	return virtualMap.end();
}

bool ShareManager::checkFile(const string& dir, const string& aFile, Directory::File::Iter& it) {
	Directory::MapIter mi = directories.find(dir);
	if(mi == directories.end())
		return false;
	Directory* d = mi->second;
								  
	string::size_type i;
	string::size_type j = 0;
	while( (i = aFile.find('/', j)) != string::npos) {
		mi = d->directories.find(aFile.substr(j, i-j));
		j = i + 1;
		if(mi == d->directories.end())
			return false;
		d = mi->second;
	}
	
	it = find_if(d->files.begin(), d->files.end(), Directory::File::StringComp(aFile.substr(j)));
	if(it == d->files.end())
		return false;

	return true;
}

string ShareManager::validateVirtual(const string& aVirt) {
	string tmp = aVirt;
	string::size_type idx;

	while( (idx = tmp.find_first_of("$|:\\/")) != string::npos) {
		tmp[idx] = '_';
	}
	return tmp;
}

void ShareManager::load(SimpleXML* aXml) {
	WLock<> l(cs);

	if(aXml->findChild("Share")) {
		aXml->stepIn();
		while(aXml->findChild("Directory")) {
			const string& virt = aXml->getChildAttrib("Virtual");
			string d(aXml->getChildData()), newVirt;

			if(d[d.length() - 1] != PATH_SEPARATOR)
				d += PATH_SEPARATOR;
			if(!virt.empty()) {
				newVirt = virt;
				if(newVirt[newVirt.length() - 1] == PATH_SEPARATOR) {
					newVirt.erase(newVirt.length() -1, 1);
				}
			} else {
				newVirt = Util::getLastDir(d);
			}

			// get rid of bad characters in virtual names
			newVirt = validateVirtual(newVirt);

			Directory* dp = new Directory(newVirt);
			directories[d] = dp;
			virtualMap.push_back(make_pair(newVirt, d));
			setIncoming(d, aXml->getBoolChildAttrib("Incoming"));
		}
		aXml->stepOut();
	}
}

void ShareManager::save(SimpleXML* aXml) {
	RLock<> l(cs);
	
	aXml->addTag("Share");
	aXml->stepIn();
	for(StringPairIter i = virtualMap.begin(); i != virtualMap.end(); ++i) {
		aXml->addTag("Directory", i->second);
		aXml->addChildAttrib("Virtual", i->first);
		aXml->addChildAttrib("Incoming", isIncoming(i->second));
	}
	aXml->stepOut();
}

void ShareManager::addDirectory(const string& aDirectory, const string& aName) throw(ShareException) {
	if(aDirectory.empty() || aName.empty()) {
		throw ShareException(STRING(NO_DIRECTORY_SPECIFIED));
	}

	if(Util::stricmp(SETTING(TEMP_DOWNLOAD_DIRECTORY), aDirectory) == 0) {
		throw ShareException(STRING(DONT_SHARE_TEMP_DIRECTORY));
	}

	string d(aDirectory);

	if(d[d.length() - 1] != PATH_SEPARATOR)
		d += PATH_SEPARATOR;

	string vName = validateVirtual(aName);

	Directory* dp = NULL;
	{
		RLock<> l(cs);

		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			if(Util::strnicmp(d, i->first, i->first.length()) == 0) {
				// Trying to share an already shared directory
				throw ShareException(STRING(DIRECTORY_ALREADY_SHARED));
			} else if(Util::strnicmp(d, i->first, d.length()) == 0) {
				// Trying to share a parent directory
				throw ShareException(STRING(REMOVE_ALL_SUBDIRECTORIES));
			}
		}

	}
	
	dp = buildTree(d, NULL);
	dp->setName(vName);

	{
		WLock<> l(cs);
		addTree(dp);

		directories[d] = dp;
		virtualMap.push_back(make_pair(vName, d));
		setDirty();
	}
}

void ShareManager::removeDirectory(const string& aDirectory, bool duringRefresh) {
	WLock<> l(cs);

	string d(aDirectory);

	if(d[d.length() - 1] != PATH_SEPARATOR)
		d += PATH_SEPARATOR;

	Directory::MapIter i = directories.find(d);
	if(i != directories.end()) {
		delete i->second;
		directories.erase(i);
	}

	for(StringPairIter j = virtualMap.begin(); j != virtualMap.end(); ++j) {
		if(Util::stricmp(j->second.c_str(), d.c_str()) == 0) {
			virtualMap.erase(j);
			break;
		}
	}

	if(!duringRefresh)
		HashManager::getInstance()->stopHashing(d);

	setDirty();
}

void ShareManager::renameDirectory(const string& path, const string& nName) throw(ShareException) {
	StringPairIter i;
	WLock<> l(cs);
	//Find the virtual name
	i = findVirtual(path);
	if( i != virtualMap.end() ) {
		// Valid newName, lets rename
		i->first = nName;

		if( directories.find(path) != directories.end() ) {
			directories.find(path)->second->setName(nName);
		}
	}

	//Do not call setDirty here since there might be more
	//dirs that should be renamed, this is to avoid creating
	//a new list during renaming.
}

int64_t ShareManager::getShareSize(const string& aDir) throw() {
	RLock<> l(cs);
	dcassert(aDir.size()>0);
	Directory::MapIter i = directories.find(aDir);

	if(i != directories.end()) {
		return i->second->getSize();
	}

	return -1;
}

int64_t ShareManager::getShareSize() throw() {
	RLock<> l(cs);
	int64_t tmp = 0;
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp += i->second->getSize();
	}
	return tmp;
}

size_t ShareManager::getSharedFiles() throw() {
	RLock<> l(cs);
	size_t tmp = 0;
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp += i->second->countFiles();
	}
	return tmp;
}


string ShareManager::Directory::getADCPath() const throw() {
	if(parent == NULL)
		return '/' + name + '/';
	return parent->getADCPath() + name + '/';
}
string ShareManager::Directory::getFullName() const throw() {
	if(parent == NULL)
		return getName() + '\\';
	return parent->getFullName() + getName() + '\\';
}

void ShareManager::Directory::addType(u_int32_t type) throw() {
	if(!hasType(type)) {
		fileTypes |= (1 << type);
		if(getParent() != NULL)
			getParent()->addType(type);
	}
}

class FileFindIter {
#ifdef _WIN32
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

		u_int32_t getLastWriteTime() {
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
		u_int32_t getLastWriteTime() {
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
#ifdef _WIN32
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
			if(Util::stricmp(newName, SETTING(TEMP_DOWNLOAD_DIRECTORY)) != 0) {
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

void ShareManager::addTree(Directory* dir) {
	bloom.add(Text::toLower(dir->getName()));

	for(Directory::MapIter i = dir->directories.begin(); i != dir->directories.end(); ++i) {
		Directory* d = i->second;
		addTree(d);
	}

	for(Directory::File::Iter i = dir->files.begin(); i != dir->files.end(); ) {
		addFile(dir, i++);
	}
}

void ShareManager::addFile(Directory* dir, Directory::File::Iter i) {
	const Directory::File& f = *i;

	dir->size+=f.getSize();
	dir->addType(getType(f.getName()));

	tthIndex.insert(make_pair(const_cast<TTHValue*>(&f.getTTH()), i));
	bloom.add(Text::toLower(f.getName()));
}

void ShareManager::removeTTH(const TTHValue& tth, const Directory::File::Iter& iter) {
	pair<HashFileIter, HashFileIter> range = tthIndex.equal_range(const_cast<TTHValue*>(&tth));
	for(HashFileIter j = range.first; j != range.second; ++j) {
		if(j->second == iter) {
			tthIndex.erase(j);
			break;
		}
	}
}

void ShareManager::refresh(bool dirs /* = false */, bool aUpdate /* = true */, bool block /* = false */, 
						   bool incoming /* = false */, bool dir /* = false*/) throw(ShareException) 
{
	update = aUpdate;
	refreshDirs = dirs;
	refreshIncoming = incoming;
	refreshDir = dir;
	join();
	start();
	if(block) {
		join();
	} else {
		setThreadPriority(Thread::LOW);
	}
}

int ShareManager::run() {
	LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_INITIATED));
	{
		if( refreshDir && !refreshDirs ){
			Directory::Map newDirs;
			{
				RLock<> l(cs);
				for(StringIter j = refreshPaths.begin(); j != refreshPaths.end(); ++j){
					Directory::MapIter i = directories.find( *j );
					if( i != directories.end() ){
						Directory* dp = buildTree(i->first, NULL);
						dp->setName(findVirtual(i->first)->first);
						newDirs.insert(make_pair(i->first, dp));
					}
				}
			}
			{
				WLock<> l(cs);
				StringPairList dirs = virtualMap;
				for(StringIter j = refreshPaths.begin(); j != refreshPaths.end(); ++j){
					removeDirectory(*j, true);
				}
				refreshPaths.clear();
				virtualMap = dirs;

				for(Directory::MapIter i = newDirs.begin(); i != newDirs.end(); ++i) {
					addTree(i->second);
					directories.insert(*i);
				}
			}
			refreshDir = false;
		}

		if(refreshIncoming && !refreshDirs) {
			lastIncomingUpdate = GET_TICK();
			Directory::Map newDirs;
			{
				RLock<> l(cs);
				for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
					if(isIncoming(i->first)) {
						Directory* dp = buildTree(i->first, NULL);
						dp->setName(findVirtual(i->first)->first);
						newDirs.insert(make_pair(i->first, dp));
					}
				}
			}
			{
				WLock<> l(cs);
				StringPairList dirs = virtualMap;
				for(StringPairIter i = dirs.begin(); i != dirs.end(); ++i) {
					if(isIncoming(i->second))
						removeDirectory(i->second, true);
				}
				
				virtualMap = dirs;

				for(Directory::MapIter i = newDirs.begin(); i != newDirs.end(); ++i) {
					addTree(i->second);
					directories.insert(*i);
				}
			}
			refreshIncoming = false;
		}

		if(refreshDirs) {
			lastFullUpdate = GET_TICK();
			StringPairList dirs;
			Directory::Map newDirs;
			{
				RLock<> l(cs);
				dirs = virtualMap;
			}

			for(StringPairIter i = dirs.begin(); i != dirs.end(); ++i) {
				Directory* dp = buildTree(i->second, NULL);
				dp->setName(i->first);
				newDirs.insert(make_pair(i->second, dp));
			}

			{
				WLock<> l(cs);
				for(StringPairIter i = dirs.begin(); i != dirs.end(); ++i) {
					removeDirectory(i->second, true);
				}
				bloom.clear();

				virtualMap = dirs;

				for(Directory::MapIter i = newDirs.begin(); i != newDirs.end(); ++i) {
					addTree(i->second);
					directories.insert(*i);
				}
			}
			refreshDirs = false;
		}
	}

	LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_FINISHED));
	if(update) {
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}

void ShareManager::generateXmlList(bool force /* = false */ ) {
	Lock l(listGenLock);
	if(xmlDirty && (lastXmlUpdate + 15 * 60 * 1000 < GET_TICK() || lastXmlUpdate < lastFullUpdate || force ) ) {
		listN++;

		try {
			SimpleXML *xml = new SimpleXML();

			xml->addTag("FileListing");
			xml->addChildAttrib("Version", 1);
			xml->addChildAttrib("CID", SETTING(CLIENT_ID));
			xml->addChildAttrib("Base", string("/"));
			xml->addChildAttrib("Generator", string(APPNAME " " VERSIONSTRING));
			xml->stepIn();

			for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
				i->second->toXml(xml, true);
			}
			
			string newXmlName = Util::getAppPath() + "files" + Util::toString(listN) + ".xml.bz2";
			{
				FilteredOutputStream<BZFilter, true> newXmlFile(new File(newXmlName, File::WRITE, File::TRUNCATE | File::CREATE));
				xml->stepOut();
				newXmlFile.write(SimpleXML::utf8Header);
				xml->toXML(&newXmlFile);
				newXmlFile.flush();
			}
			
			delete xml;

			
			if(xFile != NULL) {
				delete xFile;
				xFile = NULL;
				File::deleteFile(getBZXmlFile());
			}
			try {
				File::renameFile(newXmlName, Util::getAppPath() + "files.xml.bz2");
				newXmlName = Util::getAppPath() + "files.xml.bz2";
			} catch(const FileException&) {
				// Ignore, this is for caching only...
			}
			xFile = new File(newXmlName, File::READ, File::OPEN);
			setBZXmlFile(newXmlName);
			bzXmlListLen = File::getSize(newXmlName);
		} catch(const Exception&) {
			// No new file lists...
		}

		xmlDirty = false;
		lastXmlUpdate = GET_TICK();
	}
}
void ShareManager::generateNmdcList(bool force /* = false */) {
	Lock l(listGenLock);
	if(nmdcDirty && (lastNmdcUpdate + 15 * 60 * 1000 < GET_TICK() || lastNmdcUpdate < lastFullUpdate || force) ) {
		listN++;

		try {
			string tmp;
			string tmp2;
			string indent;

			for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
				i->second->toNmdc(tmp, indent, tmp2);
			}

			string newName = Util::getAppPath() + "MyList" + Util::toString(listN) + ".DcLst";
			tmp2.clear();
			CryptoManager::getInstance()->encodeHuffman(tmp, tmp2);
			File(newName, File::WRITE, File::CREATE | File::TRUNCATE).write(tmp2);

			if(lFile != NULL) {
				delete lFile;
				lFile = NULL;
				File::deleteFile(getListFile());
			}
			try {
				File::renameFile(newName, Util::getAppPath() + "MyList.DcLst");
				newName = Util::getAppPath() + "MyList.DcLst";
			} catch(const FileException&) {
			}
			lFile = new File(newName, File::READ, File::OPEN);
			setListFile(newName);
			listLen = File::getSize(newName);
		} catch(const Exception&) {
			// No new file lists...
		}

		nmdcDirty = false;
		lastNmdcUpdate = GET_TICK();
	}
}

MemoryInputStream* ShareManager::generatePartialList(const string& dir, bool recurse) {
	if(dir[0] != '/' || dir[dir.size()-1] != '/')
		return NULL;

	SimpleXML *xml = new SimpleXML();

	xml->addTag("FileListing");
	xml->addChildAttrib("Version", 1);
	xml->addChildAttrib("CID", SETTING(CLIENT_ID));
	xml->addChildAttrib("Base", dir);
	xml->addChildAttrib("Generator", string(APPNAME " " VERSIONSTRING));
	xml->stepIn();
	
	bool found = false;
	
	RLock<> l(cs);
	if(dir == "/") {
		found = true;
		for(ShareManager::Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			i->second->toXml(xml, recurse);
		}
	} else {
		StringList l;
		if(dir[0] =='/')
			l = StringTokenizer<string>(dir.substr(1), '/').getTokens();
		else
			l = StringTokenizer<string>(dir, '/').getTokens();

		for(StringPairIter i = virtualMap.begin(); i != virtualMap.end(); ++i) {
			if(Util::stricmp(i->first, l[0]) == 0) {
				ShareManager::Directory::MapIter j, m = directories.find(i->second);
				if(m == directories.end())
					continue;
				
				StringIter k = l.begin();
				//skip the first dir since we've already checked it
				++k;

				for(; k != l.end(); ++k) {
					j = m->second->directories.find(*k);
					if(j == m->second->directories.end()) {
						//oops this path doesn't exist in this physical dir, abort...
						break;
					}
					m = j;
				}

				//did we reach the end of the list? if so we found our dir =)
				if(k == l.end()) {
					found = true;
					for(j = m->second->directories.begin(); j != m->second->directories.end(); ++j) {
						j->second->toXml(xml, recurse);
					}
					m->second->filesToXml(xml);
				}
			}
		}
	}

	MemoryInputStream *mis = NULL;
	if(found) {
		string tmp = SimpleXML::utf8Header;
		StringOutputStream sos(tmp);
		xml->toXML(&sos);
		mis = new MemoryInputStream(tmp);
	}
	
	delete xml;

	return mis;
}

bool ShareManager::getTTH(const string& aFile, TTHValue& tth) throw() {
	if(aFile.length() < 3 || aFile[0] != '/')
		return false;

	string::size_type i = aFile.find('/', 1);
	if(i == string::npos)
		return false;

	RLock<> l(cs);
	StringPairIter j = lookupVirtual(aFile.substr(1, i-1));
	if(j == virtualMap.end()) {
		return false;
	}

	Directory::File::Iter it;
	if(!checkFile(j->second, aFile.substr(i + 1), it))
		return false;

	tth = it->getTTH();
	return true;
}

MemoryInputStream* ShareManager::getTree(const string& aFile) {
	TigerTree tree;
	if(aFile.compare(0, 4, "TTH/") == 0) {
		if(!HashManager::getInstance()->getTree(TTHValue(aFile.substr(4)), tree))
			return NULL;
	} else {
		try {
			TTHValue tth;
			if(getTTH(aFile, tth))
				HashManager::getInstance()->getTree(tth, tree);
		} catch(const Exception&) {
			return NULL;
		}
	}

	vector<u_int8_t> buf = tree.getLeafData();
	return new MemoryInputStream(&buf[0], buf.size());
}

static const string& escaper(const string& n, string& tmp) {
	if(SimpleXML::needsEscape(n, false, false)) {
		tmp.clear();
		tmp.append(n);
		return SimpleXML::escape(tmp, false, false);
	}
	return n;
}

#define LITERAL(n) n, sizeof(n)-1
void ShareManager::Directory::toNmdc(string& nmdc, string& indent, string& tmp2) {
	tmp2.clear();
	nmdc.append(indent);
	nmdc.append(Text::utf8ToAcp(name, tmp2));
	nmdc.append(LITERAL("\r\n"));

	indent += '\t';
	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		i->second->toNmdc(nmdc, indent, tmp2);
	}
	
	Directory::File::Iter j = files.begin();
	for(Directory::File::Iter i = files.begin(); i != files.end(); ++i) {
		const Directory::File& f = *i;
		nmdc.append(indent);
		tmp2.clear();
		nmdc.append(Text::utf8ToAcp(f.getName(), tmp2));
		nmdc.append(LITERAL("|"));
		nmdc.append(Util::toString(f.getSize()));
		nmdc.append(LITERAL("\r\n"));
	}
	indent.erase(indent.length()-1);
}

void ShareManager::Directory::toXml(SimpleXML* xml, bool recurse) {
	bool create = true;
	xml->resetCurrentChild();

	while( xml->findChild("Directory") ){
		if( Util::stricmp(xml->getChildAttrib("Name"), name) == 0 ){
			create = false;
			break;	
		}
	}

	if(create) {
		xml->addTag("Directory");
		xml->forceEndTag();
		xml->addChildAttrib("Name", name);
		if(!recurse && (!directories.empty() || !files.empty()))
			xml->addChildAttrib("Incomplete", 1);
	}

	if(recurse) {
		xml->stepIn();

		for(MapIter i = directories.begin(); i != directories.end(); ++i) {
			i->second->toXml(xml, recurse);
		}

		filesToXml(xml);

		xml->stepOut();
	}
}

void ShareManager::Directory::filesToXml(SimpleXML* xml) {
	for(Directory::File::Iter i = files.begin(); i != files.end(); ++i) {
		const Directory::File& f = *i;

		xml->addTag("File");
		xml->addChildAttrib("Name", f.getName() );
		xml->addChildAttrib("Size", f.getSize() );
		xml->addChildAttrib("TTH", f.getTTH().toBase32() );
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

#define IS_TYPE(x) ( type == (*((u_int32_t*)x)) )
#define IS_TYPE2(x) (Util::stricmp(aString.c_str() + aString.length() - x.length(), x.c_str()) == 0)

static bool checkType(const string& aString, int aType) {
	if(aType == SearchManager::TYPE_ANY)
		return true;

	if(aString.length() < 5)
		return false;
	
	const char* c = aString.c_str() + aString.length() - 3;
	if(!Text::isAscii(c))
		return false;

	u_int32_t type = '.' | (Text::asciiToLower(c[0]) << 8) | (Text::asciiToLower(c[1]) << 16) | (((u_int32_t)Text::asciiToLower(c[2])) << 24);

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

SearchManager::TypeModes ShareManager::getType(const string& aFileName) {
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
void ShareManager::Directory::search(SearchResult::List& aResults, StringSearch::List& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) throw() {
	// Skip everything if there's nothing to find here (doh! =)
	if(!hasType(aFileType))
		return;

	StringSearch::List* cur = &aStrings;
	auto_ptr<StringSearch::List> newStr;

	// Find any matches in the directory name
	for(StringSearch::Iter k = aStrings.begin(); k != aStrings.end(); ++k) {
		if(k->match(name)) {
			if(!newStr.get()) {
				newStr = auto_ptr<StringSearch::List>(new StringSearch::List(aStrings));
			}
			dcassert(find(newStr->begin(), newStr->end(), *k) != newStr->end());
			newStr->erase(find(newStr->begin(), newStr->end(), *k));
		}
	}

	if(newStr.get() != 0) {
		cur = newStr.get();
	}

	bool sizeOk = (aSearchType != SearchManager::SIZE_ATLEAST) || (aSize == 0);
	if( (cur->empty()) && 
		(((aFileType == SearchManager::TYPE_ANY) && sizeOk) || (aFileType == SearchManager::TYPE_DIRECTORY)) ) {
		// We satisfied all the search words! Add the directory...
		SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_DIRECTORY, 
			0, getFullName(), NULL, true);
		aResults.push_back(sr);
		ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
	}

	if(aFileType != SearchManager::TYPE_DIRECTORY) {
		for(File::Iter i = files.begin(); i != files.end(); ++i) {
			
			if(aSearchType == SearchManager::SIZE_ATLEAST && aSize > i->getSize()) {
				continue;
			} else if(aSearchType == SearchManager::SIZE_ATMOST && aSize < i->getSize()) {
				continue;
			}	
			StringSearch::Iter j = cur->begin();
			for(; j != cur->end() && j->match(i->getName()); ++j) 
				;	// Empty
			
			if(j != cur->end())
				continue;
			
			// Check file type...
			if(checkType(i->getName(), aFileType)) {
				
				SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_FILE, 
					i->getSize(), getFullName() + i->getName(), &i->getTTH(), true);
				aResults.push_back(sr);
				ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
				if(aResults.size() >= maxResults) {
					break;
				}
			}
		}
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, *cur, aSearchType, aSize, aFileType, aClient, maxResults);
	}
}

void ShareManager::search(SearchResult::List& results, const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) {
	RLock<> l(cs);
	if(aFileType == SearchManager::TYPE_TTH) {
		if(aString.compare(0, 4, "TTH:") == 0) {
			TTHValue tth(aString.substr(4));
			
			pair< HashFileIter, HashFileIter> iter = tthIndex.equal_range(&tth);
			for(; iter.first != iter.second; ++(iter.first) ){

				results.push_back( new SearchResult(aClient, SearchResult::TYPE_FILE, 
					iter.first->second->getSize(),
					iter.first->second->getParent()->getFullName() + iter.first->second->getName(), 
					&iter.first->second->getTTH(), true));

				ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
				if(ShareManager::getInstance()->getHits() == maxResults)
					break;
			}
		}
		return;
	}
	StringTokenizer<string> t(Text::toLower(aString), '$');
	StringList& sl = t.getTokens();
	if(!bloom.match(sl))
		return;

	StringSearch::List ssl;
	for(StringList::iterator i = sl.begin(); i != sl.end(); ++i) {
		if(!i->empty()) {
			ssl.push_back(StringSearch(*i));
		}
	}
	if(ssl.empty())
		return;

	for(Directory::MapIter j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
		j->second->search(results, ssl, aSearchType, aSize, aFileType, aClient, maxResults);
	}
}

namespace {
	inline u_int16_t toCode(char a, char b) { return (u_int16_t)a | ((u_int16_t)b)<<8; }
}

ShareManager::AdcSearch::AdcSearch(const StringList& params) : include(&includeX), gt(0), 
	lt(numeric_limits<int64_t>::max()), hasRoot(false), isDirectory(false)
{
	for(StringIterC i = params.begin(); i != params.end(); ++i) {
		const string& p = *i;
		if(p.length() <= 2)
			continue;

		u_int16_t cmd = toCode(p[0], p[1]);
		if(toCode('T', 'R') == cmd) {
			hasRoot = true;
			root = TTHValue(p.substr(2));
			return;
		} else if(toCode('+', '+') == cmd) {
			includeX.push_back(StringSearch(p.substr(2)));		
		} else if(toCode('-', '-') == cmd) {
			exclude.push_back(StringSearch(p.substr(2)));
		} else if(toCode('E', 'X') == cmd) {
			ext.push_back(p.substr(2));
		} else if(toCode('>', '=') == cmd) {
			gt = Util::toInt64(p.substr(2));
		} else if(toCode('<', '=') == cmd) {
			lt = Util::toInt64(p.substr(2));
		} else if(toCode('=', '=') == cmd) {
			lt = gt = Util::toInt64(p.substr(2));
		} else if(toCode('D', 'O') == cmd) {
			isDirectory = (p[2] != '0');
		}
	}
}

void ShareManager::Directory::search(SearchResult::List& aResults, AdcSearch& aStrings, StringList::size_type maxResults) throw() {
	StringSearch::List* cur = aStrings.include;
	StringSearch::List* old = aStrings.include;

	auto_ptr<StringSearch::List> newStr;

	// Find any matches in the directory name
	for(StringSearch::Iter k = cur->begin(); k != cur->end(); ++k) {
		if(k->match(name) && !aStrings.isExcluded(name)) {
			if(!newStr.get()) {
				newStr = auto_ptr<StringSearch::List>(new StringSearch::List(*cur));
			}
			dcassert(find(newStr->begin(), newStr->end(), *k) != newStr->end());
			newStr->erase(find(newStr->begin(), newStr->end(), *k));
		}
	}

	if(newStr.get() != 0) {
		cur = newStr.get();
	}

	bool sizeOk = (aStrings.gt == 0);
	if( cur->empty() && aStrings.ext.empty() && sizeOk ) {
		// We satisfied all the search words! Add the directory...
		SearchResult* sr = new SearchResult(SearchResult::TYPE_DIRECTORY, 
			0, getFullName(), NULL);
		aResults.push_back(sr);
		ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
	}

	if(!aStrings.isDirectory) {
		for(File::Iter i = files.begin(); i != files.end(); ++i) {

			if(!(i->getSize() >= aStrings.gt)) {
				continue;
			} else if(!(i->getSize() <= aStrings.lt)) {
				continue;
			}	

			if(aStrings.isExcluded(i->getName()))
				continue;

			StringSearch::Iter j = cur->begin();
			for(; j != cur->end() && j->match(i->getName()); ++j) 
				;	// Empty

			if(j != cur->end())
				continue;

			// Check file type...
			if(aStrings.hasExt(i->getName())) {

				SearchResult* sr = new SearchResult(SearchResult::TYPE_FILE, 
					i->getSize(), getFullName() + i->getName(), &i->getTTH());
				aResults.push_back(sr);
				ShareManager::getInstance()->addHits(1);
				if(aResults.size() >= maxResults) {
					return;
				}
			}
		}
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, aStrings, maxResults);
	}
	aStrings.include = old;
}

void ShareManager::search(SearchResult::List& results, const StringList& params, StringList::size_type maxResults) {
	AdcSearch srch(params);	

	RLock<> l(cs);

	if(srch.hasRoot) {
		HashFileIter i = tthIndex.find(&srch.root);
		if(i != tthIndex.end()) {
			SearchResult* sr = new SearchResult(SearchResult::TYPE_FILE, 
				i->second->getSize(), i->second->getParent()->getFullName() + i->second->getName(), 
				&i->second->getTTH());
			results.push_back(sr);
			ShareManager::getInstance()->addHits(1);
		}
		return;
	}

	for(StringSearch::Iter i = srch.includeX.begin(); i != srch.includeX.end(); ++i) {
		if(!bloom.match(i->getPattern()))
			return;
	}

	for(Directory::MapIter j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
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
		WLock<> l(cs);
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
	WLock<> l(cs);
	Directory* d = getDirectory(fname);
	if(d != NULL) {
        Directory::File::Iter i = d->findFile(Util::getFileName(fname));
		if(i != d->files.end()) {
			if(root != i->getTTH())
			removeTTH(i->getTTH(), i);
			// Get rid of false constness...
			Directory::File* f = const_cast<Directory::File*>(&(*i));
			f->setTTH(root);
			tthIndex.insert(make_pair(const_cast<TTHValue*>(&f->getTTH()), i));
		} else {
			string name = Util::getFileName(fname);
			int64_t size = File::getSize(fname);
			Directory::File::Iter it = d->files.insert(Directory::File(name, size, d, root)).first;
			addFile(d, it);
		}
		setDirty();
	}
}

void ShareManager::on(TimerManagerListener::Minute, u_int32_t tick) throw() {
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
					refresh(true, true);
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
					refresh(false, true, false, true);
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
	u_int32_t pos = 0;

	//try to read the xml from the file
	try{
		::File f(Util::getAppPath() + "Share.xml.bz2", File::READ, File::OPEN);
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

	SimpleXML *xml = NULL;

	try{
		xml = new SimpleXML();
		xml->fromXML(*xmlString);

		//stepin inside <Share>
		xml->resetCurrentChild();
		xml->findChild("Share");
		xml->stepIn();

		for(Directory::MapIter j = directories.begin(); j != directories.end(); ++j) {
			delete j->second;
		}
		directories.clear();
		virtualMap.clear();

		WLock<> l(cs);

		while (xml->findChild("Directory")) {
			string name = xml->getChildAttrib("Name");
			string path = xml->getChildAttrib("Path");
			if(path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;
			
			Directory *d = addDirectoryFromXml(xml, NULL, name, path); 
			addTree(d);
			directories[path] = d;
			virtualMap.push_back(make_pair(name,path));
		}
		setDirty();
	} catch(SimpleXMLException &e){
		LogManager::getInstance()->message(e.getError());
		result = false;
	}

	//cleanup
	delete xmlString;
	xmlString = NULL;
	if(xml != NULL)
		delete xml;

	return result;
}

ShareManager::Directory* ShareManager::addDirectoryFromXml(SimpleXML *xml, Directory *aParent, string & aName, string & aPath){
	Directory * dir = new Directory(aName, aParent);
	dir->addType(SearchManager::TYPE_DIRECTORY);
	bloom.add(Text::toLower(dir->getName()));

	xml->stepIn();

	Directory::File::Iter lastFileIter = dir->files.begin();
	while (xml->findChild("File")) {
		string name = xml->getChildAttrib("Name");
		u_int64_t size = xml->getLongLongChildAttrib("Size");

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

	xml->resetCurrentChild();

	while (xml->findChild("Directory")) {
		string name = xml->getChildAttrib("Name");
		string path = xml->getChildAttrib("Path");
		
		dir->directories[name] = addDirectoryFromXml(xml, dir, name, path);
	}


	xml->stepOut();

	return dir;
}

void ShareManager::saveXmlList(){
	RLock<> l(cs);
	string indent;
	FilteredOutputStream<BZFilter, true> *xmlFile = new FilteredOutputStream<BZFilter, true>(new File(Util::getAppPath() + "Share.xml.bz2", File::WRITE, File::TRUNCATE | File::CREATE));
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

bool ShareManager::isIncoming(const string& aDir) {
	string tmp = aDir;
	if( aDir[ aDir.length() -1 ] != PATH_SEPARATOR )
		tmp += PATH_SEPARATOR;

	StringBoolMapIter i = incomingMap.find(tmp);

	if(i == incomingMap.end())
		return false;
	
	return i->second;
}

void ShareManager::setIncoming( const string& aDir, bool incoming /*= true*/ ) {
	string tmp = aDir;
	if( aDir[ aDir.length() -1 ] != PATH_SEPARATOR )
		tmp += PATH_SEPARATOR;

	incomingMap[tmp] = incoming;
}

bool ShareManager::refresh( const string& aDir ){
	bool result = false;
	string path = Text::toLower(aDir);

	if(path[ path.length() -1 ] != PATH_SEPARATOR)
		path += PATH_SEPARATOR;

	RLock<> l(cs);

	//Directory::MapIter i = directories.find( path );
	Directory::MapIter i = directories.begin();
	for(; i != directories.end(); ++i) {
		if(Util::stricmp(i->first, path) == 0)
			break;
	}

	if( i == directories.end() ) {
		for( StringPairIter j = virtualMap.begin(); j != virtualMap.end(); ++j ){
			if( Util::stricmp( j->first, aDir ) == 0 ) {
				refreshPaths.push_back( j->second );
				result = true;
			}
		}
	} else {
		refreshPaths.push_back( path );
		result = true;
	}

	if(result)
		refresh(false, true, false, false, true);
	
	return result;
}

StringList ShareManager::getVirtualDirectories() {
	StringList result;

	RLock<> l(cs);

	for(StringPairIter i = virtualMap.begin(); i != virtualMap.end(); ++i){
		bool exists = false;

		for(StringIter j = result.begin(); j != result.end(); ++j) {
			if( Util::stricmp( *j, i->first ) == 0 ){
				exists = true;
				break;
			}
		}

		if( !exists )
			result.push_back( i->first );
	}

	sort( result.begin(), result.end() );

	return result;
}

/**
 * @file
 * $Id: ShareManager.cpp,v 1.9 2004/02/21 10:47:46 trem Exp $
 */

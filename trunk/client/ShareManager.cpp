/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#endif

#include <limits>

//just remove this to remove the hashing =)
#define USE_TTH

ShareManager::ShareManager() : hits(0), listLen(0), bzXmlListLen(0),
	xmlDirty(false), nmdcDirty(false), refreshDirs(false), update(false), listN(0), lFile(NULL), 
	xFile(NULL), lastXmlUpdate(0), lastNmdcUpdate(0), lastFullUpdate(GET_TICK()), bloom(1<<20),
	lastIncomingUpdate(GET_TICK())
{ 
	SettingsManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	HashManager::getInstance()->addListener(this);
	// Common search words used to make search more efficient, should be more dynamic
	words.push_back("avi");
	words.push_back("mp3");
	words.push_back("bin");
	words.push_back("zip");
	words.push_back("jpg");
	words.push_back("mpeg");
	words.push_back("mpg");
	words.push_back("rar");
	words.push_back("ace");
	words.push_back("bin");
	words.push_back("iso");
	words.push_back("dev");
	words.push_back("flt");
	words.push_back("ccd");
	words.push_back("txt");
	words.push_back("sub");
	words.push_back("nfo");
	words.push_back("wav");
	words.push_back("exe");
	words.push_back("ccd");

};

ShareManager::~ShareManager() {
	SettingsManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	HashManager::getInstance()->removeListener(this);

	join();
	
	saveXmlList();

	delete lFile;
	lFile = NULL;
	delete xFile;
	xFile = NULL;

	for(int i = 0; i <= listN; ++i) {
		File::deleteFile(Util::getAppPath() + "MyList" + Util::toString(i) + ".DcLst");
		File::deleteFile(Util::getAppPath() + "files" + Util::toString(i) + ".xml.bz2");
	}

	for(Directory::MapIter j = directories.begin(); j != directories.end(); ++j) {
		delete j->second;
	}
}

ShareManager::Directory::~Directory() {
	for(MapIter i = directories.begin(); i != directories.end(); ++i)
		delete i->second;
	
#ifdef USE_TTH
	for(File::Iter i = files.begin(); i != files.end(); ++i) {
		dcassert(i->getTTH() != NULL);
		ShareManager::getInstance()->removeTTH(i->getTTH(), i);
	}
#endif
}


string ShareManager::translateFileName(const string& aFile, bool adc) throw(ShareException) {
	RLock l(cs);
	if(aFile == "MyList.DcLst") {
		generateNmdcList();
		return getListFile();
	} else if(aFile == "files.xml.bz2") {
		generateXmlList();
		return getBZXmlFile();
	} else {
		string file;

		if(adc) {
			// Check for tth root identifier
			if(aFile.compare(0, 4, "TTH/") == 0) {
				TTHValue v(aFile.substr(4));
				HashFileIter i = tthIndex.find(&v);
				if(i != tthIndex.end()) {
					file = i->second->getADCPath();
				} else {
					throw ShareException("File Not Available");
				}
			} else if(aFile.compare(0, 1, "/") == 0) {
				file = aFile;
			} else {
				throw ShareException("File Not Available");
			}
			// Remove initial '/'
			file.erase(0, 1);

			// Change to NMDC path separators
			for(string::size_type i = 0; i < file.length(); ++i) {
				if(file[i] == '/') {
					file[i] = '\\';
				}
			}
			// Ok, we now should have an nmdc equivalent name
		} else {
			file = aFile;
		}

		string::size_type i = file.find('\\');
		if(i == string::npos)
			throw ShareException("File Not Available");
		
		string aDir = file.substr(0, i);
		file = file.substr(i + 1);

		RLock l(cs);

		bool found = false;

		StringPairIter j = virtualMap.begin();
		for(; j != virtualMap.end(); ++j) {
			if( Util::stricmp(aDir, j->first) == 0 ){
				if( checkFile(j->second, file) ){
					found = true;
					break;
				}
			}
		}

		if( !found )
			throw ShareException("File Not Available");
		
		return j->second + file;
	}
}

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

bool ShareManager::checkFile(const string& dir, const string& aFile) {

	Directory::MapIter mi = directories.find(dir);
	if(mi == directories.end())
		return false;
	Directory* d = mi->second;
								  
	string::size_type i;
	string::size_type j = 0;
	while( (i = aFile.find(PATH_SEPARATOR, j)) != string::npos) {
		dcdebug("dir=%s", aFile.substr(j, i-j).c_str() );
		mi = d->directories.find(aFile.substr(j, i-j));
		j = i + 1;
		if(mi == d->directories.end())
			return false;
		d = mi->second;
	}
	
	if(find_if(d->files.begin(), d->files.end(), Directory::File::StringComp(aFile.substr(j))) == d->files.end())
		return false;

	return true;
}

void ShareManager::load(SimpleXML* aXml) {
	WLock l(cs);

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
			Directory* dp = new Directory(newVirt);
			directories[d] = dp;
			virtualMap.push_back(make_pair(newVirt, d));
			setIncoming(d, aXml->getBoolChildAttrib("Incoming"));
		}
		aXml->stepOut();
	}
}

void ShareManager::save(SimpleXML* aXml) {
	RLock l(cs);
	
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

	Directory* dp = NULL;
	{
		RLock l(cs);


		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			if(Util::strnicmp(d, i->first, i->first.length()) == 0) {
				// Trying to share an already shared directory
				throw ShareException(STRING(DIRECTORY_ALREADY_SHARED));
			} else if(Util::strnicmp(d, i->first, d.length()) == 0) {
				// Trying to share a parent directory
				throw ShareException(STRING(REMOVE_ALL_SUBDIRECTORIES));
			}
		}

		dp = buildTree(d, NULL);
		dp->setName(aName);
	}
	{
		WLock l(cs);
		addTree(d, dp);

		directories[d] = dp;
		virtualMap.push_back(make_pair(aName, d));
		setDirty();
	}
}

void ShareManager::removeDirectory(const string& aDirectory) {
	WLock l(cs);

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
	setDirty();
}

int64_t ShareManager::getShareSize(const string& aDir) throw() {
	RLock l(cs);
	dcassert(aDir.size()>0);
	Directory::MapIter i = directories.find(aDir);

	if(i != directories.end()) {
		return i->second->getSize();
	}

	return -1;
}

int64_t ShareManager::getShareSize() throw() {
	RLock l(cs);
	int64_t tmp = 0;
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp += i->second->getSize();
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
void ShareManager::Directory::addSearchType(u_int32_t mask) throw() {
	if(!hasSearchType(mask)) {
		searchTypes |= mask;
		if(getParent() != NULL)
			getParent()->addSearchType(mask);
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

	DirData& operator*() { return data; }
	DirData* operator->() { return &data; }

	bool operator!=(const FileFindIter& rhs) const { return handle != rhs.handle; }
private:
	DirData data;
	HANDLE handle;
#else
public:
	// TODO...
	FileFindIter() { }
	FileFindIter(const string&) { }
	void operator++(int) { }
	struct DirData {
		string getFileName() { return Util::emptyString; }
		bool isDirectory() { return false; }
		bool isHidden() { return false; }
		int64_t getSize() { return 0; }
		u_int32_t getLastWriteTime { return 0; }
	};
#warn FIXME
#endif
};

ShareManager::Directory* ShareManager::buildTree(const string& aName, Directory* aParent) {
	Directory* dir = new Directory(Util::getLastDir(aName), aParent);
	dir->addType(SearchManager::TYPE_DIRECTORY); // needed since we match our own name in directory searches
	dir->addSearchType(getMask(dir->getName()));

	Directory::File::Iter lastFileIter = dir->files.begin();

	FileFindIter end;
	for(FileFindIter i(aName + "*"); i != end; ++i) {
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
				dir->addSearchType(dir->directories[name]->getSearchTypes()); 
			}
		} else {
			// Not a directory, assume it's a file...make sure we're not sharing the settings file...
			if( (Util::stricmp(name.c_str(), "DCPlusPlus.xml") != 0) && 
				(Util::stricmp(name.c_str(), "Favorites.xml") != 0) &&
				(Util::stricmp(Util::getFileExt(name).c_str(), ".dctmp") != 0) &&
				(Util::stricmp(Util::getFileExt(name).c_str(), ".antifrag") != 0) ){

				int64_t size = i->getSize();

#ifdef USE_TTH
				HashManager::getInstance()->checkTTH(aName + name, size, i->getLastWriteTime());
#endif

				lastFileIter = dir->files.insert(lastFileIter, Directory::File(name, size, dir, NULL));

			}
		}
	}

	return dir;
}

void ShareManager::addTree(const string& fullName, Directory* dir) {
	bloom.add(Text::toLower(dir->getName()));

	for(Directory::MapIter i = dir->directories.begin(); i != dir->directories.end(); ++i) {
		Directory* d = i->second;
		addTree(fullName + d->getName() + PATH_SEPARATOR, d);
	}

	for(Directory::File::Iter i = dir->files.begin(); i != dir->files.end(); ) {
		const Directory::File& f2 = *i;

		// We're not changing anything cruical...
		Directory::File& f = const_cast<Directory::File&>(f2);
		string fileName = fullName + f.getName();

#ifdef USE_TTH		
		f.setTTH(HashManager::getInstance()->getTTH(fileName));

		if(f.getTTH() != NULL) {
#endif
			addFile(dir, i++);
#ifdef USE_TTH
		} else {
			dir->files.erase(i++);
		}
#endif
	}
}

void ShareManager::addFile(Directory* dir, Directory::File::Iter i) {
	const Directory::File& f = *i;

	dir->size+=f.getSize();
	
	dir->addSearchType(getMask(f.getName()));
	dir->addType(getType(f.getName()));

#ifdef USE_TTH
	tthIndex.insert(make_pair(f.getTTH(), i));
#endif

	bloom.add(Text::toLower(f.getName()));
}

void ShareManager::removeTTH(TTHValue* tth, const Directory::File::Iter& iter) {
	pair<HashFileIter, HashFileIter> range = tthIndex.equal_range(tth);
	for(HashFileIter j = range.first; j != range.second; ++j) {
		if(j->second == iter) {
			tthIndex.erase(j);
			break;
		}
	}
}

void ShareManager::refresh(bool dirs /* = false */, bool aUpdate /* = true */, bool block /* = false */, 
						   bool incoming /* = false */, bool dir /* = false*/) throw(ShareException) {
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
				RLock l(cs);
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
				WLock l(cs);
				StringPairList dirs = virtualMap;
				for(StringIter j = refreshPaths.begin(); j != refreshPaths.end(); ++j){
					removeDirectory( *j );
				}
				virtualMap = dirs;

				for(Directory::MapIter i = newDirs.begin(); i != newDirs.end(); ++i) {
					addTree(i->first, i->second);
					directories.insert(*i);
				}
			}

			refreshDir = false;
			refreshPaths.clear();
		}

		if(refreshIncoming && !refreshDirs) {
			lastIncomingUpdate = GET_TICK();
			Directory::Map newDirs;
			{
				RLock l(cs);
				for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
					if(isIncoming(i->first)) {
						Directory* dp = buildTree(i->first, NULL);
						dp->setName(findVirtual(i->first)->first);
						newDirs.insert(make_pair(i->first, dp));
					}
				}
			}
			{
				WLock l(cs);
				StringPairList dirs = virtualMap;
				for(StringPairIter i = dirs.begin(); i != dirs.end(); ++i) {
					if(isIncoming(i->second))
						removeDirectory(i->second);
				}
				
				virtualMap = dirs;

				for(Directory::MapIter i = newDirs.begin(); i != newDirs.end(); ++i) {
					addTree(i->first, i->second);
					directories.insert(*i);
				}
			}
				refreshIncoming = false;
		}

		if(refreshDirs) {
			lastFullUpdate = GET_TICK();
			
			Directory::Map newDirs;
			{
				RLock l(cs);
				for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
					Directory* dp = buildTree(i->first, NULL);
					dp->setName(findVirtual(i->first)->first);
					newDirs.insert(make_pair(i->first, dp));
				}
			}
			{
				WLock l(cs);
				StringPairList dirs = virtualMap;
				for(StringPairIter i = dirs.begin(); i != dirs.end(); ++i) {
					removeDirectory(i->second);
				}
				bloom.clear();

				virtualMap = dirs;

				for(Directory::MapIter i = newDirs.begin(); i != newDirs.end(); ++i) {
					addTree(i->first, i->second);
					directories.insert(*i);
				}
			}
			refreshDirs = false;
		}
	}

	generateXmlList(true);
	generateNmdcList(true);

	LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_FINISHED));
	if(update) {
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}

void ShareManager::generateXmlList(bool force /* = false */ ) {
	if(xmlDirty && (lastXmlUpdate + 15 * 60 * 1000 < GET_TICK() || force ) ) {
		listN++;

		try {
			SimpleXML *xml = new SimpleXML();
			string tmp2;

			xml->addTag("FileListing");
			xml->addChildAttrib("Version", 1);
			xml->addChildAttrib("Generator", string(APPNAME " " VERSIONSTRING));
			xml->stepIn();

			for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
				i->second->toXml(xml, tmp2);
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
				File::copyFile(newXmlName, Util::getAppPath() + "files.xml.bz2");
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
	if(nmdcDirty && (lastNmdcUpdate + 15 * 60 * 1000 < GET_TICK() || force) ) {
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

void ShareManager::Directory::toXml(SimpleXML* xml, string& tmp2) {
	bool create = true;
	xml->resetCurrentChild();

	while( xml->findChild("Directory") ){
		if( Util::stricmp(xml->getChildAttrib("Name"), name) == 0 ){
			create = false;
			break;	
		}
	}

	if(create) {
		xml->addTag("Directory", true);
		xml->addChildAttrib("Name", name);
	}

	xml->stepIn();

	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		i->second->toXml(xml, tmp2);
	}

	for(Directory::File::Iter i = files.begin(); i != files.end(); ++i) {
		const Directory::File& f = *i;

		xml->addTag("File");
		xml->addChildAttrib("Name", f.getName() );
		xml->addChildAttrib("Size", f.getSize() );
		if(f.getTTH())
			xml->addChildAttrib("TTH", f.getTTH()->toBase32() );
	}

	xml->stepOut();
		
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
 * The mask is a set of bits that say which words a file matches. Each bit means
 * that a fileName matches the word at position n-1 in the words list where n is
 * the bit number. bit 0 is only set when no words match.
 */
u_int32_t ShareManager::getMask(const string& fileName) {
	u_int32_t mask = 0;
	int n = 1;
	for(StringIter i = words.begin(); i != words.end(); ++i, n++) {
		if(Util::findSubString(fileName, *i) != string::npos) {
			mask |= (1 << n);
		}
	}
	return (mask == 0) ? 1 : mask;
}

u_int32_t ShareManager::getMask(StringList& l) {
	u_int32_t mask = 0;
	int n = 1;

	for(StringIter i = words.begin(); i != words.end(); ++i, n++) {
		for(StringIter j = l.begin(); j != l.end(); ++j) {
			if(Util::findSubString(*j, *i) != string::npos) {
				mask |= (1 << n);
			}
		}
	}
	return (mask == 0) ? 1 : mask;	
}

u_int32_t ShareManager::getMask(StringSearch::List& l) {
	u_int32_t mask = 0;
	int n = 1;

	for(StringIter i = words.begin(); i != words.end(); ++i, n++) {
		for(StringSearch::Iter j = l.begin(); j != l.end(); ++j) {
			if(Util::findSubString(j->getPattern(), *i) != string::npos) {
				mask |= (1 << n);
			}
		}
	}
	return (mask == 0) ? 1 : mask;	
}

/**
 * Alright, the main point here is that when searching, a search string is most often found in 
 * the filename, not directory name, so we want to make that case faster. Also, we want to
 * avoid changing StringLists unless we absolutely have to --> this should only be done if a string
 * has been matched in the directory name. This new stringlist should also be used in all descendants,
 * but not the parents...
 */
void ShareManager::Directory::search(SearchResult::List& aResults, StringSearch::List& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults, u_int32_t mask) throw() {
	// Skip everything if there's nothing to find here (doh! =)
	if(!hasType(aFileType))
		return;

	if(!hasSearchType(mask))
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
			u_int32_t xmask = ShareManager::getInstance()->getMask(k->getPattern());
			if(xmask != 1) {
				mask &= ~xmask;
			}
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
					i->getSize(), getFullName() + i->getName(), i->getTTH(), true);
				aResults.push_back(sr);
				ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
				if(aResults.size() >= maxResults) {
					break;
				}
			}
		}
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, *cur, aSearchType, aSize, aFileType, aClient, maxResults, mask);
	}
}

void ShareManager::search(SearchResult::List& results, const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) {
	RLock l(cs);
	if(aFileType == SearchManager::TYPE_HASH) {
		if(aString.compare(0, 4, "TTH:") == 0) {
			TTHValue tth(aString.substr(4));
			
			pair< HashFileIter, HashFileIter> iter = tthIndex.equal_range(&tth);
			for(; iter.first != iter.second; ++(iter.first) ){
				dcassert(iter.first->second->getTTH() != NULL);

				SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_FILE, 
					iter.first->second->getSize(),
					iter.first->second->getParent()->getFullName() + iter.first->second->getName(), 
					iter.first->second->getTTH(), true);

				results.push_back(sr);
				ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
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
	u_int32_t mask = getMask(sl);

	for(Directory::MapIter j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
		j->second->search(results, ssl, aSearchType, aSize, aFileType, aClient, maxResults, mask);
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
		if(toCode('T', 'L') == cmd) {
			hasRoot = true;
			root = p.substr(2);
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

void ShareManager::Directory::search(SearchResult::List& aResults, AdcSearch& aStrings, Client* aClient, StringList::size_type maxResults, u_int32_t mask) throw() {
	if(!hasSearchType(mask))
		return;

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
			u_int32_t xmask = ShareManager::getInstance()->getMask(k->getPattern());
			if(xmask != 1) {
				mask &= ~xmask;
			}
		}
	}

	if(newStr.get() != 0) {
		cur = newStr.get();
	}

	bool sizeOk = (aStrings.gt == 0);
	if( cur->empty() && aStrings.ext.empty() && sizeOk ) {
		// We satisfied all the search words! Add the directory...
		SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_DIRECTORY, 
			0, getFullName(), NULL, true);
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

				SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_FILE, 
					i->getSize(), getFullName() + i->getName(), i->getTTH(), true);
				aResults.push_back(sr);
				ShareManager::getInstance()->addHits(1);
				if(aResults.size() >= maxResults) {
					return;
				}
			}
		}
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, aStrings, aClient, maxResults, mask);
	}
	aStrings.include = old;
}

void ShareManager::search(SearchResult::List& results, const StringList& params, Client* aClient, StringList::size_type maxResults) {
	AdcSearch srch(params);	

	RLock l(cs);

	if(srch.hasRoot) {
		HashFileIter i = tthIndex.find(&srch.root);
		if(i != tthIndex.end()) {
			dcassert(i->second->getTTH() != NULL);

			SearchResult* sr = new SearchResult(aClient,
				SearchResult::TYPE_FILE, i->second->getSize(), i->second->getParent()->getFullName() + i->second->getName(), 
				i->second->getTTH(), true);
			results.push_back(sr);
			ShareManager::getInstance()->addHits(1);
		}
		return;
	}

	for(StringSearch::Iter i = srch.includeX.begin(); i != srch.includeX.end(); ++i) {
		if(!bloom.match(i->getPattern()))
			return;
	}

	u_int32_t mask = getMask(srch.includeX);

	for(Directory::MapIter j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
		j->second->search(results, srch, aClient, maxResults, mask);
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
		WLock l(cs);
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

void ShareManager::on(HashManagerListener::TTHDone, const string& fname, TTHValue* root) throw() {
	WLock l(cs);
	Directory* d = getDirectory(fname);
	if(d != NULL) {
		Directory::File::Iter i = find_if(d->files.begin(), d->files.end(), Directory::File::StringComp(Util::getFileName(fname)));
		if(i != d->files.end()) {
			if(i->getTTH() != NULL) { // TTH of file updated?
				dcassert(tthIndex.find(i->getTTH()) != tthIndex.end());
				removeTTH(i->getTTH(), i);
			}
			// Get rid of false constness...
			Directory::File* f = const_cast<Directory::File*>(&(*i));
			f->setTTH(root);
			tthIndex.insert(make_pair(root, i));
		} else {
			string name = Util::getFileName(fname);
			int64_t size = File::getSize(fname);
			Directory::File::Iter it = d->files.insert(Directory::File(name, size, d, root)).first;
			addFile(d, it);
		}
	}
}

void ShareManager::on(TimerManagerListener::Minute, u_int32_t tick) throw() {
	if(BOOLSETTING(AUTO_UPDATE_LIST)) {
		if(lastFullUpdate + SETTING(SHARE_REFRESH_TIME) * 60 * 1000 < tick) {
			try {
				refresh(true, true);
			} catch(const ShareException&) {
			}
		}
	}
	if(BOOLSETTING(AUTO_UPDATE_INCOMING)) {
		if(lastIncomingUpdate + SETTING(INCOMING_REFRESH_TIME) * 60 * 1000 < tick) {
			try{
				refresh(false, true, false, true);
			} catch(const ShareException&){
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
		//if we for some reason failed, return false to indicate that a refresh is needed
		return false;
	}


	//cleanup
	delete[] buf;
	buf = NULL;

	//same here =)
	if(xmlString->empty()){
		delete xmlString;
		return false;
	}

	bool result = true;

	SimpleXML *xml = NULL;

	try{
		xml = new SimpleXML();
		//convert the xml string to an xml object
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

		WLock l(cs);

		while (xml->findChild("Directory")) {
			string name = xml->getChildAttrib("Name");
			string path = xml->getChildAttrib("Path");
			if(path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;
			
			Directory *d = addDirectoryFromXml(xml, NULL, name, path); 
			addTree(path, d);
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
	dir->addSearchType(getMask(dir->getName()));
	bloom.add(Text::toLower(dir->getName()));

	xml->stepIn();

	Directory::File::Iter lastFileIter = dir->files.begin();
	while (xml->findChild("File")) {
		string name = xml->getChildAttrib("Name");
		u_int64_t size = xml->getIntChildAttrib("Size");

#ifdef USE_TTH
		if( aPath[ aPath.length() -1 ] == PATH_SEPARATOR )
			HashManager::getInstance()->checkTTH(aPath + name, size);
		else
			HashManager::getInstance()->checkTTH(aPath + PATH_SEPARATOR + name, size);
#endif
		lastFileIter = dir->files.insert(lastFileIter, Directory::File(name, size, dir, NULL));
	}

	xml->resetCurrentChild();

	while (xml->findChild("Directory")) {
		string name = xml->getChildAttrib("Name");
		string path = xml->getChildAttrib("Path");
		
		dir->directories[name] = addDirectoryFromXml(xml, dir, name, path);
		dir->addSearchType(dir->directories[name]->getSearchTypes());
	}


	xml->stepOut();

	return dir;
}

void ShareManager::saveXmlList(){
	RLock l(cs);
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

	try{
		delete xmlFile;
		xmlFile = NULL;
	}catch (Exception&) {
	}

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
	string path = aDir;

	if(path[ path.length() -1 ] != PATH_SEPARATOR)
		path += PATH_SEPARATOR;

	RLock l(cs);

	Directory::MapIter i = directories.find( path );

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

	refresh(false, true, false, false, true);
	
	return result;
}

StringList ShareManager::getVirtualDirectories() {
	StringList result;

	RLock l(cs);

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


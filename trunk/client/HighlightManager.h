#ifndef HIGHLIGHTMANAGER_H
#define HIGHLIGHTMANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SettingsManager.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "ColorSettings.h"
#include "SimpleXML.h"

typedef vector< ColorSettings* > ColorList;
typedef ColorList::iterator ColorIter;

class HighlightManager : public Singleton<HighlightManager>, private SettingsManagerListener
{
public:
	HighlightManager(void);
	~HighlightManager(void);

	ColorList*	rLock();
	ColorList*	wLock();
	void		rUnlock();
	void		wUnlock();
private:
	//store all highlights
	ColorList colorSettings;

	RWLock lock;

	void load(SimpleXML *aXml);
	void save(SimpleXML *aXml);

	virtual void onAction(SettingsManagerListener::Types type, SimpleXML* xml) throw();
};

#endif

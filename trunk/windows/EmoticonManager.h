#ifndef EMOTICONMANAGER_H
#define EMOTICONMANAGER_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/Singleton.h"
#include "../client/SimpleXML.h"
#include "../client/Util.h"

#include <GdiPlus.h>
//#include <GdiPlusMetaFile.h>

class EmoticonManager : public Singleton<EmoticonManager>
{
public:
	EmoticonManager(void) {};
	~EmoticonManager(void){};
		

	void load(HWND aWnd) {
		wnd = aWnd;
		try {
			File f(Util::getAppPath() + "Emoticons.xml", File::READ, File::OPEN);
			string buf = f.read();
			f.close();
			
			if(buf.empty())
				return;


			SimpleXML xml;
			xml.fromXML(buf);
			xml.resetCurrentChild();
			xml.stepIn();

			string pattern, file;
			while(xml.findChild("Emotion")){
				pattern = xml.getChildAttrib("ReplacedText");
				file = xml.getChildAttrib("BitmapPath");

				CreateRtf(pattern, Util::getAppPath() + file);
			}
		} catch (FileException &e ){
			//...
		} catch (SimpleXMLException &e ) {
			//...
		}
	}
	StringMap* getEmoticons() {
		return &emoticons;
	}
private:
	StringMap emoticons;
	HWND wnd;
	
	void CreateRtf(string& pattern, string& path){
		//path = "f:\\temp\\metafile.emf";
		//WCHAR* tmp = new WCHAR[1024];
		//int s = MultiByteToWideChar(CP_ACP, 0, path.c_str(), path.length(), tmp, 1024);
		//tmp[s] = '\0';
		//Gdiplus::Image* img = new Gdiplus::Image(tmp);

		//delete[] tmp;

		HBITMAP bmp = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		//Get a device handle from mainframe window
		HDC winDC = GetDC(NULL);
		HDC memDC = CreateCompatibleDC(winDC);
		int iWidthMM = GetDeviceCaps(winDC, HORZSIZE); 
		int iHeightMM = GetDeviceCaps(winDC, VERTSIZE); 
		int iWidthPels = GetDeviceCaps(winDC, HORZRES); 
		int iHeightPels = GetDeviceCaps(winDC, VERTRES); 

		
		BITMAP bm;
		GetObject(bmp, sizeof(bm), &bm);
		CRect rc(0, 0, bm.bmWidth, bm.bmHeight);
		rc.right = (rc.right * iWidthMM * 100)/iWidthPels; 
		rc.bottom = (rc.bottom * iHeightMM * 100)/iHeightPels; 
		

		//::SelectObject(winDC, bmp);

		//Gdiplus::Metafile metafile/* = new Gdiplus::Metafile*/(winDC);//, rc, (Gdiplus::MetafileFrameUnit)7, (Gdiplus::EmfType)5, NULL);
		HDC metaDC = CreateEnhMetaFile(memDC, 0, rc, 0);
		::SelectObject(metaDC, bmp);
		HENHMETAFILE hEmf = CloseEnhMetaFile(metaDC);
		
		//Gdiplus::Bitmap bit(bmp, NULL);
		//Gdiplus::Graphics* g;
		//g = Gdiplus::Graphics::FromImage(&metafile);
		//Gdiplus::RectF rc(0, 0, img->GetWidth(), img->GetHeight());
		
		//g->DrawImage(&bit, 0, 0);
		

		//get the image size before releasing the hdc
		//int picw	 = GetDeviceCaps(winDC, HORZSIZE)*100;
		//int pich	 = GetDeviceCaps(winDC,	VERTSIZE)*100;
		//int picwgoal = GetDeviceCaps(winDC, LOGPIXELSX)*1440;
		//int pichgoal = GetDeviceCaps(winDC, LOGPIXELSY)*1440;
		
		//HENHMETAFILE hEmf = metafile->GetHENHMETAFILE();
		UINT size = 0;
		//size = Gdiplus::Metafile::EmfToWmfBits(hEmf, 0, NULL, MM_ANISOTROPIC, 0);
		size = GetWinMetaFileBits(hEmf, 0, NULL, MM_ANISOTROPIC, memDC);

		BYTE* buf = new BYTE[size];
		
		size = GetWinMetaFileBits(hEmf, size, buf, MM_ANISOTROPIC, memDC);
		//Gdiplus::Metafile::EmfToWmfBits(hEmf, size, buf, MM_ANISOTROPIC, 0);
		
		
		//create the string so it's ready to be inserted into the richedit
		string tmp = "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1053{\\fonttbl{\\f0\\fnil\\fcharset0 Microsoft Sans Serif;}}{\\pict\\wmetafile24\\picw" + 
			Util::toString(iWidthMM) + "\\pich" + Util::toString(iHeightMM) + 
			"\\picwgoal" + Util::toString(rc.right) +  "\\pichgoal" + 
			Util::toString(rc.bottom) +	" " + toHex(buf, size) + "}\\par }";
			
		emoticons[pattern] = tmp;
		//cleanup
		delete[] buf;
		//return the handle to avoid mem leaks
		ReleaseDC(wnd, winDC);
		DeleteDC(memDC);
		DeleteEnhMetaFile(hEmf);

		//delete metafile;
		//delete g;

		//DeleteDC(metaDC);
	}

	string toHex(LPBYTE buf, int size) {
		char values[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		int tmpSize = size*2 + 5;
		char* tmp = new char[tmpSize];

		for(int i = 0, j = 0; i <= size; ++i){
			int k = (*buf) % 16;
			tmp[j++] = values[((*buf)-k) / 16];
			tmp[j++] = values[k];
			++buf;
		}
		tmp[j] = 0;

		string t(tmp);
		delete[] tmp;
		return t;
	}
};
#endif
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
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

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

				CreateRtfBitmap(pattern, Util::getAppPath() + file);
				//CreateRtfMetafile(pattern, Util::getAppPath() + file);
			}
		} catch (FileException &e ){
			//...
		} catch (SimpleXMLException &e ) {
			//...
		}

		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
	StringMap* getEmoticons() {
		return &emoticons;
	}
private:
	StringMap emoticons;
	HWND wnd;
	
	void CreateRtfMetafile(string& pattern, string& path){
		HBITMAP bmp = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		//Get a device handle from mainframe window
		HDC winDC = GetDC(wnd);
		HDC memDC = CreateCompatibleDC(winDC);
		//HDC memDC = CreateCompatibleDC(winDC);
		int iWidthMM = GetDeviceCaps(winDC, HORZSIZE); 
		int iHeightMM = GetDeviceCaps(winDC, VERTSIZE); 
		int iWidthPels = GetDeviceCaps(winDC, HORZRES); 
		int iHeightPels = GetDeviceCaps(winDC, VERTRES); 
		float iDpiY =	  GetDeviceCaps(winDC, LOGPIXELSY);
		float iDpiX =	  GetDeviceCaps(winDC, LOGPIXELSX);

		
		BITMAP bm;
		GetObject(bmp, sizeof(bm), &bm);
		CRect rc(0, 0, bm.bmWidth, bm.bmHeight);
		rc.right = (rc.right * iWidthMM * 100)/iWidthPels; 
		rc.bottom = (rc.bottom * iHeightMM * 100)/iHeightPels; 

		int picw	 = (bm.bmWidth  / iDpiX) * 2540;
		int pich	 = (bm.bmHeight / iDpiY) * 2540;
		int picwgoal = (bm.bmWidth  / iDpiX) * 1440;
		int pichgoal = (bm.bmHeight / iDpiY) * 1440;
		

		::SelectObject(memDC, bmp);
        HDC metaDC = CreateEnhMetaFile(winDC, 0, NULL, 0);
		SetMapMode(metaDC, MM_ANISOTROPIC);
		BitBlt(metaDC, 0, 0, bm.bmWidth, bm.bmHeight, memDC, 0, 0, SRCCOPY);
		HENHMETAFILE hEmf = CloseEnhMetaFile(metaDC);
		
		UINT size = 0;
		size = GetWinMetaFileBits(hEmf, 0, NULL, MM_ANISOTROPIC, metaDC);
		
		BYTE* buf = new BYTE[size];
		size = GetWinMetaFileBits(hEmf, size, buf, MM_ANISOTROPIC, metaDC);
		
				
		//create the string so it's ready to be inserted into the richedit
		string tmp = "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1053{\\fonttbl{\\f0\\fnil\\fcharset0 Microsoft Sans Serif;}}{\\pict\\wmetafile8\\picw" + 
			Util::toString(picw) + "\\pich" + Util::toString(pich) + 
			"\\picwgoal" + Util::toString(picwgoal) +  "\\pichgoal" + 
			Util::toString(pichgoal) +	" " + toHex(buf, size);
			
		int sz = tmp.length();
		emoticons[pattern] = tmp + "}\\par}";
		//cleanup
		delete[] buf;
		//return the handle to avoid mem leaks
		ReleaseDC(wnd, winDC);
		DeleteDC(memDC);
		DeleteDC(metaDC);
		DeleteEnhMetaFile(hEmf);
	}

	void CreateRtfBitmap(string& pattern, string& path) {
		HBITMAP bmp = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		WCHAR* wPath = new WCHAR[512];
		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wPath, 512);
		Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(wPath);

		HDC winDC = GetDC(NULL);
		int iWidthMM = GetDeviceCaps(winDC, HORZSIZE); 
		int iHeightMM = GetDeviceCaps(winDC, VERTSIZE); 
		int iWidthPels = GetDeviceCaps(winDC, HORZRES); 
		int iHeightPels = GetDeviceCaps(winDC, VERTRES); 
		float iDpiY =	  GetDeviceCaps(winDC, LOGPIXELSY);
		float iDpiX =	  GetDeviceCaps(winDC, LOGPIXELSX);


		BITMAP bm;
		GetObject(bmp, sizeof(bm), &bm);
		CRect rc(0, 0, bm.bmWidth, bm.bmHeight);
		rc.right = (rc.right * iWidthMM * 100)/iWidthPels; 
		rc.bottom = (rc.bottom * iHeightMM * 100)/iHeightPels; 

		int picw	 = (bm.bmWidth  / iDpiX) * 2540;
		int pich	 = (bm.bmHeight / iDpiY) * 2540;
		int picwgoal = (bm.bmWidth  / iDpiX) * 1440;
		int pichgoal = (bm.bmHeight / iDpiY) * 1440;


		//HDC metaDC = CreateEnhMetaFile(winDC, 0, NULL, 0);
		//SetMapMode(metaDC, MM_ANISOTROPIC);
		//::SelectObject(metaDC, bmp);
		//Gdiplus::Graphics graphics(metaDC);
		Gdiplus::Metafile metafile(winDC);

		Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromImage(&metafile);

		graphics->DrawImage(bitmap, 0, 0, bm.bmWidth, bm.bmHeight);

		//HENHMETAFILE hEmf = CloseEnhMetaFile(metaDC);
		HENHMETAFILE hEmf = metafile.GetHENHMETAFILE();

		UINT size = 0;
		//size = GetWinMetaFileBits(hEmf, 0, NULL, MM_ANISOTROPIC, winDC);
		size = Gdiplus::Metafile::EmfToWmfBits(hEmf, 0, NULL);
		

		BYTE* buf = new BYTE[size];
		//size = GetWinMetaFileBits(hEmf, size, buf, MM_ANISOTROPIC, winDC);
		size = Gdiplus::Metafile::EmfToWmfBits(hEmf, size, buf);
		


		//create the string so it's ready to be inserted into the richedit
		string tmp = "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1053{\\fonttbl{\\f0\\fnil\\fcharset0 Microsoft Sans Serif;}}{\\pict\\wmetafile8\\picw" + 
			Util::toString(picw) + "\\pich" + Util::toString(pich) + 
			"\\picwgoal" + Util::toString(picwgoal) +  "\\pichgoal" + 
			Util::toString(pichgoal) +	" " + toHex(buf, size) + "}";

		emoticons[pattern] = tmp;
		int sz = tmp.length();
		//cleanup
		delete[] buf;
		delete bitmap;
		delete graphics;
		//return the handle to avoid mem leaks
		ReleaseDC(wnd, winDC);
		//DeleteDC(metaDC);
		DeleteEnhMetaFile(hEmf);

		
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
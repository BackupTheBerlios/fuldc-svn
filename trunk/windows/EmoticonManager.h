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
#include <objidl.h>
#include <Objbase.h>

using namespace Gdiplus;

class EmoticonManager : public Singleton<EmoticonManager>
{
public:
	EmoticonManager(void) {};
	~EmoticonManager(void){};
		

	void load(HWND aWnd) {
		/*GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

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

		GdiplusShutdown(gdiplusToken);
		*/
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
		float iDpiY =	  (float)GetDeviceCaps(winDC, LOGPIXELSY);
		float iDpiX =	  (float)GetDeviceCaps(winDC, LOGPIXELSX);

		
		BITMAP bm;
		GetObject(bmp, sizeof(bm), &bm);
		CRect rc(0, 0, bm.bmWidth, bm.bmHeight);
		rc.right = (rc.right * iWidthMM * 100)/iWidthPels; 
		rc.bottom = (rc.bottom * iHeightMM * 100)/iHeightPels; 

		int picw	 = (int)(bm.bmWidth  / iDpiX) * 2540;
		int pich	 = (int)(bm.bmHeight / iDpiY) * 2540;
		int picwgoal = (int)(bm.bmWidth  / iDpiX) * 1440;
		int pichgoal = (int)(bm.bmHeight / iDpiY) * 1440;
		

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
		WCHAR* wPath = new WCHAR[512];
		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wPath, 512);
		Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(wPath, true);
		
		HDC winDC = GetDC(NULL);
		HDC memDC = CreateCompatibleDC(winDC);
				
		float iDpiY     = (float)GetDeviceCaps(winDC, LOGPIXELSY);
		float iDpiX     = (float)GetDeviceCaps(winDC, LOGPIXELSX);
		int iWidthMM    = GetDeviceCaps(winDC, HORZSIZE) * 100; 
		int iHeightMM   = GetDeviceCaps(winDC, VERTSIZE) * 100; 
		int iWidthPels  = GetDeviceCaps(winDC, HORZRES); 
		int iHeightPels = GetDeviceCaps(winDC, VERTRES); 

		RECT rc;
		rc.top = 0;
		rc.left = 0;
		rc.right  = (bitmap->GetWidth()  * iWidthMM  ) / iWidthPels;
		rc.bottom = (bitmap->GetHeight() * iHeightMM ) / iHeightPels;


		int picw	 = int(((bitmap->GetWidth()  / iDpiX) * 2540.0) + 0.5);
		int pich	 = int(((bitmap->GetHeight() / iDpiY) * 2540.0) + 0.5);
		int picwgoal = int(((bitmap->GetWidth()  / iDpiX) * 1440.0) + 0.5);
		int pichgoal = int(((bitmap->GetHeight() / iDpiY) * 1440.0) + 0.5);


		HDC metaDC = CreateEnhMetaFile(NULL, NULL, &rc, NULL);
		//SetMapMode(metaDC, MM_ANISOTROPIC);
		//Metafile* meta = new Metafile(winDC, RectF(0, 0, bitmap->GetWidth(), bitmap->GetHeight()),
		//	MetafileFrameUnitPixel, EmfTypeEmfPlusDual);
		
		Graphics* graphics = new Graphics(metaDC);
		//Graphics* graphics = Graphics::FromImage(meta);
		
		graphics->DrawImage(bitmap, 0, 0, bitmap->GetWidth(), bitmap->GetHeight());
			
		HENHMETAFILE hEmf = CloseEnhMetaFile(metaDC);
		//HENHMETAFILE hEmf = meta->GetHENHMETAFILE();


		UINT size = 0;
		size = Metafile::EmfToWmfBits(hEmf, 0, NULL, MM_ANISOTROPIC, EmfToWmfBitsFlagsDefault);
		//size = GetWinMetaFileBits(hEmf, 0, NULL, MM_ANISOTROPIC, metaDC);

		

		BYTE* buf = new BYTE[size];
		size = Metafile::EmfToWmfBits(hEmf, size, buf, MM_ANISOTROPIC, EmfToWmfBitsFlagsDefault);
		//size = GetWinMetaFileBits(hEmf, size, buf, MM_ANISOTROPIC, metaDC);
		


		//create the string so it's ready to be inserted into the richedit
		//\\viewkind4\\uc1\\pard\\f0\\fs17
		string tmp = "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033{\\fonttbl{\\f0\\\\fnil\\fcharset0 Microsoft Sans Serif;}}{\\pict\\wmetafile8\\picw" + 
			Util::toString(picw) + "\\pich" + Util::toString(pich) + 
			"\\picwgoal" + Util::toString(picwgoal) +  "\\pichgoal" + 
			Util::toString(pichgoal) +	" " + toHex(buf, size) + "}";

		
		//File e("f:\\temp\\emotest.txt", File::READ, File::OPEN);
		//emoticons[pattern] = e.read(8096);
		//e.close();
		emoticons[pattern] = tmp;
		File f("f:\\temp\\testemot.txt", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(tmp);
		f.flush();
		f.close();
		//cleanup
		delete[] buf;
		delete bitmap;
		delete graphics;
		//return the handle to avoid mem leaks
		ReleaseDC(wnd, winDC);
		DeleteDC(memDC);
		DeleteEnhMetaFile(hEmf);

		
	}

	string toHex(LPBYTE buf, int size) {
		char values[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		int tmpSize = size*2 + 5;
		char* tmp = new char[tmpSize];

		for(int i = 0, j = 0; i < size; ++i){
			//int k = (*buf) % 16;
			//tmp[j++] = values[((*buf)-k) / 16];
			//tmp[j++] = values[k];
			tmp[j++] = values[(*buf) >> 4];
			tmp[j++] = values[(*buf) & 0x0F];
			++buf;
		}
		tmp[j] = 0;

		string t(tmp);
		delete[] tmp;
		return t;
	}
};
#endif
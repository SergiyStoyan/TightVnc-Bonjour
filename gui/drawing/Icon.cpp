// Copyright (C) 2009,2010,2011,2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the CisteraVNC software.  Please visit our Web site:
//
//                       http://www.cistera.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "Icon.h"

Icon::Icon()
: m_hasOwnIcon(true), m_icon(NULL)
{
}

Icon::Icon(HICON icon)
: m_hasOwnIcon(true), m_icon(icon)
{
}

Icon::Icon(Bitmap *bitmap)
: m_hasOwnIcon(true), m_icon(NULL)
{
  Bitmap mask(bitmap->getWidth(), bitmap->getHeight());
  fromBitmap(bitmap, &mask);
}

Icon::Icon(Bitmap *bitmap, Bitmap *mask)
: m_hasOwnIcon(true), m_icon(NULL)
{
  fromBitmap(bitmap, mask);
}

Icon::Icon(DWORD icon)
: m_hasOwnIcon(false)
{
  HINSTANCE hInstance = GetModuleHandle(NULL);
  m_icon = LoadIcon(hInstance, MAKEINTRESOURCE(icon));
}

Icon::~Icon()
{
  if (m_hasOwnIcon) {
    DestroyIcon(m_icon);
  }
}

HICON Icon::getHICON()
{
  return m_icon;
}

void Icon::fromBitmap(Bitmap *bitmap, Bitmap *mask)
{
  ICONINFO ii;

  memset(&ii, 0, sizeof(ICONINFO));

  ii.hbmColor = (bitmap != 0) ? bitmap->m_bitmap : 0;
  ii.hbmMask = (mask != 0) ? mask->m_bitmap : 0;

  m_icon = CreateIconIndirect(&ii);
}

void Icon::Blend(float factorA, float factorR, float factorG, float factorB)
{//BUGGY!!! NEEDS FIXING
	ICONINFO iconinfo;
	GetIconInfo(m_icon, &iconinfo);
	HBITMAP hBitmap = iconinfo.hbmColor;
	
	HDC hdc, hdcMem;
	hdc = GetDC(NULL);
	hdcMem = CreateCompatibleDC(hdc);
	BITMAPINFO MyBMInfo = { 0 };
	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);
	// Get the BITMAPINFO structure from the bitmap
	if (0 == GetDIBits(hdcMem, hBitmap, 0, 16, NULL, &MyBMInfo, DIB_RGB_COLORS))
		throw exception("FAIL");
	BYTE* bs = new BYTE[MyBMInfo.bmiHeader.biSizeImage];
	//MyBMInfo.bmiHeader.biBitCount = 32;
	//MyBMInfo.bmiHeader.biCompression = BI_RGB;
	//MyBMInfo.bmiHeader.biHeight = (MyBMInfo.bmiHeader.biHeight < 0) ? (-MyBMInfo.bmiHeader.biHeight) : (MyBMInfo.bmiHeader.biHeight);
	//if (0 == GetDIBits(hdcMem, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, (LPVOID)bs, &MyBMInfo, DIB_RGB_COLORS))
	//	throw exception("FAIL");
	
	for (int i = 0; i < 16 * 4; i += 4) 
	{
		bs[i] = (BYTE)(factorB * (float)bs[i]);//blue
		bs[i + 1] = (BYTE)(factorG * (float)bs[i+1]);//green
		bs[i + 2] = (BYTE)(factorR * (float)bs[i+2]);//red
		bs[i + 3] = (BYTE)(factorA * (float)bs[i+3]);//alpha
	}

	if (0 == SetDIBits(hdcMem, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, (LPVOID)bs, &MyBMInfo, DIB_RGB_COLORS))
		throw exception("FAIL");
	
	Bitmap* bitmap = new Bitmap(hBitmap);
	Bitmap mask(bitmap->getWidth(), bitmap->getHeight());
	fromBitmap(bitmap, &mask);
}
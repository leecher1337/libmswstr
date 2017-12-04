#ifndef __MSWSTR_H__
#define __MSWSTR_H__

#include "wintypes.h"
#include "winnls.h"

int WINAPI DBLCMapStringW(LCID Locale, int dwFlags, WCHAR *lpSrcString, signed int cchSrc, LPBYTE pDest, int cchDest);

#endif

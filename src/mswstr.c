#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include "wintypes.h"
#endif
#include <winnls.h>

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "nls.h"
#include "nlstables.h"

#ifdef WIN32
#define NlsStrLenW lstrlenW             // Hack
#else
#define NlsStrLenW wcslen
#endif


//
//  Invalid weight value.
//
#define MAP_INVALID_UW  0xffff  

#define NUM_BYTES_UW		2
#define NUM_BYTES_DW		1
#define NUM_BYTES_CW		1
#define NUM_BYTES_XW		4 

#define NORM_DROP_CW         (NORM_IGNORECASE | NORM_IGNOREWIDTH) 

#define EXTRA_WEIGHT_POS(WtNum)        (*(pPosXW + (WtNum * WeightLen)))

#define SPECIAL_CASE_HANDLER( SM,                                           \
                              pWeight,                                      \
                              pSortkey,                                     \
                              pExpand,                                      \
                              Position,                                     \
                              fStringSort,                                  \
                              fIgnoreSymbols,                               \
                              pCur,                                         \
                              pBegin )                                      \
{                                                                           \
    PSORTKEY pExpWt;              /* weight of 1 expansion char */          \
    BYTE AW;                      /* alphanumeric weight */                 \
    BYTE XW;                      /* case weight value with extra bits */   \
    DWORD PrevWt;                 /* previous weight */                     \
    BYTE PrevSM;                  /* previous script member */              \
    BYTE PrevAW;                  /* previuos alphanumeric weight */        \
    BYTE PrevCW;                  /* previuos case weight */                \
    LPWSTR pPrev;                 /* ptr to previous char */                \
                                                                            \
                                                                            \
    switch (SM)                                                             \
    {                                                                       \
        case ( UNSORTABLE ) :                                               \
        {                                                                   \
            /*                                                              \
             *  Character is unsortable, so skip it.                        \
             */                                                             \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( NONSPACE_MARK ) :                                            \
        {                                                                   \
            /*                                                              \
             *  Character is a nonspace mark, so only store                 \
             *  the diacritic weight.                                       \
             */                                                             \
            if (pPosDW > pDW)                                               \
            {                                                               \
                (*(pPosDW - 1)) += GET_DIACRITIC(pWeight);                  \
            }                                                               \
            else                                                            \
            {                                                               \
                *pPosDW = GET_DIACRITIC(pWeight);                           \
                pPosDW++;                                                   \
            }                                                               \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( EXPANSION ) :                                                \
        {                                                                   \
            /*                                                              \
             *  Expansion character - one character has 2                   \
             *  different weights.  Store each weight separately.           \
             */                                                             \
            pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP1]); \
            *pPosUW = GET_UNICODE(pExpWt);                                  \
            *pPosDW = GET_DIACRITIC(pExpWt);                                \
            *pPosCW = GET_CASE(pExpWt) & CaseMask;                          \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP2]); \
            *pPosUW = GET_UNICODE(pExpWt);                                  \
            *pPosDW = GET_DIACRITIC(pExpWt);                                \
            *pPosCW = GET_CASE(pExpWt) & CaseMask;                          \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( PUNCTUATION ) :                                              \
        {                                                                   \
            if (!fStringSort)                                               \
            {                                                               \
                /*                                                          \
                 *  Word Sort Method.                                       \
                 *                                                          \
                 *  Character is punctuation, so only store the special     \
                 *  weight.                                                 \
                 */                                                         \
                *((LPBYTE)pPosSW)       = HIBYTE(GET_POSITION_SW(Position)); \
                *(((LPBYTE)pPosSW) + 1) = LOBYTE(GET_POSITION_SW(Position)); \
                pPosSW++;                                                   \
                *pPosSW = GET_SPECIAL_WEIGHT(pWeight);                      \
                pPosSW++;                                                   \
                                                                            \
                break;                                                      \
            }                                                               \
                                                                            \
            /*                                                              \
             *  If using STRING sort method, treat punctuation the same     \
             *  as symbol.  So, FALL THROUGH to the symbol cases.           \
             */                                                             \
        }                                                                   \
                                                                            \
        case ( SYMBOL_1 ) :                                                 \
        case ( SYMBOL_2 ) :                                                 \
        case ( SYMBOL_3 ) :                                                 \
        case ( SYMBOL_4 ) :                                                 \
        case ( SYMBOL_5 ) :                                                 \
        {                                                                   \
            /*                                                              \
             *  Character is a symbol.                                      \
             *  Store the Unicode weights ONLY if the NORM_IGNORESYMBOLS    \
             *  flag is NOT set.                                            \
             */                                                             \
            if (!fIgnoreSymbols)                                            \
            {                                                               \
                *pPosUW = GET_UNICODE(pWeight);                             \
                *pPosDW = GET_DIACRITIC(pWeight);                           \
                *pPosCW = GET_CASE(pWeight) & CaseMask;                     \
                pPosUW++;                                                   \
                pPosDW++;                                                   \
                pPosCW++;                                                   \
            }                                                               \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( FAREAST_SPECIAL ) :                                          \
        {                                                                   \
            /*                                                              \
             *  Get the alphanumeric weight and the case weight of the      \
             *  current code point.                                         \
             */                                                             \
            AW = GET_ALPHA_NUMERIC(pWeight);                                \
            XW = (GET_CASE(pWeight) & CaseMask) | CASE_XW_MASK;             \
                                                                            \
            /*                                                              \
             *  Special case Repeat and Cho-On.                             \
             *    AW = 0  =>  Repeat                                        \
             *    AW = 1  =>  Cho-On                                        \
             *    AW = 2+ =>  Kana                                          \
             */                                                             \
            if (AW <= MAX_SPECIAL_AW)                                       \
            {                                                               \
                /*                                                          \
                 *  If the script member of the previous character is       \
                 *  invalid, then give the special character an             \
                 *  invalid weight (highest possible weight) so that it     \
                 *  will sort AFTER everything else.                        \
                 */                                                         \
                pPrev = pCur - 1;                                           \
                *pPosUW = MAP_INVALID_UW;                                   \
                while (pPrev >= pBegin)                                     \
                {                                                           \
                    PrevWt = GET_DWORD_WEIGHT(pHashN, *pPrev);              \
                    PrevSM = GET_SCRIPT_MEMBER(&PrevWt);                    \
                    if (PrevSM < FAREAST_SPECIAL)                           \
                    {                                                       \
                        if (PrevSM != EXPANSION)                            \
                        {                                                   \
                            /*                                              \
                             *  UNSORTABLE or NONSPACE_MARK.                \
                             *                                              \
                             *  Just ignore these, since we only care       \
                             *  about the previous UW value.                \
                             */                                             \
                            pPrev--;                                        \
                            continue;                                       \
                        }                                                   \
                    }                                                       \
                    else if (PrevSM == FAREAST_SPECIAL)                     \
                    {                                                       \
                        PrevAW = GET_ALPHA_NUMERIC(&PrevWt);                \
                        if (PrevAW <= MAX_SPECIAL_AW)                       \
                        {                                                   \
                            /*                                              \
                             *  Handle case where two special chars follow  \
                             *  each other.  Keep going back in the string. \
                             */                                             \
                            pPrev--;                                        \
                            continue;                                       \
                        }                                                   \
                                                                            \
                        *pPosUW = MAKE_UNICODE_WT(KANA, PrevAW);            \
                                                                            \
                        /*                                                  \
                         *  Only build weights 4, 5, 6, and 7 if the        \
                         *  previous character is KANA.                     \
                         *                                                  \
                         *  Always:                                         \
                         *    4W = previous CW  &  ISOLATE_SMALL            \
                         *    6W = previous CW  &  ISOLATE_KANA             \
                         *                                                  \
                         */                                                 \
                        PrevCW = (GET_CASE(&PrevWt) & CaseMask) |           \
                                 CASE_XW_MASK;                              \
                        EXTRA_WEIGHT_POS(0) = PrevCW & ISOLATE_SMALL;       \
                        EXTRA_WEIGHT_POS(2) = PrevCW & ISOLATE_KANA;        \
                                                                            \
                        if (AW == AW_REPEAT)                                \
                        {                                                   \
                            /*                                              \
                             *  Repeat:                                     \
                             *    UW = previous UW   (set above)            \
                             *    5W = WT_FIVE_REPEAT                       \
                             *    7W = previous CW  &  ISOLATE_WIDTH        \
                             */                                             \
                            EXTRA_WEIGHT_POS(1) = WT_FIVE_REPEAT;           \
                            EXTRA_WEIGHT_POS(3) = PrevCW & ISOLATE_WIDTH;   \
                        }                                                   \
                        else                                                \
                        {                                                   \
                            /*                                              \
                             *  Cho-On:                                     \
                             *    UW = previous UW  &  CHO_ON_UW_MASK       \
                             *    5W = WT_FIVE_CHO_ON                       \
                             *    7W = current  CW  &  ISOLATE_WIDTH        \
                             */                                             \
                            *pPosUW &= CHO_ON_UW_MASK;                      \
                            EXTRA_WEIGHT_POS(1) = WT_FIVE_CHO_ON;           \
                            EXTRA_WEIGHT_POS(3) = XW & ISOLATE_WIDTH;       \
                        }                                                   \
                                                                            \
                        pPosXW++;                                           \
                    }                                                       \
                    else                                                    \
                    {                                                       \
                        *pPosUW = GET_UNICODE(&PrevWt);                     \
                    }                                                       \
                                                                            \
                    break;                                                  \
                }                                                           \
                                                                            \
                /*                                                          \
                 *  Make sure there is a valid UW.  If not, quit out        \
                 *  of switch case.                                         \
                 */                                                         \
                if (*pPosUW == MAP_INVALID_UW)                              \
                {                                                           \
                    pPosUW++;                                               \
                    break;                                                  \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                /*                                                          \
                 *  Kana:                                                   \
                 *    SM = KANA                                             \
                 *    AW = current AW                                       \
                 *    4W = current CW  &  ISOLATE_SMALL                     \
                 *    5W = WT_FIVE_KANA                                     \
                 *    6W = current CW  &  ISOLATE_KANA                      \
                 *    7W = current CW  &  ISOLATE_WIDTH                     \
                 */                                                         \
                *pPosUW = MAKE_UNICODE_WT(KANA, AW);                        \
                EXTRA_WEIGHT_POS(0) = XW & ISOLATE_SMALL;                   \
                EXTRA_WEIGHT_POS(1) = WT_FIVE_KANA;                         \
                EXTRA_WEIGHT_POS(2) = XW & ISOLATE_KANA;                    \
                EXTRA_WEIGHT_POS(3) = XW & ISOLATE_WIDTH;                   \
                                                                            \
                pPosXW++;                                                   \
            }                                                               \
                                                                            \
            /*                                                              \
             *  Always:                                                     \
             *    DW = current DW                                           \
             *    CW = minimum CW                                           \
             */                                                             \
            *pPosDW = GET_DIACRITIC(pWeight);                               \
            *pPosCW = MIN_CW;                                               \
                                                                            \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( RESERVED_2 ) :                                               \
        case ( RESERVED_3 ) :                                               \
        {                                                                   \
            /*                                                              \
             *  Fill out the case statement so the compiler                 \
             *  will use a jump table.                                      \
             */                                                             \
            ;                                                               \
        }                                                                   \
    }                                                                       \
} 


BYTE pXWDrop[] =                  // values to drop from XW
{
    0xc6,                         // weight 4
    0x03,                         // weight 5
    0xe4,                         // weight 6
    0xc5                          // weight 7
}; 
BYTE pXWSeparator[] =             // separator values for XW
{
    0xff,                         // weight 4
    0x02,                         // weight 5
    0xff,                         // weight 6
    0xff                          // weight 7
}; 

BYTE pAllWSeparator[] = 
{
  0x01, 0x01, 0xFF, 0x02, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

BOOL fBaseUsed = FALSE, fCBaseUsed = FALSE;
LOC_HASH *g_pLocale = NULL;
MAP_BUFF g_RootBuff={0};


MAP_BUFF *GetMapBuff(DWORD cbBuff)
{
  MAP_BUFF *pBuf;

  //EnterCriticalSection(&g_CritSec);
  for ( pBuf = &g_RootBuff; ; pBuf = pBuf->pNext )
  {
	if ( cbBuff <= pBuf->cbBuff && !pBuf->bAllocated )
	{
		pBuf->bAllocated = TRUE;
		//LeaveCriticalSection(&g_CritSec);
		return pBuf;
	}
	if (!pBuf->pNext) break;
  }
  cbBuff += 4095;
  cbBuff &= 0xFFFFF000;
  pBuf->pNext = malloc(cbBuff + sizeof(MAP_BUFF));
  pBuf = pBuf->pNext;
  if (pBuf)
  {
    pBuf->pNext = NULL;
    pBuf->cbBuff = cbBuff;
    pBuf->bAllocated = TRUE;
  }
  //LeaveCriticalSection(&g_CritSec);
  return pBuf;
}

MAP_BUFF *FreeMapBuff(MAP_BUFF *pPBuf)
{
  MAP_BUFF *pBuf;

  for (pBuf = &g_RootBuff; ; pBuf = pBuf->pNext)
  {
    if (pPBuf == pBuf)
	{
      pBuf->bAllocated = FALSE;
	  break;
	}
  }
  return pBuf;
}

PLOC_HASH DBCreateTables(int Locale, int uFlags, SORTKEY *pSortKeys)
{
	// TODO: Implement
	return NULL;
}

PLOC_HASH SetSortInfo(LCID Locale, DWORD dwMapFlags)
{
	LCID Loc = Locale;
	PLOC_HASH pHashN;

	if (dwMapFlags & LCMAP_LINGUISTIC_CASING) Loc*=-1;
	for (pHashN = (PLOC_HASH)&g_pLocale; pHashN; pHashN=pHashN->pNext)
	{
		if (pHashN->Locale == Loc) return pHashN;
		if (!pHashN->pNext) break;
	}

#ifndef MDBTOOLS
	if (!fBaseUsed && Loc == MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT))
	{
		if (!(pHashN->pNext=calloc(sizeof(LOC_HASH), 1))) return NULL;
		pHashN->pNext->Locale = Loc;
		pHashN->pNext->pSortkey = (PSORTKEY)g_SortKeys;
		pHashN->pNext->wUnk1 = 34;
		fBaseUsed = TRUE;
		return pHashN->pNext;
	}
#endif

	if (!fCBaseUsed && (int)Loc == (int)(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT))*-1)
	{
		if (!(pHashN->pNext=calloc(sizeof(LOC_HASH), 1))) return NULL;
		pHashN->pNext->Locale = Loc;
		pHashN->pNext->pSortkey = (PSORTKEY)g_SortCompKeys;
		pHashN->pNext->wUnk1 = 127;
		fCBaseUsed = TRUE;
		return pHashN->pNext;
	}
	// MSWDAT10.DLL
	pHashN->pNext = DBCreateTables(Locale, dwMapFlags, 
#ifndef MDBTOOLS
		fBaseUsed?NULL:(PSORTKEY)g_SortKeys
#else
		NULL
#endif
		);
	fBaseUsed = TRUE;
	return pHashN->pNext;
}


int WINAPI DBLCMapStringW(LCID Locale, int dwFlags, WCHAR *lpSrcString, signed int cchSrc, LPBYTE pDest, int cchDest)
{
	signed int cchExtra, cchSep;
	BYTE CaseMask;
	register int WeightLen;	// Length of one Weight set
	MAP_BUFF *pBuf;
	LPWSTR pUW;				// Unicode Weights
	LPBYTE pDW;                   // ptr to Diacritic Weights
	LPBYTE pCW;                   // ptr to Case Weights
	LPBYTE pXW;                   // ptr to Extra Weights
	LPWSTR pSW;                   // ptr to Special Weights 
	LPWSTR pPosUW;				// pUW buf ptr
	LPBYTE pPosCW;				// pCW buf ptr
	LPBYTE pPosDW;				// pDW buf ptr
	LPBYTE pPosXW;				// ptr to position in pXW buffer 
	LPWSTR pPosSW;                // ptr to position in pSW buffer 
	PSORTKEY pWeight;			// weight of char ptr
	LPBYTE pTmp;                  // ptr to go through UW, XW, and SW 
	LPBYTE pPosTmp, pPosOffset;               // ptr to tmp position in XW 
	int PosCtr, ctr;
	LPWSTR pPos;
	PLOC_HASH pHashN;			// Hash table
	BYTE SM;					// Script member
	BOOL fStringSort;             // if using string sort method 
	BOOL fIgnoreSymbols;          // if ignore symbols flag is set 

	if (!lpSrcString || !(pHashN = SetSortInfo(Locale, dwFlags))) return 0;
	if (cchSrc <= -1) cchSrc = NlsStrLenW(lpSrcString) + 1;

	if (dwFlags & (LCMAP_LOWERCASE | LCMAP_UPPERCASE))
	{
		PCASE pMapTable;

		if (dwFlags & LCMAP_LOWERCASE)
		{
			if (dwFlags & LCMAP_LINGUISTIC_CASING)
			{
				pMapTable = g_LinguisticLowerCase;
				for (PosCtr = 0; g_SpecialCase[PosCtr].Locale; PosCtr++)
				{
					if (g_SpecialCase[PosCtr].Locale == Locale)
					{
						pMapTable = g_SpecialCase[PosCtr].pLowerCase;
						break;
					}
				}
			}
			else
			{
				pMapTable = g_LowerCase;
			}
		}
		else
		{
			if (dwFlags & LCMAP_LINGUISTIC_CASING)
			{
				pMapTable = g_LinguisticUpperCase;
				for (PosCtr = 0; g_SpecialCase[PosCtr].Locale; PosCtr++)
				{
					if (g_SpecialCase[PosCtr].Locale == Locale)
					{
						pMapTable = g_SpecialCase[PosCtr].pUpperCase;
						break;
					}
				}
			}
			else
			{
				pMapTable = g_UpperCase;
			}
		}
		if ( cchSrc > 0 )
		{
			for (PosCtr = 0; PosCtr < cchSrc; PosCtr++)
			{
				pDest[PosCtr] = lpSrcString[PosCtr] + g_CasePages[pMapTable[lpSrcString[PosCtr]]][lpSrcString[PosCtr]];
			}
		}
		return PosCtr;
	}

	// TODO: Invalid Flags Check
	WeightLen = cchSrc * MAX_EXPANSION;
	if (!(dwFlags & LCMAP_SORTKEY) || !(pBuf = GetMapBuff(WeightLen * MAX_WEIGHTS * sizeof(WCHAR))))
		return 0;

	pUW = (LPWSTR)((DWORD)pBuf + sizeof(MAP_BUFF));
	pDW = (LPBYTE)(pUW + (WeightLen * (NUM_BYTES_UW / sizeof(WCHAR))));
	pCW = (LPBYTE)(pDW + (WeightLen * NUM_BYTES_DW));
	pXW = (LPBYTE)(pCW + (WeightLen * NUM_BYTES_CW));
	pSW = (LPWSTR)(pXW + (WeightLen * NUM_BYTES_XW));
	pPosUW = pUW;
	pPosDW = pDW;
	pPosCW = pCW;
	pPosXW = pXW;
	pPosSW = pSW; 

	CaseMask = 0xFF;
	if ( dwFlags & NORM_IGNORECASE )
		CaseMask = CASE_UPPER_MASK;
	if ( dwFlags & NORM_IGNOREKANATYPE )
		CaseMask &= CASE_KANA_MASK;
	if ( dwFlags & NORM_IGNOREWIDTH )
		CaseMask &= CASE_WIDTH_MASK;

	fStringSort = dwFlags & SORT_STRINGSORT; 
	fIgnoreSymbols = dwFlags & NORM_IGNORESYMBOLS;
	pPos = lpSrcString;
	if ( cchSrc >= 1 )
	{
		// TODO: JUMPOUT(pHashN[4], 0, &loc_10001DFF);	// IfCompression?
		for (PosCtr = 1; PosCtr <= cchSrc; PosCtr++, pPos++)
		{
			pWeight = (PSORTKEY)(&(pHashN->pSortkey)[*pPos]);
			SM = GET_SCRIPT_MEMBER(pWeight);

			if (SM > MAX_SPECIAL_CASE)
			{
				*pPosUW = UNICODE_WT(pWeight);
				*pPosDW = GET_DIACRITIC(pWeight);
				*pPosCW = GET_CASE(pWeight) & CaseMask;
				pPosUW++;
				pPosDW++;
				pPosCW++;
			}
			else
			{
                SPECIAL_CASE_HANDLER( SM,
                                      pWeight,
                                      pHashN->pSortkey,
                                      g_Expansion,
                                      pPosUW - pUW + 1,
                                      fStringSort,
                                      fIgnoreSymbols,
                                      pPos,
                                      (LPWSTR)lpSrcString ); 
			}
		}
	}
	PosCtr = 0;
	if (cchDest == 0)
	{
		if ( dwFlags & LCMAP_LINGUISTIC_CASING )
		{
			LPBYTE pUWTmp;

			for (pUWTmp = (LPBYTE)pUW; pUWTmp != (LPBYTE)pPosUW; pUWTmp++)
				if (*pUWTmp) ++PosCtr;
		}
		else
		{
			PosCtr += ((LPBYTE)pPosUW - (LPBYTE)pUW); 
		}
		PosCtr++; 
		if (!(dwFlags & NORM_IGNORENONSPACE))
		{
			pPosDW--;
			if (pHashN->IfReverseDW == TRUE)
			{
				while ((pDW <= pPosDW) && (*pDW <= MIN_DW))
				{
					pDW++;
				}
				PosCtr += (pPosDW - pDW + 1);
			}
			else
			{
				while ((pPosDW >= pDW) && (*pPosDW <= MIN_DW))
				{
					pPosDW--;
				}
				PosCtr += (pPosDW - pDW + 1);
			}
		}

		PosCtr++; 

		if ((dwFlags & NORM_DROP_CW) != NORM_DROP_CW)
		{
			pPosCW--;
			while ((pPosCW >= pCW) && (*pPosCW <= MIN_CW))
			{
				pPosCW--;
			}
			PosCtr += (pPosCW - pCW + 1);
		}

		PosCtr++; 


		if (pXW < pPosXW)
		{
			if (dwFlags & NORM_IGNORENONSPACE)
			{
				PosCtr += 2;
				ctr = 2;
			}
			else
			{
				ctr = 0;
			}
			pPosXW--;
			for (; ctr < NUM_BYTES_XW; ctr++)
			{
				pTmp = pXW + (WeightLen * ctr);
				pPosTmp = pPosXW + (WeightLen * ctr);
				while ((pPosTmp >= pTmp) && (*pPosTmp == pXWDrop[ctr]))
				{
					pPosTmp--;
				}
				if ( dwFlags & LCMAP_LINGUISTIC_CASING )
				{
					PosCtr += (pPosTmp - pTmp + 1 + gXW_comp[ctr][3]) / (gXW_comp[ctr][3]+1);
				}
				else
				{
					PosCtr += (pPosTmp - pTmp + 1);
				}
				PosCtr++;
			}
		} 

		PosCtr++;

		if (!fIgnoreSymbols)
		{
			PosCtr += ((LPBYTE)pPosSW - (LPBYTE)pSW);
		}

		PosCtr++;
	}
	else
	{
		if (dwFlags & LOCALE_NOUSEROVERRIDE)
		{
		// TODO: JUMPOUT(dwFlags & LOCALE_NOUSEROVERRIDE, 0, &loc_10002502);

		}

		cchExtra = 0;
		cchSep = 0;
		if ( pPosUW != pUW )
		{
			if (dwFlags & LCMAP_LINGUISTIC_CASING)
			{
				LPBYTE pUWTmp;

				for (pUWTmp = (LPBYTE)pUW; pUWTmp != (LPBYTE)pPosUW; pUWTmp++)
					if (*pUWTmp) pDest[PosCtr++]=*pUWTmp;
			}
			else
			{
				PosCtr = ((LPBYTE)pPosUW - (LPBYTE)pUW);
				if (cchDest < PosCtr)
				{
					FreeMapBuff(pBuf);
					return 0;
				}
				memcpy (pDest, pUW, PosCtr);

			}
		}
		if (cchDest < (PosCtr + 1))
		{
			FreeMapBuff(pBuf);
			return 0;
		} 
		pDest[PosCtr] = SORTKEY_SEPARATOR;
		PosCtr++;


		if (!(dwFlags & NORM_IGNORENONSPACE))
		{
			pPosDW--;
			if (pHashN->IfReverseDW == TRUE)
			{
				while ((pDW <= pPosDW) && (*pDW <= MIN_DW))
				{
					pDW++;
				}
				if ((cchDest - PosCtr) <= (pPosDW - pDW + 1))
				{
					FreeMapBuff(pBuf);
					return (0);
				}
				while (pPosDW >= pDW)
				{
					pDest[PosCtr] = *pPosDW;
					PosCtr++;
					pPosDW--;
				}
			}
			else
			{
				while ((pPosDW >= pDW) && (*pPosDW <= MIN_DW))
				{
					pPosDW--;
				}
				if ((cchDest - PosCtr) <= (pPosDW - pDW + 1))
				{
					FreeMapBuff(pBuf);
					return (0);
				}
				memcpy(&pDest[PosCtr], pDW, pPosDW - pDW + 1);
				PosCtr += pPosDW - pDW + 1;
			}
		} 


		if ( PosCtr >= cchDest )
		{
			if (dwFlags & LCMAP_LINGUISTIC_CASING)
			{
				FreeMapBuff(pBuf);
				return (0);
			}
		}
		else
		{
			pDest[PosCtr] = SORTKEY_SEPARATOR;
			PosCtr++;

			if ( PosCtr < cchDest )
				cchExtra = 1;
		}

		if ((dwFlags & NORM_DROP_CW) != NORM_DROP_CW) 
		{
			pPosCW--;
			while ((pPosCW >= pCW) && (*pPosCW <= MIN_CW))
			{
				pPosCW--;
			}
			if ((cchDest - PosCtr) <= (pPosCW - pCW + 1))
			{
				FreeMapBuff(pBuf);
				return (0);
			}
			memcpy(&pDest[PosCtr], pCW, pPosCW - pCW);
			PosCtr += pPosCW - pCW;
		}
		if ( PosCtr >= cchDest )
		{
			if (dwFlags & LCMAP_LINGUISTIC_CASING)
			{
				FreeMapBuff(pBuf);
				return (0);
			}
		}
		else
		{
			pDest[PosCtr] = SORTKEY_SEPARATOR;
			PosCtr++; 
			if ( PosCtr < cchDest ) ++cchExtra;
		}

	//JUMPOUT(pXW, pPosXW, sub_1000262D);
		if (pXW < pPosXW)
		{
			if (dwFlags & NORM_IGNORENONSPACE)
			{
				if ( PosCtr >= cchDest )
				{
					if (dwFlags & LCMAP_LINGUISTIC_CASING)
					{
						FreeMapBuff(pBuf);
						return (0);
					}
				}
				else
				{
					pDest[PosCtr++] = pXWSeparator[0];
					if ( PosCtr >= cchDest )
					{
						if (dwFlags & LCMAP_LINGUISTIC_CASING)
						{
							FreeMapBuff(pBuf);
							return (0);
						}
					}
					else
					{
						cchSep = 1;
						++cchExtra;
						pDest[PosCtr++] = pXWSeparator[1];
						if ( PosCtr < cchDest ) ++cchExtra;
					}
					ctr = 2;	
				}
			}
			else
			{
				ctr = 0;
			} 

			for (; ctr < NUM_BYTES_XW; ctr++) 
			{
				if ( pPosXW - pXW )
				{
					pTmp = pXW + (WeightLen * ctr);
					pPosTmp = pPosXW + (WeightLen * ctr); 
					pPosOffset = (LPBYTE)(pPosXW - pXW);

					while ((pPosTmp >= pTmp) && (*pPosTmp == pXWDrop[ctr])) 
					{
						pPosTmp--;
					}
					if ( (dwFlags & LCMAP_LINGUISTIC_CASING) )
					{
						BYTE pNumOffs, c;

						for (c = 0x80, pNumOffs=(BYTE)gXW_comp[ctr][3]; pPosTmp > pTmp; pTmp++)
						{
							c |= (BYTE)((ctr==1?((*pTmp>>1)|*pTmp&1):*pTmp) & gXW_comp[ctr][0]) >> 
							gXW_comp[ctr][1] << pNumOffs * gXW_comp[ctr][2];
							if (pNumOffs)
							{
								pNumOffs--;
								continue;
							}
							if ( cchDest - PosCtr < 2)
							{
								FreeMapBuff(pBuf);
								return (0);
							}
							pDest[PosCtr++] = c;
							pNumOffs = gXW_comp[ctr][3];
							c = 0x80;
						}
						if (pNumOffs != gXW_comp[ctr][3] && cchDest - PosCtr >= 2) pDest[PosCtr++] = c;
					}
					else
					{
						if ((cchDest - PosCtr) <= (pPosTmp - pTmp + 1))
						{
							FreeMapBuff(pBuf);
							return (0);
						} 
						memcpy(&pDest[PosCtr], pTmp, pPosTmp - pTmp);
						PosCtr += pPosTmp - pTmp;
					}
				}
				if ( PosCtr >= cchDest )
				{
					if (dwFlags & LCMAP_LINGUISTIC_CASING)
					{
						FreeMapBuff(pBuf);
						return (0);
					}
				}
				else
				{
					pDest[PosCtr] = pXWSeparator[ctr];
					PosCtr++;
					if ( PosCtr < cchDest )
					{
						++cchExtra;
						++cchSep;
					}
				}
			}

			if ( PosCtr >= cchDest )
			{
				if (dwFlags & LCMAP_LINGUISTIC_CASING)
				{
					FreeMapBuff(pBuf);
					return (0);
				}
			}
		}

		pDest[PosCtr] = SORTKEY_SEPARATOR;   
		PosCtr++;

		if ( PosCtr < cchDest ) cchExtra++;
		if (!fIgnoreSymbols)
		{
			if ((cchDest - PosCtr) <= ((LPBYTE)pPosSW - (LPBYTE)pSW))
			{
				FreeMapBuff(pBuf);
				return (0);
			}
			memcpy(&pDest[PosCtr], pSW, (LPBYTE)pPosSW - (LPBYTE)pSW);
			PosCtr += (LPBYTE)pPosSW - (LPBYTE)pSW;
		}

		if ( PosCtr >= cchDest )
		{
			if (dwFlags & LCMAP_LINGUISTIC_CASING)
			{
				FreeMapBuff(pBuf);
				return (0);
			}
		}

		pDest[PosCtr] = SORTKEY_TERMINATOR; 
		if ( cchExtra && (dwFlags & LCMAP_LINGUISTIC_CASING) )
		{
			do
			{
				--cchExtra;
				if ( (cchSep && pDest[PosCtr - 2] != pAllWSeparator[cchExtra]) || pDest[PosCtr - 2] != SORTKEY_SEPARATOR )
					break;
				pDest[PosCtr - 1] = SORTKEY_TERMINATOR;
				PosCtr--;
			}
			while ( cchExtra );
		}	

	}
	FreeMapBuff(pBuf);
	return PosCtr;
}

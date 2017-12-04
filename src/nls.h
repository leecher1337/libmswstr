//
//  Sortkey Structure.
//
typedef struct sortkey_s {

    union {
        struct sm_aw_s {
            BYTE   Alpha;              // alphanumeric weight
            BYTE   Script;             // script member
        } SM_AW;

        WORD  Unicode;                 // unicode weight

    } UW;

    BYTE      Diacritic;               // diacritic weight
    BYTE      Case;                    // case weight (with COMP)

} SORTKEY, *PSORTKEY; 

//
//  Expansion Structure.
//
typedef struct expand_s {
    WCHAR     UCP1;                    // Unicode code point 1
    WCHAR     UCP2;                    // Unicode code point 2
} EXPAND, *PEXPAND; 


//
//  Separator and Terminator Values - Sortkey String.
//
#define SORTKEY_SEPARATOR    0x01
#define SORTKEY_TERMINATOR   0x00 

//
// Dummy, currently not needed
//
typedef PVOID PCOMPRESS_HDR;
typedef PVOID PCOMPRESS_2;
typedef PVOID PCOMPRESS_3;

//
//  Locale Hash Table Structure.
//
typedef struct loc_hash_s {
    struct loc_hash_s *pNext;          // ptr to next locale hash node
    LCID           Locale;             // locale ID
    PSORTKEY       pSortkey;           // ptr to sortkey table
    BOOL           IfReverseDW;        // if DW should go from right to left
    BOOL           IfCompression;      // if compression code points exist
    BOOL           IfDblCompression;   // if double compression exists
    PCOMPRESS_HDR  pCompHdr;           // ptr to compression header
    PCOMPRESS_2    pCompress2;         // ptr to 2 compression table
    PCOMPRESS_3    pCompress3;         // ptr to 3 compression table
	WORD		   wUnk1;
} LOC_HASH, *PLOC_HASH;

typedef  LPBYTE    PCASE;          // ptr to Lower or Upper Case table 

typedef struct sc_hash_s {
    LCID           Locale;             // locale ID
    PCASE          pUpperCase;         // ptr to Upper Case table
    PCASE          pLowerCase;         // ptr to Lower Case table
} SC_HASH, *PSC_HASH;

typedef struct map_buff_s {
	struct map_buff_s *	pNext;
	DWORD				cbBuff;
	BOOL				bAllocated;
} MAP_BUFF;


//
//  SORTKEY WEIGHT MACROS
// 
#define UNICODE_WT_SWP(pwt)       ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) )
#define UNICODE_WT(pwt)			  ( (WORD)(LOBYTE(UNICODE_WT_SWP(pwt))<<8))|HIBYTE(UNICODE_WT_SWP(pwt))
#define GET_SCRIPT_MEMBER(pwt)	  ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Script) ) 
#define GET_CASE(pwt)             ( (BYTE)((((PSORTKEY)(pwt))->Case) & CASE_MASK) ) 
#define GET_DIACRITIC(pwt)        ( (BYTE)(((PSORTKEY)(pwt))->Diacritic) ) 


//
//  Masks to isolate the various bits in the case weight.
//
//  NOTE: Bit 2 must always equal 1 to avoid getting a byte value
//        of either 0 or 1.
//
#define CASE_XW_MASK              0xc4 

#define ISOLATE_SMALL             ( (BYTE)((~CASE_SMALL_MASK) | CASE_XW_MASK) )
#define ISOLATE_KANA              ( (BYTE)((~CASE_KANA_MASK)  | CASE_XW_MASK) )
#define ISOLATE_WIDTH             ( (BYTE)((~CASE_WIDTH_MASK) | CASE_XW_MASK) ) 

//
//  UW Mask for Cho-On:
//    Leaves bit 7 on in AW, so it becomes Repeat if it follows Kana N.
//
#define CHO_ON_UW_MASK            0xff87 

//
//  Values for weight 5.
//
#define WT_FIVE_KANA    3
#define WT_FIVE_REPEAT  4
#define WT_FIVE_CHO_ON  5 


//
//  Bit mask values.
// 
//  Case Weight (CW) - 8 bits:
//    bit 0   => width
//    bit 1,2 => small kana, sei-on
//    bit 3,4 => upper/lower case
//    bit 5   => kana
//    bit 6,7 => compression
// 
#define COMPRESS_3_MASK      0xc0           // compress 3-to-1 or 2-to-1
#define COMPRESS_2_MASK      0x80           // compress 2-to-1 

#define CASE_MASK            0x3f           // zero out compression bits

#define CASE_UPPER_MASK      0xe7           // zero out case bits
#define CASE_SMALL_MASK      0xf9           // zero out small modifier bits
#define CASE_KANA_MASK       0xdf           // zero out kana bit
#define CASE_WIDTH_MASK      0xfe           // zero out width bit 

#define SW_POSITION_MASK     0x8003         // avoid 0 or 1 in bytes of word 

//
//  Values for fareast special case alphanumeric weights.
//
#define AW_REPEAT       0
#define AW_CHO_ON       1
#define MAX_SPECIAL_AW  AW_CHO_ON 

//
//  Script Member Offsets into SMWeight structure.
//
#define UNSORTABLE           0
#define NONSPACE_MARK        1
#define EXPANSION            2
#define FAREAST_SPECIAL      3

  //  Values 4 thru 5 are available for other special cases
#define RESERVED_2           4
#define RESERVED_3           5

#define PUNCTUATION          6

#define SYMBOL_1             7
#define SYMBOL_2             8
#define SYMBOL_3             9
#define SYMBOL_4             10
#define SYMBOL_5             11

#define NUMERIC_1            12
#define NUMERIC_2            13 

#define KANA                 34 

#define MAX_SPECIAL_CASE     SYMBOL_5 

//
//  String Constants.
// 
#define MAX_EXPANSION             3    // max number of expansion characters 
#define MAX_WEIGHTS               6    // max number of words in all weights 

//
//  Lowest weight values.
//  Used to remove trailing DW and CW values.
//
#define MIN_DW  2
#define MIN_CW  2 





////////////////////////////////////////////////////////////////////////////
//
//  SORTKEY WEIGHT MACROS
//
//  Parse out the different sortkey weights from a DWORD value.
//
//////////////////////////////////////////////////////////////////////////// 
#define GET_ALPHA_NUMERIC(pwt)    ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Alpha) ) 

#define GET_UNICODE(pwt)          UNICODE_WT(pwt)

#define GET_COMPRESSION(pwt)      ( (BYTE)((((PSORTKEY)(pwt))->Case) & COMPRESS_3_MASK) )

#define GET_EXPAND_INDEX(pwt)     ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Alpha) )

#define GET_SPECIAL_WEIGHT(pwt)   GET_UNICODE(pwt)

#define MAKE_SORTKEY_DWORD(wt)    ( (DWORD)(*((LPDWORD)(&(wt)))) ) 

#define MAKE_UNICODE_WT(sm, aw)   ( (WORD)((((WORD)(sm)) << 8) | (WORD)(aw)) )

#define GET_DWORD_WEIGHT(pHashN, wch)                                      \
      ( MAKE_SORTKEY_DWORD(((pHashN)->pSortkey)[wch]) ) 

//  position returned is backwards - byte reversal
#define GET_POSITION_SW(pos)      ( (WORD)(((pos) << 2) | SW_POSITION_MASK) ) 



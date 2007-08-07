/*******************************************************************
 *
 *  Copyright 2005  David Turner, The FreeType Project (www.freetype.org)
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_GLOBAL_H
#define HARFBUZZ_GLOBAL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __cplusplus
#define HB_BEGIN_HEADER  extern "C" {
#define HB_END_HEADER  }
#else
#define HB_BEGIN_HEADER  /* nothing */
#define HB_END_HEADER  /* nothing */
#endif

HB_BEGIN_HEADER

typedef uint8_t HB_Bool;

typedef uint8_t HB_Byte;
typedef uint16_t HB_UShort;
typedef uint32_t HB_UInt;
typedef int8_t HB_Char;
typedef int16_t HB_Short;
typedef int32_t HB_Int;

typedef uint16_t HB_UChar16;
typedef uint32_t HB_UChar32;
typedef uint32_t HB_Glyph;
typedef int32_t HB_Fixed; /* 26.6 */

typedef int32_t HB_16Dot16; /* 16.16 */

typedef void * HB_Pointer;
typedef uint32_t HB_Tag;

typedef enum {
    HB_Err_Ok = 0,
    HB_Err_Invalid_Stream_Operation,
    HB_Err_Invalid_Argument,
    HB_Err_Out_Of_Memory,
    HB_Err_Invalid_Face_Handle, 
    HB_Err_Table_Missing
} HB_Error;

typedef struct {
    HB_Fixed x;
    HB_Fixed y;
} HB_FixedPoint;

typedef struct HB_Font_ *HB_Font;
typedef struct HB_StreamRec_ *HB_Stream;

#define HB_IsHighSurrogate(ucs) \
    (((ucs) & 0xfc00) == 0xd800)

#define HB_IsLowSurrogate(ucs) \
    (((ucs) & 0xfc00) == 0xdc00)

#define HB_SurrogateToUcs4(high, low) \
    (((HB_UChar32)(high))<<10) + (low) - 0x35fdc00;

#define HB_MAKE_TAG( _x1, _x2, _x3, _x4 ) \
          ( ( (HB_UInt)_x1 << 24 ) |     \
            ( (HB_UInt)_x2 << 16 ) |     \
            ( (HB_UInt)_x3 <<  8 ) |     \
              (HB_UInt)_x4         )

/* memory macros used by the OpenType parser */
#define  ALLOC(_ptr,_size)   \
           ( (_ptr) = _hb_alloc( _size, &error ), error != 0 )

#define  REALLOC(_ptr,_oldsz,_newsz)  \
           ( (_ptr) = _hb_realloc( (_ptr), (_oldsz), (_newsz), &error ), error != 0 )

#define  FREE(_ptr)                    \
  do {                                 \
    if ( (_ptr) )                      \
    {                                  \
      _hb_free( _ptr );     \
      _ptr = NULL;                     \
    }                                  \
  } while (0)

#define  ALLOC_ARRAY(_ptr,_count,_type)   \
           ALLOC(_ptr,(_count)*sizeof(_type))

#define  REALLOC_ARRAY(_ptr,_oldcnt,_newcnt,_type) \
           REALLOC(_ptr,(_oldcnt)*sizeof(_type),(_newcnt)*sizeof(_type))

#define  MEM_Copy(dest,source,count)   memcpy( (char*)(dest), (const char*)(source), (size_t)(count) )


HB_Pointer _hb_alloc( HB_UInt   size,
                      HB_Error  *perror_ );

HB_Pointer _hb_realloc( HB_Pointer  block,
                        HB_UInt    old_size,
                        HB_UInt    new_size,
                        HB_Error   *perror_ );

void _hb_free( HB_Pointer  block );

HB_END_HEADER

#endif

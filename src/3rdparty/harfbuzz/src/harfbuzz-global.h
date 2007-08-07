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

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h>

FT_BEGIN_HEADER

typedef uint16_t HB_UChar16;
typedef uint32_t HB_UChar32;
typedef uint32_t HB_Glyph;
typedef uint8_t HB_Bool;
typedef uint32_t HB_Fixed; /* 26.6 */

typedef enum {
    HB_Err_Ok,
    HB_Err_Invalid_Stream_Operation,
    HB_Err_Invalid_Argument,
    HB_Err_Out_Of_Memory,
    HB_Err_Invalid_Face_Handle
} HB_Error;

typedef struct {
    HB_Fixed x;
    HB_Fixed y;
} HB_FixedPoint;

#define HB_IsHighSurrogate(ucs) \
    (((ucs) & 0xfc00) == 0xd800)

#define HB_IsLowSurrogate(ucs) \
    (((ucs) & 0xfc00) == 0xdc00)

#define HB_SurrogateToUcs4(high, low) \
    (((HB_UChar32)(high))<<10) + (low) - 0x35fdc00;


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


FT_Pointer _hb_alloc( FT_ULong   size,
                             HB_Error  *perror_ );

FT_Pointer _hb_realloc( FT_Pointer  block,
                               FT_ULong    old_size,
                               FT_ULong    new_size,
                               HB_Error   *perror_ );

void _hb_free( FT_Pointer  block );

FT_END_HEADER

#endif

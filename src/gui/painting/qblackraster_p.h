/***************************************************************************/
/*                                                                         */
/*  qblackraster_p.h, derived from ftraster.h                              */
/*                                                                         */
/*    The FreeType glyph rasterizer (specification).                       */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, ../../3rdparty/freetype/docs/FTL.TXT.  By continuing to use,  */
/*  modify, or distribute this file you indicate that you have read        */
/*  the license and understand and accept it fully.                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTRASTER_H__
#define __FTRASTER_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include <private/qrasterdefs_p.h>

QT_FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* Uncomment the following line if you are using ftraster.c as a         */
  /* standalone module, fully independent of FreeType.                     */
  /*                                                                       */
/* #define _STANDALONE_ */

#ifndef QT_FT_EXPORT_VAR
#define QT_FT_EXPORT_VAR( x )  extern  x
#endif

  QT_FT_EXPORT_VAR( const QT_FT_Raster_Funcs )  qt_ft_standard_raster;


QT_FT_END_HEADER

#ifdef __cplusplus
    }
#endif


#endif /* __FTRASTER_H__ */


/* END */

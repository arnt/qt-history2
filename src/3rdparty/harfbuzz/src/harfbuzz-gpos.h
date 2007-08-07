/*******************************************************************
 *
 *  Copyright 1996-2000 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  Copyright 2006  Behdad Esfahbod
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_GPOS_H
#define HARFBUZZ_GPOS_H

#include "harfbuzz-gdef.h"
#include "harfbuzz-buffer.h"

HB_BEGIN_HEADER

/* Lookup types for glyph positioning */

#define HB_GPOS_LOOKUP_SINGLE     1
#define HB_GPOS_LOOKUP_PAIR       2
#define HB_GPOS_LOOKUP_CURSIVE    3
#define HB_GPOS_LOOKUP_MARKBASE   4
#define HB_GPOS_LOOKUP_MARKLIG    5
#define HB_GPOS_LOOKUP_MARKMARK   6
#define HB_GPOS_LOOKUP_CONTEXT    7
#define HB_GPOS_LOOKUP_CHAIN      8
#define HB_GPOS_LOOKUP_EXTENSION  9

/* A pointer to a function which accesses the PostScript interpreter.
   Multiple Master fonts need this interface to convert a metric ID
   (as stored in an OpenType font version 1.2 or higher) `metric_id'
   into a metric value (returned in `metric_value').

   `data' points to the user-defined structure specified during a
   call to HB_GPOS_Register_MM_Function().

   `metric_value' must be returned as a scaled value (but shouldn't
   be rounded).                                                       */

typedef HB_Error  (*HB_MMFunction)(HB_Font       font,
				    HB_UShort    metric_id,
				    HB_Fixed*      metric_value,
				    void*        data );


struct  HB_GPOSHeader_
{
  HB_16Dot16           Version;

  HB_ScriptList     ScriptList;
  HB_FeatureList    FeatureList;
  HB_LookupList     LookupList;

  HB_GDEFHeader*    gdef;

  /* this is OpenType 1.2 -- Multiple Master fonts need this
     callback function to get various metric values from the
     PostScript interpreter.                                 */

  HB_MMFunction     mmfunc;
  void*              data;
};

typedef struct HB_GPOSHeader_  HB_GPOSHeader;
typedef HB_GPOSHeader* HB_GPOS;


HB_Error  HB_Load_GPOS_Table( HB_Stream stream, 
                              HB_GPOSHeader** gpos,
			      HB_GDEFHeader*  gdef,
                              HB_Stream       gdefStream );


HB_Error  HB_Done_GPOS_Table( HB_GPOSHeader* gpos );


HB_Error  HB_GPOS_Select_Script( HB_GPOSHeader*  gpos,
				 HB_UInt         script_tag,
				 HB_UShort*       script_index );

HB_Error  HB_GPOS_Select_Language( HB_GPOSHeader*  gpos,
				   HB_UInt         language_tag,
				   HB_UShort        script_index,
				   HB_UShort*       language_index,
				   HB_UShort*       req_feature_index );

HB_Error  HB_GPOS_Select_Feature( HB_GPOSHeader*  gpos,
				  HB_UInt         feature_tag,
				  HB_UShort        script_index,
				  HB_UShort        language_index,
				  HB_UShort*       feature_index );


HB_Error  HB_GPOS_Query_Scripts( HB_GPOSHeader*  gpos,
				 HB_UInt**       script_tag_list );

HB_Error  HB_GPOS_Query_Languages( HB_GPOSHeader*  gpos,
				   HB_UShort        script_index,
				   HB_UInt**       language_tag_list );

HB_Error  HB_GPOS_Query_Features( HB_GPOSHeader*  gpos,
				  HB_UShort        script_index,
				  HB_UShort        language_index,
				  HB_UInt**       feature_tag_list );


HB_Error  HB_GPOS_Add_Feature( HB_GPOSHeader*  gpos,
			       HB_UShort        feature_index,
			       HB_UInt          property );

HB_Error  HB_GPOS_Clear_Features( HB_GPOSHeader*  gpos );


HB_Error  HB_GPOS_Register_MM_Function( HB_GPOSHeader*  gpos,
					HB_MMFunction   mmfunc,
					void*            data );

/* If `dvi' is TRUE, glyph contour points for anchor points and device
   tables are ignored -- you will get device independent values.         */


HB_Error  HB_GPOS_Apply_String( HB_Font           font,
				HB_GPOSHeader*   gpos,
				HB_UShort         load_flags,
				HB_Buffer        buffer,
				HB_Bool           dvi,
				HB_Bool           r2l );

HB_END_HEADER

#endif /* HARFBUZZ_GPOS_H */

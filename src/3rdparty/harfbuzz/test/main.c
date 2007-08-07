/*******************************************************************
 *
 *  Copyright 2006  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <harfbuzz-shape.h>
#include <harfbuzz.h>

static FT_Library freetype;

static FT_Face
loadFace(const char *fontPattern)
{
    FcPattern *pattern;
    FcPattern *match;
    FcFontSet *fontset;
    FcResult  result;
    FcChar8   *file;
    int       index;
    FT_Face   face;

    FT_Init_FreeType (&freetype);
    FcInit ();

    pattern = FcNameParse (fontPattern);
    FcConfigSubstitute (0, pattern, FcMatchPattern);
    FcDefaultSubstitute (pattern);

    match = FcFontMatch (0, pattern, &result);
    if (!match) {
        fprintf(stderr, "Cannot find font for pattern %s\n", fontPattern);
        exit(1);
    }

    fontset = FcFontSetCreate ();
    FcFontSetAdd (fontset, match);

    if (fontset->nfont < 1) {
        fprintf(stderr, "Fontset is empty?!\n");
        exit(1);
    }

    if (FcPatternGetString (fontset->fonts[0], FC_FILE, 0, &file) != FcResultMatch) {
        fprintf(stderr, "Cannot get font filename\n");
        exit(1);
    }

    if (FcPatternGetInteger (fontset->fonts[0], FC_INDEX, 0, &index) != FcResultMatch) {
        fprintf(stderr, "Cannot get index in font\n");
        exit(1);
    }

    if (FT_New_Face (freetype, file, index, &face)) {
        fprintf(stderr, "Cannot open font\n");
        exit(1);
    }

    FcPatternDestroy (pattern);
    FcFontSetDestroy (fontset);

    return face;
}

typedef struct {
    HB_GDEF gdef;
    HB_GSUB gsub;
} HB_OpenType_Helper;

typedef struct {
    FT_UInt tag;
    FT_UInt property;
} HB_OpenType_ScriptFeature;

FT_Error
hb_opentype_helper_new(FT_Face face, HB_OpenType_Helper *helper)
{
    HB_Load_GDEF_Table (face, &helper->gdef);
    HB_Load_GSUB_Table (face, &helper->gsub, helper->gdef);

    return FT_Err_Ok;
}

FT_Error
hb_opentype_helper_select_script(HB_OpenType_Helper *helper, FT_UInt tag, HB_OpenType_ScriptFeature *features)
{
    FT_UShort scriptIndex = 0;
    FT_UShort featureIndex = 0;
    FT_Error error;
    HB_OpenType_ScriptFeature *feature;
    int i;

    error = HB_GSUB_Select_Script (helper->gsub, tag, &scriptIndex);
    if (error)
        return error;

    for (i = 0; features[i].tag; ++i) {
        if (!HB_GSUB_Select_Feature (helper->gsub, features[i].tag, scriptIndex, 0xffff, &featureIndex)) {
            HB_GSUB_Add_Feature (helper->gsub, featureIndex, features[i].property);
        }
    }

    return FT_Err_Ok;
}

FT_Error
hb_opentype_helper_shape(HB_OpenType_Helper *helper, HB_Buffer buffer)
{
    FT_Error error = HB_GSUB_Apply_String (helper->gsub, buffer);
    if (error && error != HB_Err_Not_Covered)
        return error;
    return FT_Err_Ok;
}

FT_Error
hb_opentype_helper_free(HB_OpenType_Helper *helper)
{
    if (helper->gsub)
        HB_Done_GSUB_Table (helper->gsub);
    if (helper->gdef)
        HB_Done_GDEF_Table (helper->gdef);
}

typedef enum {
    CcmpProperty = 0x1,
    InitProperty = 0x2,
    IsolProperty = 0x4,
    FinaProperty = 0x8,
    MediProperty = 0x10,
    RligProperty = 0x20,
    CaltProperty = 0x40,
    LigaProperty = 0x80,
    DligProperty = 0x100,
    CswhProperty = 0x200,
    MsetProperty = 0x400
} ArabicProperty;

static HB_OpenType_ScriptFeature arabicGSubFeatures[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('i', 's', 'o', 'l'), IsolProperty },
    { FT_MAKE_TAG('f', 'i', 'n', 'a'), FinaProperty },
    { FT_MAKE_TAG('m', 'e', 'd', 'i'), MediProperty },
    { FT_MAKE_TAG('i', 'n', 'i', 't'), InitProperty },
    { FT_MAKE_TAG('r', 'l', 'i', 'g'), RligProperty },
    { FT_MAKE_TAG('c', 'a', 'l', 't'), CaltProperty },
    { FT_MAKE_TAG('l', 'i', 'g', 'a'), LigaProperty },
    { FT_MAKE_TAG('d', 'l', 'i', 'g'), DligProperty },
    { FT_MAKE_TAG('c', 's', 'w', 'h'), CswhProperty },
    { 0, 0 }
};

static void
shapetest(FT_Face face)
{
    HB_OpenType_Helper helper;
    HB_Buffer buffer;
    int i;

    hb_opentype_helper_new (face, &helper);

    hb_opentype_helper_select_script (&helper, FT_MAKE_TAG ('a', 'r', 'a', 'b'), arabicGSubFeatures);

    hb_buffer_new (face->memory, &buffer);

    /* lam */
    if (hb_buffer_add_glyph (buffer,
                             FT_Get_Char_Index (face, 0x0644),
                             /*properties*/ IsolProperty|MediProperty|FinaProperty,
                             /*cluster*/ 0)) {
        fprintf (stderr, "add_glyph failed?!\n");
        return;
    }

    /* alef */
    if (hb_buffer_add_glyph (buffer,
                             FT_Get_Char_Index (face, 0x0627),
                             /*properties*/ IsolProperty|MediProperty|InitProperty,
                             /*cluster*/ 1)) {
        fprintf (stderr, "add_glyph[2] failed?!\n");
        return;
    }

    printf ("input:\n");
    for (i = 0; i < buffer->in_length; ++i)
        printf ("i %d glyph %4x\n", i, buffer->in_string[i].gindex);

    if (hb_opentype_helper_shape (&helper, buffer)) {
        fprintf(stderr, "harfbuzz gsub error\n");
        return;
    }

    printf ("output:\n");
    for (i = 0; i < buffer->in_length; ++i)
        printf ("i %d glyph %4x \n", i, buffer->in_string[i].gindex);

    hb_opentype_helper_free (&helper);
    hb_buffer_free (buffer);
}

int
main()
{
    FT_Face face;
    face = loadFace ("DejaVu Sans");

    shapetest (face);

    FT_Done_Face (face);
    FT_Done_FreeType (freetype);
    FcFini ();
    return 0;
}

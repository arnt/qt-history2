/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qopentype_p.h"
#include "qfontengine_p.h"
#include "qscriptengine_p.h"

#ifndef QT_NO_OPENTYPE
//  --------------------------------------------------------------------------------------------------------------------
// Open type support
//  --------------------------------------------------------------------------------------------------------------------

//#define OT_DEBUG

#ifdef OT_DEBUG
static inline char *tag_to_string(FT_ULong tag)
{
    static char string[5];
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
    return string;
}
#endif

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG('D', 'F', 'L', 'T')

enum {
    RequiresGsub = 1,
    RequiresGpos = 2
};

struct OTScripts {
    unsigned int tag;
    int flags;
};
static const OTScripts ot_scripts [] = {
    // Common
    { FT_MAKE_TAG('D', 'F', 'L', 'T'), 0 },
    // Hebrew
    { FT_MAKE_TAG('h', 'e', 'b', 'r'), 1 },
    // Arabic
    { FT_MAKE_TAG('a', 'r', 'a', 'b'), 1 },
    // Syriac
    { FT_MAKE_TAG('s', 'y', 'r', 'c'), 1 },
    // Thaana
    { FT_MAKE_TAG('t', 'h', 'a', 'a'), 1 },
    // Devanagari
    { FT_MAKE_TAG('d', 'e', 'v', 'a'), 1 },
    // Bengali
    { FT_MAKE_TAG('b', 'e', 'n', 'g'), 1 },
    // Gurmukhi
    { FT_MAKE_TAG('g', 'u', 'r', 'u'), 1 },
    // Gujarati
    { FT_MAKE_TAG('g', 'u', 'j', 'r'), 1 },
    // Oriya
    { FT_MAKE_TAG('o', 'r', 'y', 'a'), 1 },
    // Tamil
    { FT_MAKE_TAG('t', 'a', 'm', 'l'), 1 },
    // Telugu
    { FT_MAKE_TAG('t', 'e', 'l', 'u'), 1 },
    // Kannada
    { FT_MAKE_TAG('k', 'n', 'd', 'a'), 1 },
    // Malayalam
    { FT_MAKE_TAG('m', 'l', 'y', 'm'), 1 },
    // Sinhala
    // ### could not find any OT specs on this
    { FT_MAKE_TAG('s', 'i', 'n', 'h'), 1 },
    // Thai
    { FT_MAKE_TAG('t', 'h', 'a', 'i'), 1 },
    // Lao
    { FT_MAKE_TAG('l', 'a', 'o', ' '), 1 },
    // Tibetan
    { FT_MAKE_TAG('t', 'i', 'b', 't'), 1 },
    // Myanmar
    { FT_MAKE_TAG('m', 'y', 'm', 'r'), 1 },
    // Hangul
    { FT_MAKE_TAG('h', 'a', 'n', 'g'), 1 },
    // Khmer
    { FT_MAKE_TAG('k', 'h', 'm', 'r'), 1 },
};

QOpenType::QOpenType(QFontEngine *fe, FT_Face _face)
    : fontEngine(fe), face(_face), gdef(0), gsub(0), gpos(0), current_script(0)
{
    hb_buffer_new(face->memory, &hb_buffer);
    tmpAttributes = 0;
    tmpLogClusters = 0;

    FT_Error error;
    if ((error = HB_Load_GDEF_Table(face, &gdef))) {
#ifdef OT_DEBUG
        qDebug("error loading gdef table: %d", error);
#endif
        gdef = 0;
    }

    if ((error = HB_Load_GSUB_Table(face, &gsub, gdef))) {
        gsub = 0;
#ifdef OT_DEBUG
        if (error != FT_Err_Table_Missing) {
            qDebug("error loading gsub table: %d", error);
        } else {
            qDebug("face doesn't have a gsub table");
        }
#endif
    }

    if ((error = HB_Load_GPOS_Table(face, &gpos, gdef))) {
        gpos = 0;
#ifdef OT_DEBUG
        qDebug("error loading gpos table: %d", error);
#endif
    }

    for (uint i = 0; i < QUnicodeTables::ScriptCount; ++i)
        supported_scripts[i] = checkScript(i);
}

QOpenType::~QOpenType()
{
    if (gpos)
        HB_Done_GPOS_Table(gpos);
    if (gsub)
        HB_Done_GSUB_Table(gsub);
    if (gdef)
        HB_Done_GDEF_Table(gdef);
    if (hb_buffer)
        hb_buffer_free(hb_buffer);
    if (tmpAttributes)
        free(tmpAttributes);
    if (tmpLogClusters)
        free(tmpLogClusters);
}

bool QOpenType::checkScript(unsigned int script)
{
    assert(script < QUnicodeTables::ScriptCount);

    uint tag = ot_scripts[script].tag;
    int requirements = ot_scripts[script].flags;

    if (requirements & RequiresGsub) {
        if (!gsub)
            return false;

        FT_UShort script_index;
        FT_Error error = HB_GSUB_Select_Script(gsub, tag, &script_index);
        if (error) {
#ifdef OT_DEBUG
            qDebug("could not select script %d in GSub table: %d", (int)script, error);
#endif
            return false;
        }
    }

    if (requirements & RequiresGpos) {
        if (!gpos)
            return false;

        FT_UShort script_index;
        FT_Error error = HB_GPOS_Select_Script(gpos, script, &script_index);
        if (error) {
#ifdef OT_DEBUG
            qDebug("could not select script in gpos table: %d", error);
#endif
            return false;
        }

    }
    return true;
}


void QOpenType::selectScript(unsigned int script, const Features *features)
{
    if (current_script == script)
        return;

    assert(script < QUnicodeTables::ScriptCount);
    // find script in our list of supported scripts.
    uint tag = ot_scripts[script].tag;

    if (gsub && features) {
#ifdef OT_DEBUG
        {
            TTO_FeatureList featurelist = gsub->FeatureList;
            int numfeatures = featurelist.FeatureCount;
            qDebug("gsub table has %d features", numfeatures);
            for(int i = 0; i < numfeatures; i++) {
                TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
                qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
            }
        }
#endif
        HB_GSUB_Clear_Features(gsub);
        FT_UShort script_index;
        FT_Error error = HB_GSUB_Select_Script(gsub, tag, &script_index);
        if (!error) {
#ifdef OT_DEBUG
            qDebug("script %s has script index %d", tag_to_string(script), script_index);
#endif
            while (features->tag) {
                FT_UShort feature_index;
                error = HB_GSUB_Select_Feature(gsub, features->tag, script_index, 0xffff, &feature_index);
                if (!error) {
#ifdef OT_DEBUG
                    qDebug("  adding feature %s", tag_to_string(features->tag));
#endif
                    HB_GSUB_Add_Feature(gsub, feature_index, features->property);
                }
                ++features;
            }
        }
    }

    if (gpos) {
        HB_GPOS_Clear_Features(gpos);
        FT_UShort script_index;
        FT_Error error = HB_GPOS_Select_Script(gpos, tag, &script_index);
        if (!error) {
#ifdef OT_DEBUG
            {
                TTO_FeatureList featurelist = gpos->FeatureList;
                int numfeatures = featurelist.FeatureCount;
                qDebug("gpos table has %d features", numfeatures);
                for(int i = 0; i < numfeatures; i++) {
                    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
                    FT_UShort feature_index;
                    HB_GPOS_Select_Feature(gpos, r->FeatureTag, script_index, 0xffff, &feature_index);
                    qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
                }
            }
#endif
            FT_ULong *feature_tag_list;
            error = HB_GPOS_Query_Features(gpos, script_index, 0xffff, &feature_tag_list);
            if (!error) {
                while (*feature_tag_list) {
                    FT_UShort feature_index;
                    error = HB_GPOS_Select_Feature(gpos, *feature_tag_list, script_index, 0xffff, &feature_index);
                    if (!error)
                        HB_GPOS_Add_Feature(gpos, feature_index, PositioningProperties);
                    ++feature_tag_list;
                }
            }
        }
    }

    current_script = script;
}

#ifdef OT_DEBUG
static void dump_string(HB_Buffer buffer)
{
    for (uint i = 0; i < buffer->in_length; ++i) {
        qDebug("    %x: cluster=%d", buffer->in_string[i].gindex, buffer->in_string[i].cluster);
    }
}
#endif

extern void qt_heuristicPosition(QShaperItem *item);

bool QOpenType::shape(QShaperItem *item, const unsigned int *properties)
{
    length = item->num_glyphs;

    hb_buffer_clear(hb_buffer);

    tmpAttributes = (QGlyphLayout::Attributes *) realloc(tmpAttributes, length*sizeof(QGlyphLayout::Attributes));
    tmpLogClusters = (unsigned int *) realloc(tmpLogClusters, length*sizeof(unsigned int));
    for (int i = 0; i < length; ++i) {
        hb_buffer_add_glyph(hb_buffer, item->glyphs[i].glyph, properties ? properties[i] : 0, i);
        tmpAttributes[i] = item->glyphs[i].attributes;
        tmpLogClusters[i] = item->log_clusters[i];
    }

#ifdef OT_DEBUG
    qDebug("-----------------------------------------");
//     qDebug("log clusters before shaping:");
//     for (int j = 0; j < length; j++)
//         qDebug("    log[%d] = %d", j, item->log_clusters[j]);
    qDebug("original glyphs: %p", item->glyphs);
    for (int i = 0; i < length; ++i)
        qDebug("   glyph=%4x", hb_buffer->in_string[i].gindex);
//     dump_string(hb_buffer);
#endif

    loadFlags = item->flags & QTextEngine::DesignMetrics ? FT_LOAD_NO_HINTING : FT_LOAD_DEFAULT;

    if (gsub) {
        uint error = HB_GSUB_Apply_String(gsub, hb_buffer);
        if (error && error != TTO_Err_Not_Covered)
            return false;
    }

#ifdef OT_DEBUG
//     qDebug("log clusters before shaping:");
//     for (int j = 0; j < length; j++)
//         qDebug("    log[%d] = %d", j, item->log_clusters[j]);
    qDebug("shaped glyphs:");
    for (int i = 0; i < length; ++i)
        qDebug("   glyph=%4x", hb_buffer->in_string[i].gindex);
    qDebug("-----------------------------------------");
//     dump_string(hb_buffer);
#endif

    return true;
}

bool QOpenType::positionAndAdd(QShaperItem *item, int availableGlyphs, bool doLogClusters)
{
    if (gpos) {
#ifdef Q_WS_X11
        Q_ASSERT(fontEngine->type() == QFontEngine::Freetype);
        face = static_cast<QFontEngineFT *>(fontEngine)->lockFace();
#endif
        memset(hb_buffer->positions, 0, hb_buffer->in_length*sizeof(HB_PositionRec));
        // #### check that passing "false,false" is correct
        HB_GPOS_Apply_String(face, gpos, loadFlags, hb_buffer, false, false);
#ifdef Q_WS_X11
        static_cast<QFontEngineFT *>(fontEngine)->unlockFace();
#endif
    }

    // make sure we have enough space to write everything back
    if (availableGlyphs < (int)hb_buffer->in_length) {
        item->num_glyphs = hb_buffer->in_length;
        return false;
    }

    QGlyphLayout *glyphs = item->glyphs;

    for (unsigned int i = 0; i < hb_buffer->in_length; ++i) {
        glyphs[i].glyph = hb_buffer->in_string[i].gindex;
        glyphs[i].attributes = tmpAttributes[hb_buffer->in_string[i].cluster];
        if (i && hb_buffer->in_string[i].cluster == hb_buffer->in_string[i-1].cluster)
            glyphs[i].attributes.clusterStart = false;
    }
    item->num_glyphs = hb_buffer->in_length;

    if (doLogClusters) {
        // we can't do this for indic, as we pass the stuf in syllables and it's easier to do it in the shaper.
        unsigned short *logClusters = item->log_clusters;
        int clusterStart = 0;
        int oldCi = 0;
        for (unsigned int i = 0; i < hb_buffer->in_length; ++i) {
            int ci = hb_buffer->in_string[i].cluster;
            //         qDebug("   ci[%d] = %d mark=%d, cmb=%d, cs=%d",
            //                i, ci, glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart);
            if (!glyphs[i].attributes.mark && glyphs[i].attributes.clusterStart && ci != oldCi) {
                for (int j = oldCi; j < ci; j++)
                    logClusters[j] = clusterStart;
                clusterStart = i;
                oldCi = ci;
            }
        }
        for (int j = oldCi; j < length; j++)
            logClusters[j] = clusterStart;
    }

    // calulate the advances for the shaped glyphs
//     qDebug("unpositioned: ");
    item->font->recalcAdvances(item->num_glyphs, glyphs, QFlag(item->flags));

    // positioning code:
    if (gpos) {
        HB_Position positions = hb_buffer->positions;

//         qDebug("positioned glyphs:");
        for (unsigned int i = 0; i < hb_buffer->in_length; i++) {
//             qDebug("    %d:\t orig advance: (%d/%d)\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
//                    glyphs[i].advance.x.toInt(), glyphs[i].advance.y.toInt(),
//                    (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6),
//                    (int)(positions[i].x_pos >> 6), (int)(positions[i].y_pos >> 6),
//                    positions[i].back, positions[i].new_advance);
            // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
            if (positions[i].new_advance) {
                glyphs[i].advance.x = QFixed::fromFixed(item->flags & QTextEngine::RightToLeft
                                          ? -positions[i].x_advance : positions[i].x_advance);
                glyphs[i].advance.y = QFixed::fromFixed(-positions[i].y_advance);
            } else {
                glyphs[i].advance.x += QFixed::fromFixed(item->flags & QTextEngine::RightToLeft
                                           ? -positions[i].x_advance : positions[i].x_advance);
                glyphs[i].advance.y -= QFixed::fromFixed(positions[i].y_advance);
            }
            glyphs[i].offset.x = QFixed::fromFixed(positions[i].x_pos);
            glyphs[i].offset.y = QFixed::fromFixed(-positions[i].y_pos);
            int back = positions[i].back;
            if (item->flags & QTextEngine::RightToLeft) {
                while (back--) {
                    glyphs[i].offset.x -= glyphs[i-back].advance.x;
                    glyphs[i].offset.y -= -glyphs[i-back].advance.y;
                }
            } else {
                while (back) {
                    glyphs[i].offset.x -= glyphs[i-back].advance.x;
                    glyphs[i].offset.y -= -glyphs[i-back].advance.y;
                    --back;
                }
            }
//             qDebug("   ->\tadv=%d\tpos=(%d/%d)",
//                    glyphs[i].advance.x.toInt(), glyphs[i].offset.x.toInt(), glyphs[i].offset.y.toInt());
        }
    } else {
        qt_heuristicPosition(item);
    }

#ifdef OT_DEBUG
    if (doLogClusters) {
        qDebug("log clusters after shaping:");
        for (int j = 0; j < length; j++)
            qDebug("    log[%d] = %d", j, item->log_clusters[j]);
    }
    qDebug("final glyphs:");
    for (int i = 0; i < (int)hb_buffer->in_length; ++i)
        qDebug("   glyph=%4x char_index=%d mark: %d cmp: %d, clusterStart: %d advance=%d/%d offset=%d/%d",
               glyphs[i].glyph, hb_buffer->in_string[i].cluster, glyphs[i].attributes.mark,
               glyphs[i].attributes.combiningClass, glyphs[i].attributes.clusterStart,
               glyphs[i].advance.x.toInt(), glyphs[i].advance.y.toInt(),
               glyphs[i].offset.x.toInt(), glyphs[i].offset.y.toInt());
    qDebug("-----------------------------------------");
#endif
    return true;
}

#endif // QT_NO_FREETYPE

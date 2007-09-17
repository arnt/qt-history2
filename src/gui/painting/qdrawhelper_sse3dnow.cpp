/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qdrawhelper_x86_p.h>

#if defined(QT_HAVE_3DNOW) && defined(QT_HAVE_SSE)

#include <private/qdrawhelper_sse_p.h>
#include <mm3dnow.h>

QT_BEGIN_NAMESPACE

struct QSSE3DNOWIntrinsics : public QSSEIntrinsics
{
    static inline void end() {
        _m_femms();
    }
};

CompositionFunctionSolid qt_functionForModeSolid_SSE3DNOW[numCompositionFunctions] = {
    comp_func_solid_SourceOver<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationOver<QSSE3DNOWIntrinsics>,
    comp_func_solid_Clear<QSSE3DNOWIntrinsics>,
    comp_func_solid_Source<QSSE3DNOWIntrinsics>,
    0,
    comp_func_solid_SourceIn<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationIn<QSSE3DNOWIntrinsics>,
    comp_func_solid_SourceOut<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationOut<QSSE3DNOWIntrinsics>,
    comp_func_solid_SourceAtop<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationAtop<QSSE3DNOWIntrinsics>,
    comp_func_solid_XOR<QSSE3DNOWIntrinsics>
};

CompositionFunction qt_functionForMode_SSE3DNOW[numCompositionFunctions] = {
    comp_func_SourceOver<QSSE3DNOWIntrinsics>,
    comp_func_DestinationOver<QSSE3DNOWIntrinsics>,
    comp_func_Clear<QSSE3DNOWIntrinsics>,
    comp_func_Source<QSSE3DNOWIntrinsics>,
    0,
    comp_func_SourceIn<QSSE3DNOWIntrinsics>,
    comp_func_DestinationIn<QSSE3DNOWIntrinsics>,
    comp_func_SourceOut<QSSE3DNOWIntrinsics>,
    comp_func_DestinationOut<QSSE3DNOWIntrinsics>,
    comp_func_SourceAtop<QSSE3DNOWIntrinsics>,
    comp_func_DestinationAtop<QSSE3DNOWIntrinsics>,
    comp_func_XOR<QSSE3DNOWIntrinsics>
};

void qt_blend_color_argb_sse3dnow(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QSSE3DNOWIntrinsics>(count, spans, userData,
                                                 (CompositionFunctionSolid*)qt_functionForModeSolid_SSE3DNOW);
}

void qt_memfill32_sse3dnow(quint32 *dest, quint32 value, int count)
{
    return qt_memfill32_sse_template<QSSE3DNOWIntrinsics>(dest, value, count);
}


void qt_bitmapblit16_sse3dnow(QRasterBuffer *rasterBuffer, int x, int y,
                              quint32 color,
                              const uchar *src,
                              int width, int height, int stride)
{
    return qt_bitmapblit16_sse_template<QSSE3DNOWIntrinsics>(rasterBuffer, x,y,
                                                             color, src, width,
                                                             height, stride);
}

QT_END_NAMESPACE

#endif // QT_HAVE_3DNOW && QT_HAVE_SSE

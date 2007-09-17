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

#ifdef QT_HAVE_SSE

#include <private/qdrawhelper_sse_p.h>

QT_BEGIN_NAMESPACE

CompositionFunctionSolid qt_functionForModeSolid_SSE[numCompositionFunctions] = {
    comp_func_solid_SourceOver<QSSEIntrinsics>,
    comp_func_solid_DestinationOver<QSSEIntrinsics>,
    comp_func_solid_Clear<QSSEIntrinsics>,
    comp_func_solid_Source<QSSEIntrinsics>,
    0,
    comp_func_solid_SourceIn<QSSEIntrinsics>,
    comp_func_solid_DestinationIn<QSSEIntrinsics>,
    comp_func_solid_SourceOut<QSSEIntrinsics>,
    comp_func_solid_DestinationOut<QSSEIntrinsics>,
    comp_func_solid_SourceAtop<QSSEIntrinsics>,
    comp_func_solid_DestinationAtop<QSSEIntrinsics>,
    comp_func_solid_XOR<QSSEIntrinsics>
};

CompositionFunction qt_functionForMode_SSE[numCompositionFunctions] = {
    comp_func_SourceOver<QSSEIntrinsics>,
    comp_func_DestinationOver<QSSEIntrinsics>,
    comp_func_Clear<QSSEIntrinsics>,
    comp_func_Source<QSSEIntrinsics>,
    0,
    comp_func_SourceIn<QSSEIntrinsics>,
    comp_func_DestinationIn<QSSEIntrinsics>,
    comp_func_SourceOut<QSSEIntrinsics>,
    comp_func_DestinationOut<QSSEIntrinsics>,
    comp_func_SourceAtop<QSSEIntrinsics>,
    comp_func_DestinationAtop<QSSEIntrinsics>,
    comp_func_XOR<QSSEIntrinsics>
};

void qt_blend_color_argb_sse(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QSSEIntrinsics>(count, spans, userData,
                                            (CompositionFunctionSolid*)qt_functionForModeSolid_SSE);
}

void qt_memfill32_sse(quint32 *dest, quint32 value, int count)
{
    return qt_memfill32_sse_template<QSSEIntrinsics>(dest, value, count);
}

void qt_bitmapblit16_sse(QRasterBuffer *rasterBuffer, int x, int y,
                         quint32 color,
                         const uchar *src,
                         int width, int height, int stride)
{
    return qt_bitmapblit16_sse_template<QSSEIntrinsics>(rasterBuffer, x,y,
                                                        color, src, width,
                                                        height, stride);
}

QT_END_NAMESPACE

#endif // QT_HAVE_SSE

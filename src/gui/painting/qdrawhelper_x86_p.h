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

#ifndef QDRAWHELPER_X86_P_H
#define QDRAWHELPER_X86_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifdef QT_HAVE_SSE2
void qt_memfill32_sse2(quint32 *dest, quint32 value, int count);
void qt_memfill16_sse2(quint16 *dest, quint16 value, int count);
void qt_bitmapblit32_sse2(QRasterBuffer *rasterBuffer, int x, int y,
                          quint32 color,
                          const uchar *src, int width, int height, int stride);
void qt_bitmapblit16_sse2(QRasterBuffer *rasterBuffer, int x, int y,
                          quint32 color,
                          const uchar *src, int width, int height, int stride);
#endif // SSE2

#ifdef QT_HAVE_SSE
void qt_memfill32_sse(quint32 *dest, quint32 value, int count);
void qt_bitmapblit16_sse(QRasterBuffer *rasterBuffer, int x, int y,
                         quint32 color,
                         const uchar *src, int width, int height, int stride);

extern const CompositionFunction qt_functionForMode_SSE[];
extern const CompositionFunctionSolid qt_functionForModeSolid_SSE[];
void qt_blend_color_argb_sse(int count, const QSpan *spans, void *userData);

#ifdef QT_HAVE_3DNOW
void qt_memfill32_sse3dnow(quint32 *dest, quint32 value, int count);
void qt_bitmapblit16_sse3dnow(QRasterBuffer *rasterBuffer, int x, int y,
                              quint32 color,
                              const uchar *src, int width, int height,
                              int stride);
extern const CompositionFunction qt_functionForMode_SSE3DNOW[];
extern const CompositionFunctionSolid qt_functionForModeSolid_SSE3DNOW[];
void qt_blend_color_argb_sse3dnow(int count, const QSpan *spans, void *userData);
#endif

#endif // SSE

#ifdef QT_HAVE_MMX
extern const CompositionFunction qt_functionForMode_MMX[];
extern const CompositionFunctionSolid qt_functionForModeSolid_MMX[];
void qt_blend_color_argb_mmx(int count, const QSpan *spans, void *userData);

#ifdef QT_HAVE_3DNOW
extern const CompositionFunction qt_functionForMode_MMX3DNOW[];
extern const CompositionFunctionSolid qt_functionForModeSolid_MMX3DNOW[];
void qt_blend_color_argb_mmx3dnow(int count, const QSpan *spans, void *userData);
#endif

#endif // MMX

#endif // QDRAWHELPER_X86_P_H

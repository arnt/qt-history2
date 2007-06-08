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

#include "private/qmemrotate_p.h"

#if QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
static const int tileSize = 32;
#endif

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#if QT_ROTATION_ALGORITHM == QT_ROTATION_PACKED || QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
#error Big endian version not implemented for the transformed driver!
#endif
#endif

#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD || defined(QT_QWS_DEPTH_18) || defined(QT_QWS_DEPTH_24)

template <class DST, class SRC>
static inline void qt_memrotate90_cachedRead(const SRC *src, int w, int h,
                                             int sstride,
                                             DST *dest, int dstride)
{
    for (int y = 0; y < h; ++y) {
        for (int x = w - 1; x >= 0; --x) {
            dest[(w - x - 1) * dstride + y] = qt_colorConvert<DST,SRC>(src[x], 0);
        }
        src += sstride;
    }
}

template <class DST, class SRC>
static inline void qt_memrotate270_cachedRead(const SRC *src, int w, int h,
                                              int sstride,
                                              DST *dest, int dstride)
{
    src += (h - 1) * sstride;
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            dest[x * dstride + h - y - 1] = qt_colorConvert<DST,SRC>(src[x], 0);
        }
        src -= sstride;
    }
}

#endif // QT_ROTATION_CACHEDREAD

#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE

template <class DST, class SRC>
static inline void qt_memrotate90_cachedWrite(const SRC *src, int w, int h,
                                              int sstride,
                                              DST *dest, int dstride)
{
    for (int x = w - 1; x >= 0; --x) {
        DST *d = dest + (w - x - 1) * dstride;
        for (int y = 0; y < h; ++y) {
            *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
        }
    }

}

template <class DST, class SRC>
static inline void qt_memrotate270_cachedWrite(const SRC *src, int w, int h,
                                               int sstride,
                                               DST *dest, int dstride)
{
    for (int x = 0; x < w; ++x) {
        DST *d = dest + x * dstride;
        for (int y = h - 1; y >= 0; --y) {
            *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
        }
    }
}

#endif // QT_ROTATION_CACHEDWRITE

#if QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING

// TODO: packing algorithms should probably be modified on 64-bit architectures

template <class DST, class SRC>
static inline void qt_memrotate90_packing(const SRC *src, int w, int h,
                                          int sstride,
                                          DST *dest, int dstride)
{
    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = int((long(dest) & (sizeof(quint32)-1))) / sizeof(DST);

    for (int x = w - 1; x >= 0; --x) {
        int y = 0;

        for (int i = 0; i < unaligned; ++i) {
            dest[(w - x - 1) * dstride + y]
                = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            ++y;
        }

        quint32 *d = reinterpret_cast<quint32*>(dest + (w - x - 1) * dstride
                                                + unaligned);
        const int rest = (h - unaligned) % pack;
        while (y < h - rest) {
            quint32 c = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            for (int i = 1; i < pack; ++i) {
                c |= qt_colorConvert<DST,SRC>(src[(y + i) * sstride + x], 0)
                     << (sizeof(int) * 8 / pack * i);
            }
            *d++ = c;
            y += pack;
        }

        while (y < h) {
            dest[(w - x - 1) * dstride + y]
                = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            ++y;
        }
    }
}

template <class DST, class SRC>
static inline void qt_memrotate270_packing(const SRC *src, int w, int h,
                                           int sstride,
                                           DST *dest, int dstride)
{
    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = int((long(dest) & (sizeof(quint32)-1))) / sizeof(DST);

    for (int x = 0; x < w; ++x) {
        int y = h - 1;

        for (int i = 0; i < unaligned; ++i) {
            dest[x * dstride + h - y - 1]
                = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            --y;
        }

        quint32 *d = reinterpret_cast<quint32*>(dest + x * dstride
                                                + unaligned);
        const int rest = (h - unaligned) % pack;
        while (y > rest) {
            quint32 c = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            for (int i = 1; i < pack; ++i) {
                c |= qt_colorConvert<DST,SRC>(src[(y - i) * sstride + x], 0)
                     << (sizeof(int) * 8 / pack * i);
            }
            *d++ = c;
            y -= pack;
        }
        while (y >= 0) {
            dest[x * dstride + h - y - 1]
                = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            --y;
        }
    }
}

#endif // QT_ROTATION_PACKING

#if QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
template <class DST, class SRC>
static inline void qt_memrotate90_tiled(const SRC *src, int w, int h,
                                        int sstride,
                                        DST *dest, int dstride)
{
    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned =
        qMin(uint((long(dest) & (sizeof(quint32)-1)) / sizeof(DST)), uint(h));
    const int restX = w % tileSize;
    const int restY = (h - unaligned) % tileSize;
    const int unoptimizedY = restY % pack;
    const int numTilesX = w / tileSize + (restX > 0);
    const int numTilesY = (h - unaligned) / tileSize + (restY >= pack);

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = w - tx * tileSize - 1;
        const int stopx = qMax(startx - tileSize, 0);

        if (unaligned) {
            for (int x = startx; x >= stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride;
                for (int y = 0; y < unaligned; ++y) {
                    *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
                }
            }
        }

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = ty * tileSize + unaligned;
            const int stopy = qMin(starty + tileSize, h - unoptimizedY);

            for (int x = startx; x >= stopx; --x) {
                quint32 *d = reinterpret_cast<quint32*>(dest + (w - x - 1) * dstride + starty);
                for (int y = starty; y < stopy; y += pack) {
                    quint32 c = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
                    for (int i = 1; i < pack; ++i) {
                        const int shift = (sizeof(int) * 8 / pack * i);
                        const DST color = qt_colorConvert<DST,SRC>(src[(y + i) * sstride + x], 0);
                        c |= color << shift;
                    }
                    *d++ = c;
                }
            }
        }

        if (unoptimizedY) {
            const int starty = h - unoptimizedY;
            for (int x = startx; x >= stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride + starty;
                for (int y = starty; y < h; ++y) {
                    *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
                }
            }
        }
    }
}

template <class DST, class SRC>
static inline void qt_memrotate90_tiled_unpacked(const SRC *src, int w, int h,
                                                 int sstride,
                                                 DST *dest, int dstride)
{
    const int numTilesX = (w + tileSize - 1) / tileSize;
    const int numTilesY = (h + tileSize - 1) / tileSize;

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = w - tx * tileSize - 1;
        const int stopx = qMax(startx - tileSize, 0);

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = ty * tileSize;
            const int stopy = qMin(starty + tileSize, h);

            for (int x = startx; x >= stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride + starty;
                for (int y = starty; y < stopy; ++y)
                    *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            }
        }
    }
}

template <class DST, class SRC>
static inline void qt_memrotate270_tiled(const SRC *src, int w, int h,
                                         int sstride,
                                         DST *dest, int dstride)
{
    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned =
        qMin(uint((long(dest) & (sizeof(quint32)-1)) / sizeof(DST)), uint(h));
    const int restX = w % tileSize;
    const int restY = (h - unaligned) % tileSize;
    const int unoptimizedY = restY % pack;
    const int numTilesX = w / tileSize + (restX > 0);
    const int numTilesY = (h - unaligned) / tileSize + (restY >= pack);

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = tx * tileSize;
        const int stopx = qMin(startx + tileSize, w);

        if (unaligned) {
            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride;
                for (int y = h - 1; y >= h - unaligned; --y) {
                    *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
                }
            }
        }

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = h - 1 - unaligned - ty * tileSize;
            const int stopy = qMax(starty - tileSize, unoptimizedY);

            for (int x = startx; x < stopx; ++x) {
                quint32 *d = reinterpret_cast<quint32*>(dest + x * dstride
                                                        + h - 1 - starty);
                for (int y = starty; y > stopy; y -= pack) {
                    quint32 c = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
                    for (int i = 1; i < pack; ++i) {
                        const int shift = (sizeof(int) * 8 / pack * i);
                        const DST color = qt_colorConvert<DST,SRC>(src[(y - i) * sstride + x], 0);
                        c |= color << shift;
                    }
                    *d++ = c;
                }
            }
        }
        if (unoptimizedY) {
            const int starty = unoptimizedY - 1;
            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride + h - 1 - starty;
                for (int y = starty; y >= 0; --y) {
                    *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
                }
            }
        }
    }
}

template <class DST, class SRC>
static inline void qt_memrotate270_tiled_unpacked(const SRC *src, int w, int h,
                                                  int sstride,
                                                  DST *dest, int dstride)
{
    const int numTilesX = (w + tileSize - 1) / tileSize;
    const int numTilesY = (h + tileSize - 1) / tileSize;

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = tx * tileSize;
        const int stopx = qMin(startx + tileSize, w);

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = h - 1 - ty * tileSize;
            const int stopy = qMax(starty - tileSize, 0);

            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride + h - 1 - starty;
                for (int y = starty; y >= stopy; --y)
                    *d++ = qt_colorConvert<DST,SRC>(src[y * sstride + x], 0);
            }
        }
    }
}

#endif // QT_ROTATION_ALFORITHM

template <class DST, class SRC>
static inline void qt_memrotate90_template(const SRC *src,
                                           int srcWidth, int srcHeight, int srcStride,
                                           DST *dest, int dstStride)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    qt_memrotate90_cachedRead<DST,SRC>(src, srcWidth, srcHeight, srcStride,
                                       dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    qt_memrotate90_cachedWrite<DST,SRC>(src, srcWidth, srcHeight, srcStride,
                                        dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
    qt_memrotate90_packing<DST,SRC>(src, srcWidth, srcHeight, srcStride,
                                    dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    qt_memrotate90_tiled<DST,SRC>(src, srcWidth, srcHeight, srcStride,
                                  dest, dstStride);
#endif
}

template <class DST, class SRC>
static inline void qt_memrotate180_template(const SRC *src,
                                            int w, int h, int sstride,
                                            DST *dest, int dstride)
{
    src += (h - 1) * sstride;
    for (int y = h - 1; y >= 0; --y) {
        for (int x = w - 1; x >= 0; --x) {
            dest[(h - y - 1) * dstride + w - x - 1] = qt_colorConvert<DST,SRC>(src[x], 0);
        }
        src -= sstride;
    }
}

template <class DST, class SRC>
static inline void qt_memrotate270_template(const SRC *src,
                                            int srcWidth, int srcHeight, int srcStride,
                                            DST *dest, int dstStride)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    qt_memrotate270_cachedRead<DST,SRC>(src, srcWidth, srcHeight, srcStride,
                                        dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    qt_memrotate270_cachedWrite<DST,SRC>(src, srcWidth, srcHeight, srcStride,
                                         dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
    qt_memrotate270_packing<DST,SRC>(src, srcWidth, srcHeight, srcStride,
                                     dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    qt_memrotate270_tiled_unpacked<DST,SRC>(src, srcWidth, srcHeight,
                                            srcStride,
                                            dest, dstStride);
#endif
}

#ifdef QT_QWS_DEPTH_24
template <>
static inline void qt_memrotate90_template<quint24, quint32>(const quint32 *src,
                                                             int srcWidth, int srcHeight, int srcStride,
                                                             quint24 *dest, int dstStride)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    qt_memrotate90_cachedRead<quint24,quint32>(src, srcWidth, srcHeight,
                                               srcStride, dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    qt_memrotate90_cachedWrite<quint24,quint32>(src, srcWidth, srcHeight,
                                                srcStride, dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
    // packed algorithm not implemented
    qt_memrotate90_cachedRead<quint24,quint32>(src, srcWidth, srcHeight,
                                               srcStride, dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    // packed algorithm not implemented
    qt_memrotate90_tiled_unpacked<quint24,quint32>(src, srcWidth, srcHeight,
                                                   srcStride, dest, dstStride);
#endif
}
#endif // QT_QWS_DEPTH_24

#ifdef QT_QWS_DEPTH_18
template <>
static inline void qt_memrotate90_template<quint18, quint32>(const quint32 *src,
                                                             int srcWidth, int srcHeight, int srcStride,
                                                             quint18 *dest, int dstStride)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    qt_memrotate90_cachedRead<quint18,quint32>(src, srcWidth, srcHeight,
                                               srcStride, dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    qt_memrotate90_cachedWrite<quint18,quint32>(src, srcWidth, srcHeight,
                                                srcStride, dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
    // packed algorithm not implemented
    qt_memrotate90_cachedRead<quint18,quint32>(src, srcWidth, srcHeight,
                                               srcStride, dest, dstStride);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    // packed algorithm not implemented
    qt_memrotate90_tiled_unpacked<quint18,quint32>(src, srcWidth, srcHeight,
                                                   srcStride, dest, dstStride);
#endif
}
#endif // QT_QWS_DEPTH_24

#define QT_IMPL_MEMROTATE(srctype, desttype)                        \
void qt_memrotate90(const srctype *src, int w, int h, int sstride,  \
                    desttype *dest, int dstride)                    \
{                                                                   \
    qt_memrotate90_template(src, w, h, sstride, dest, dstride);     \
}                                                                   \
void qt_memrotate180(const srctype *src, int w, int h, int sstride, \
                     desttype *dest, int dstride)                   \
{                                                                   \
    qt_memrotate180_template(src, w, h, sstride, dest, dstride);    \
}                                                                   \
void qt_memrotate270(const srctype *src, int w, int h, int sstride, \
                     desttype *dest, int dstride)                   \
{                                                                   \
    qt_memrotate270_template(src, w, h, sstride, dest, dstride);    \
}

QT_IMPL_MEMROTATE(quint32, quint32)
QT_IMPL_MEMROTATE(quint32, quint16)
QT_IMPL_MEMROTATE(quint16, quint32)
QT_IMPL_MEMROTATE(quint16, quint16)
#ifdef QT_QWS_DEPTH_24
QT_IMPL_MEMROTATE(quint32, quint24)
#endif
#ifdef QT_QWS_DEPTH_18
QT_IMPL_MEMROTATE(quint32, quint18)
#endif
QT_IMPL_MEMROTATE(quint32, quint8)
QT_IMPL_MEMROTATE(quint16, quint8)
QT_IMPL_MEMROTATE(quint8, quint8)


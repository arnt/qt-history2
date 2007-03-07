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

#ifndef QIMAGESCALE_P_H
#define QIMAGESCALE_P_H

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

#include <qimage.h>

namespace QImageScale {
    struct QImageScaleInfo;
}

typedef void (*qt_qimageScaleFunc)(QImageScale::QImageScaleInfo *isi, unsigned int *dest,
                                   int dxx, int dyy, int dx, int dy, int dw,
                                   int dh, int dow, int sow);


#ifdef QT_HAVE_MMX
extern "C" {
    void __qt_qimageScaleMmxArgb(QImageScale::QImageScaleInfo *isi,
                                 unsigned int *dest, int dxx, int dyy,
                                 int dx, int dy, int dw, int dh,
                                 int dow, int sow);
};
#endif

Q_GUI_EXPORT void qt_qimageScaleAARGB(QImageScale::QImageScaleInfo *isi, unsigned int *dest,
                                      int dxx, int dyy, int dx, int dy, int dw,
                                      int dh, int dow, int sow);

Q_GUI_EXPORT void qt_qimageScaleAARGBA(QImageScale::QImageScaleInfo *isi, unsigned int *dest,
                                       int dxx, int dyy, int dx, int dy, int dw,
                                       int dh, int dow, int sow);

/*
  This version will convert the img to a supported format
*/
QImage qSmoothScaleImageAutoConvert(QImage &img, int w, int h);

/*
  This version accepts only supported formats.
*/
QImage qSmoothScaleImage(const QImage &img, int w, int h);

#endif

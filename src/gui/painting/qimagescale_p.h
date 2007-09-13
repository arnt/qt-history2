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

QT_BEGIN_NAMESPACE

/*
  This version will convert the img to a supported format
*/
QImage qSmoothScaleImageAutoConvert(QImage &img, int w, int h);

/*
  This version accepts only supported formats.
*/
QImage qSmoothScaleImage(const QImage &img, int w, int h);

QT_END_NAMESPACE

#endif

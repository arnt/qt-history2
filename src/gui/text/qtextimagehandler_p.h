/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTIMAGEHANDLER_P_H
#define QTEXTIMAGEHANDLER_P_H

#ifndef QT_H
#include <qobject.h>
#include <qabstracttextdocumentlayout.h>

#include "qtextdocument_p.h"
#endif // QT_H

class QTextImageFormat;

class Q_GUI_EXPORT QTextImageHandler : public QObject,
                          public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    QTextImageHandler(QObject *parent = 0);

    virtual QSize intrinsicSize(const QTextFormat &format);
    virtual void drawObject(QPainter *p, const QRect &rect, const QTextFormat &format);

    typedef QImage (*ExternalImageLoaderFunction)(const QString &name, const QString &context);
    static ExternalImageLoaderFunction externalLoader;
};

#endif // QTEXTIMAGEHANDLER_P_H

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

#ifndef DOMIMAGE_H
#define DOMIMAGE_H

#include <QPixmap>
#include <QImage>
#include <QMetaType>

#include <qscriptengine.h>

class DomImage
{
public:
    DomImage();
    static void setup(QScriptEngine *e);
    
    int width() const;
    int height() const;

    QString src() const;
    void setSrc(const QString &src);

    QString name() const;

    static QScriptValue s_self;

    const QPixmap &image() const
    {
        return m_image;
    }
private:
    QPixmap  m_image;
    QString m_src;
    //attribute boolean         isMap;
    //attribute DOMString       longDesc;
    //attribute DOMString       useMap;
    //attribute DOMString       align;
    //attribute DOMString       alt;
    //attribute DOMString       border;
    //attribute long            vspace;
    //attribute long            hspace;
};

Q_DECLARE_METATYPE(DomImage)
Q_DECLARE_METATYPE(DomImage*)

#endif

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qml.h#16 $
**
** Definition of QML classes
**
** Created : 931107
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QML_H
#define QML_H

#include "qlist.h"
#include "qdict.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"


#if defined(Q_TEMPLATEDLL)
/*
  Gives moc syntac error
template class Q_EXPORT QDict<QPixmap>;
template class Q_EXPORT QDict<QString>;
*/
#endif

class Q_EXPORT QMLProvider : public QObject
{
    Q_OBJECT
public:
    QMLProvider( QObject *parent=0, const char *name=0 );
    virtual ~QMLProvider();

    static QMLProvider* defaultProvider();
    static void setDefaultProvider( QMLProvider* );

    virtual QPixmap image(const QString &name);
    virtual QString document(const QString &name);

    virtual void setImage(const QString& name, const QPixmap& pm);
    virtual void setDocument(const QString& name, const QString& contents);

    virtual void setPath( const QString &path );
    QString path() const;
    virtual void setReferenceDocument( const QString &doc );

    // TODO add nifty pixmap cache stuff

private:
    QDict<QPixmap> images;
    QDict<QString> documents;
    QString searchPath;
    QString absoluteFilename( const QString&) const;
};




#endif // QML_H

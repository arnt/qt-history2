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

#include <qpixmap.h>
#include <qmap.h>
#include "dndbase.h"

#ifndef DNDDEMO_H
#define DNDDEMO_H

class IconItem
{
public:
    IconItem( const QString& name = QString::null, const QString& icon = QString::null );

    QString name() { return _name; }
    QPixmap *pixmap() { return &_pixmap; }

    Q_DUMMY_COMPARISON_OPERATOR( IconItem )

protected:
    QPixmap loadPixmap( const QString& name );

private:
    QString _name;
    QPixmap _pixmap;
};

class DnDDemo : public DnDDemoBase
{
    Q_OBJECT

public:
    DnDDemo( QWidget* parent = 0, const char* name = 0 );
    ~DnDDemo();

    IconItem findItem( const QString& tag );

private:
    QMap<QString,IconItem> items;
};

#endif

/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PIXMAPCOLLECTION_H
#define PIXMAPCOLLECTION_H

#include <qstring.h>
#include <qpixmap.h>
#include <qvaluelist.h>
#include "designerappiface.h"

class QMimeSourceFactory;
class Project;

class PixmapCollection
{
public:
    struct Pixmap
    {
	QPixmap pix;
	QString name;
	QString absname;
	Q_DUMMY_COMPARISON_OPERATOR( Pixmap )
    };

    PixmapCollection( Project *pro );
    ~PixmapCollection();

    bool addPixmap( const Pixmap &pix, bool force = TRUE );
    void removePixmap( const QString &name );
    QPixmap pixmap( const QString &name );

    QValueList<Pixmap> pixmaps() const;
    bool isEmpty() const;

    void setActive( bool b );

    void load( const QString& filename );

    DesignerPixmapCollection *iFace();

private:
    QString unifyName( const QString &n );
    void savePixmap( Pixmap &pix );

    QString imageDir() const;
    void mkdir();

private:
    QValueList<Pixmap> pixList;
    QMimeSourceFactory *mimeSourceFactory;
    Project *project;
    DesignerPixmapCollectionImpl *iface;

};

#endif

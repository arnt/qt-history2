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

#ifndef DROPSITE_H
#define DROPSITE_H

#include <qlabel.h>
#include <qmovie.h>
#include "qdropsite.h"

class QDragObject;

class DropSite: public QLabel
{
    Q_OBJECT
public:
    DropSite( QWidget * parent = 0, const char * name = 0 );
    ~DropSite();

signals:
    void message( const QString& );

protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
    void backgroundColorChange( const QColor& );

    // this is a normal even
    void mousePressEvent( QMouseEvent * );
};

class DragMoviePlayer : public QObject {
    Q_OBJECT
    QDragObject* dobj;
    QMovie movie;
public:
    DragMoviePlayer(QDragObject*);
private slots:
    void updatePixmap( const QRect& );
};


#endif

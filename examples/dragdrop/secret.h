/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SECRETDRAG_H
#define SECRETDRAG_H

#include <qdragobject.h>
#include <qlabel.h>

class SecretDrag: public QStoredDrag {
public:
    SecretDrag( uchar, QWidget * parent = 0, const char * name = 0 );
    ~SecretDrag() {};

    static bool canDecode( QDragMoveEvent* e );
    static bool decode( QDropEvent* e, QString& s );
};


class SecretSource: public QLabel
{
public:
    SecretSource( int secret, QWidget *parent = 0, const char * name = 0 );
    ~SecretSource();

protected:
    void mousePressEvent( QMouseEvent * );
private:
    int mySecret;
};

#endif

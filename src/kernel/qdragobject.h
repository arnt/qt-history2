/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.h#2 $
**
** Definition of
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QDRAGOBJECT_H
#define QDRAGOBJECT_H

struct QDragData;
class QWidget;

#include "qobject.h"

class QDragObject: public QObject {
    Q_OBJECT
public:
    QDragObject( QWidget * dragSource = 0, const char * name = 0 );
    ~QDragObject();

    virtual void setAutoDelete( bool );
    bool autoDelete() const;
    
    virtual void startDrag();
    
    virtual void setFormat( const char * mimeType );
    const char * format() const;

    void setEncodedData( QByteArray & );
    const QByteArray encodedData() const;
    
    virtual void encode();

private:
    QDragData * d;
};


class QTextDragObject: public QDragObject {
    Q_OBJECT
public:
    QTextDragObject( const char *,
		     QWidget * parent = 0, const char * name = 0 );
    QTextDragObject( QWidget * parent = 0, const char * name = 0 );
    ~QTextDragObject();

    void setText( const char * );
};


#endif

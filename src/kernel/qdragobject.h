/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.h#8 $
**
** Definition of QDragObject
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QDRAGOBJECT_H
#define QDRAGOBJECT_H

struct QDragData;
class QWidget;

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


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

    QWidget * source();

    virtual void setAlternative( QDragObject * );
    QDragObject * alternative() const;

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


// QDragManager is not part of the public API.  It is defined in a
// header file simply so different .cpp files can implement different
// member functions.
//

class QDragManager: public QObject {
    Q_OBJECT

public:
    static void registerDropType( QWidget *, const char * );

private:
    QDragManager();
    ~QDragManager();
    // only friend classes can use QDragManager.
    friend class QDragObject;

    bool eventFilter( QObject *, QEvent * );

    void startDrag( QDragObject * );

    void cancel();
    void move( const QPoint & );
    void drop();

private:
    QDragObject * object;

    QWidget * dragSource;
    QWidget * dropWidget;
    bool beingCancelled;
    bool restoreCursor;
};


#endif

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.h#34 $
**
** Definition of QDragObject
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QDRAGOBJECT_H
#define QDRAGOBJECT_H

struct QDragData;
struct QStoredDragData;
class QWidget;

#ifndef QT_H
#include "qobject.h"
#include "qimage.h"
#include "qstrlist.h"
#endif // QT_H


class Q_EXPORT QDragObject: public QObject, public QMimeSource {
    Q_OBJECT
public:
    QDragObject( QWidget * dragSource = 0, const char * name = 0 );
    ~QDragObject();

    bool drag();
    bool dragMove();
    void dragCopy();

    virtual void setPixmap(QPixmap);
    virtual void setPixmap(QPixmap, QPoint hotspot);
    QPixmap pixmap() const;
    QPoint pixmapHotSpot() const;

    QWidget * source();
    QWidget * target();

    void setTarget(QWidget*);

    enum DragMode { DragDefault, DragCopy, DragMove, DragCopyOrMove };

protected:
    virtual bool drag(DragMode);

private:
    QDragData * d;
};

class Q_EXPORT QStoredDrag: public QDragObject {
    Q_OBJECT
    QStoredDragData * d;

public:
    QStoredDrag( const char * mimeType,
		 QWidget * dragSource = 0, const char * name = 0 );
    ~QStoredDrag();

    virtual void setEncodedData( const QByteArray & );

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;
};

class Q_EXPORT QTextDrag: public QDragObject {
    Q_OBJECT
    QString txt;
public:
    QTextDrag( const QString &,
	       QWidget * dragSource = 0, const char * name = 0 );
    QTextDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QTextDrag();

    virtual void setText( const QString &);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode( QMimeSource* e );
    static bool decode( QMimeSource* e, QString& s );
};


class Q_EXPORT QImageDrag: public QDragObject {
    Q_OBJECT
    QImage img;
    QStrList ofmts;

public:
    QImageDrag( QImage image,
		QWidget * dragSource = 0, const char * name = 0 );
    QImageDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QImageDrag();

    virtual void setImage( QImage image );

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode( QMimeSource* e );
    static bool decode( QMimeSource* e, QImage& i );
    static bool decode( QMimeSource* e, QPixmap& i );
};


class Q_EXPORT QUrlDrag: public QStoredDrag {
    Q_OBJECT

public:
    QUrlDrag( QStrList urls,
		QWidget * dragSource = 0, const char * name = 0 );
    QUrlDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QUrlDrag();

    virtual void setUrls( QStrList urls );

    static QString urlToLocalFile(const char*);
    static bool canDecode( QMimeSource* e );
    static bool decode( QMimeSource* e, QStrList& i );
    static bool decodeLocalFiles( QMimeSource* e, QStrList& i );
};


// QDragManager is not part of the public API.  It is defined in a
// header file simply so different .cpp files can implement different
// member functions.
//

class Q_EXPORT QDragManager: public QObject {
    Q_OBJECT

private:
    QDragManager();
    ~QDragManager();
    // only friend classes can use QDragManager.
    friend class QDragObject;

    bool eventFilter( QObject *, QEvent * );

    bool drag( QDragObject *, QDragObject::DragMode );

    void cancel();
    void move( const QPoint & );
    void drop();
    void updatePixmap();

private:
    QDragObject * object;
    void updateMode( ButtonState newstate );
    void updateCursor();

    QWidget * dragSource;
    QWidget * dropWidget;
    bool beingCancelled;
    bool restoreCursor;
    bool willDrop;

    QPixmap *pm_cursor;
    int n_cursor;
};


#endif

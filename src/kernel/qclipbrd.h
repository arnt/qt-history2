/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipbrd.h#5 $
**
** Definition of QClipboard class
**
** Created : 960430
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QCLIPBRD_H
#define QCLIPBRD_H

#include "qobject.h"


class QClipboard : public QObject
{
    Q_OBJECT
private:
    QClipboard( QObject *parent=0, const char *name=0 );
   ~QClipboard();

public:
    void	clear();

    void       *data( const char *format ) const;
    void	setData( const char *format, void * );

    const char *text()	 const;
    void	setText( const char * );
    QPixmap    *pixmap() const;
    void	setPixmap( const QPixmap & );

signals:
    void	dataChanged();

private slots:
    void	ownerDestroyed();

protected:
    void	connectNotify( const char * );
    bool	event( QEvent * );

    friend class QApplication;

private:	// Disabled copy constructor and operator=
    QClipboard( const QClipboard & ) {}
    QClipboard &operator=( const QClipboard & ) { return *this; }
};


#endif // QCLIPBRD_H

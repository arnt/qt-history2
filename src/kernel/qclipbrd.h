/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipbrd.h#1 $
**
** Definition of QClipboard class
**
** Author  : Haavard Nord
** Created : 960430
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
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
    bool	isEmpty() const;
    void	clear();

    bool	available( const char *format ) const;

    void       *data( const char *format ) const;
    void	setData( const char *format, void * );

    const char *text()	 const;
    void	setText( const char * );
    QPixmap    *pixmap() const;
    void	setPixmap( const QPixmap & );

signals:
    void	dataChanged();

protected:
    bool	event( QEvent * );

    friend class QApplication;
};


#endif // QCLIPBRD_H

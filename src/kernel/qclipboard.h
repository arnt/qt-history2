/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard.h#24 $
**
** Definition of QClipboard class
**
** Created : 960430
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

#ifndef QCLIPBOARD_H
#define QCLIPBOARD_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QMimeSource;

class Q_EXPORT QClipboard : public QObject
{
    Q_OBJECT
private:
    QClipboard( QObject *parent=0, const char *name=0 );
   ~QClipboard();

public:
    void	clear();

    QMimeSource* data() const;
    void  setData( QMimeSource* );

    QString     text()	 const;
    void	setText( const QString &);
    QImage	image() const;
    QPixmap	pixmap() const;
    void	setImage( const QImage & );
    void	setPixmap( const QPixmap & );

signals:
    void	dataChanged();

private slots:
    void	ownerDestroyed();

protected:
    void	connectNotify( const char * );
    bool	event( QEvent * );

    friend class QApplication;
    friend class QDragManager;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QClipboard( const QClipboard & );
    QClipboard &operator=( const QClipboard & );
#endif
};


#endif // QCLIPBOARD_H

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.h#34 $
**
** Definition of QDialog class
**
** Created : 950502
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

#ifndef QDIALOG_H
#define QDIALOG_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class QPushButton;


class Q_EXPORT QDialog : public QWidget			// dialog widget
{
friend class QPushButton;
    Q_OBJECT
public:
    QDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
	     WFlags f=0 );
   ~QDialog();

    enum DialogCode { Rejected, Accepted };

    int		exec();
    int		result()  const { return rescode; }

    void	show();
    void	hide();
    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );
    void	setGeometry( const QRect & );

protected slots:
    virtual void done( int );
    void	accept();
    void	reject();

protected:
    void	setResult( int r )	{ rescode = r; }
    void	keyPressEvent( QKeyEvent * );
    void	closeEvent( QCloseEvent * );

private:
    virtual void	setDefault( QPushButton * );
    int		rescode;
    uint	did_move   : 1;
    uint	did_resize : 1;
    QPushButton* mainDef;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDialog( const QDialog & );
    QDialog &operator=( const QDialog & );
#endif
};


#endif // QDIALOG_H

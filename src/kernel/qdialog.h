/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.h#22 $
**
** Definition of QDialog class
**
** Created : 950502
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDIALOG_H
#define QDIALOG_H

#include "qwidget.h"


class QPushButton;


class QDialog : public QWidget			// dialog widget
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
    void	setDefault( QPushButton * );
    int		rescode;
    uint	did_move   : 1;
    uint	did_resize : 1;

private:	// Disabled copy constructor and operator=
    QDialog( const QDialog & ) {}
    QDialog &operator=( const QDialog & ) { return *this; }
};


#endif // QDIALOG_H

/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.h#5 $
**
** Definition of QDialog class
**
** Author  : Haavard Nord
** Created : 950502
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDIALOG_H
#define QDIALOG_H

#include "qview.h"


class QPushButton;


class QDialog : public QView			// modal dialog widget
{
friend class QPushButton;
    Q_OBJECT
public:
    QDialog( QWidget *parent=0, const char *name=0, WFlags f=WType_Modal );
   ~QDialog();

    enum DialogCode { Rejected, Accepted };

    int		exec();
    int		result() const		{ return rescode; }

    virtual void adjustSize();

    void	show();
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

private:
    void	setDefault( QPushButton * );
    int		rescode;
    uint	did_move   : 1;
    uint	did_resize : 1;
};


#endif // QDIALOG_H

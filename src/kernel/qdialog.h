/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdialog.h#40 $
**
** Definition of QDialog class
**
** Created : 950502
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QDIALOG_H
#define QDIALOG_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#if 0
Q_OBJECT
#endif

class QDialogPrivate;
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
    	
    void	setOrientation( Orientation orientation );
    Orientation	orientation() const;
    
    void	setExtension( QWidget* extension );
    QWidget* extension() const;
    
    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;

protected slots:
    virtual void done( int );
    virtual void accept();
    virtual void reject();
    
    void	showExtension( bool );

protected:
    void	setResult( int r )	{ rescode = r; }
    void	keyPressEvent( QKeyEvent * );
    void	closeEvent( QCloseEvent * );
private:
    virtual void	setDefault( QPushButton * ); // ## remove virtual 3.0
    void		hideDefault();
    int		rescode;
    uint	did_move   : 1;
    uint	did_resize : 1;
    uint 	in_loop: 1;
    QDialogPrivate* d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDialog( const QDialog & );
    QDialog &operator=( const QDialog & );
#endif
};


#endif // QDIALOG_H

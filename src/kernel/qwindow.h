/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindow.h#1 $
**
** Definition of QWindow class
**
** Author  : Haavard Nord
** Created : 931112
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWINDOW_H
#define QWINDOW_H

#include "qwidget.h"


class QWindow : public QWidget			// window widget class
{
friend class QObject;
    Q_OBJECT
public:
    QWindow( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QWindow();

    char   *caption() const;			// get caption text
    void    setCaption( const char * );		// set caption text
    char   *iconText() const;			// get icon text
    void    setIconText( const char * );	// set icon text
    void    setIcon( QPixmap * );		// set icon pixmap

private:
    QString ctext;
    QString itext;
    QPixmap *ipm;
};


#endif // QWINDOW_H

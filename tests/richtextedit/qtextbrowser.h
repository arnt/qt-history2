/****************************************************************************
** $Id$
**
** Definition of the QTextBrowser class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#include "qptrlist.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"

#include "qtextview.h"

class QtTextBrowserData;
class QtTextCharFormat;

class  Q_EXPORT QtTextBrowser : public QtTextView
{
    Q_OBJECT
public:
    QtTextBrowser( QWidget *parent=0, const char *name=0 );
    ~QtTextBrowser();

    virtual void setSource(const QString& name);
    QString source() const;

    void setText( const QString& contents, const QString& context=QString::null );

    void scrollToAnchor(const QString& name);


public slots:
    virtual void backward();
    virtual void forward();
    virtual void home();

signals:
    void backwardAvailable( bool );
    void forwardAvailable( bool );
    void highlighted( const QString& );
    void textChanged();

protected:
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * e);

private:
    void popupDetail( const QString& contents, const QPoint& pos );
    QString anchorAt(const QPoint& pos);
    QtTextBrowserData *d;

};


#endif

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextbrowser.h#6 $
**
** Definition of the QTextBrowser class
**
** Created : 990101
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

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#ifndef QT_H
#include "qlist.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"
#include "qtextview.h"
#endif // QT_H

class QTextBrowserData;

class Q_EXPORT QTextBrowser : public QTextView
{
    Q_OBJECT
public:
    QTextBrowser( QWidget *parent=0, const char *name=0 );
    ~QTextBrowser();
qproperties:
    virtual void setSource(const QString& name);
    QString source() const;
public:
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
    const QTextContainer* anchor(const QPoint& pos);
    QTextBrowserData *d;

};


#endif

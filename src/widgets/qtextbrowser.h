/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextbrowser.h#6 $
**
** Definition of the QTextBrowser class
**
** Created : 990101
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

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#ifndef QT_H
#include "qlist.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"
#include "qtextview.h"
#endif // QT_H

#if QT_FEATURE_TEXTBROWSER

class QTextBrowserData;

class Q_EXPORT QTextBrowser : public QTextView
{
    Q_OBJECT
    Q_PROPERTY( QString source READ source WRITE setSource )

public:
    QTextBrowser( QWidget *parent=0, const char *name=0 );
    ~QTextBrowser();

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
    void showEvent( QShowEvent* );

private:
    void popupDetail( const QString& contents, const QPoint& pos );
    QString anchorAt(const QPoint& pos); // public in 3.0
    QTextBrowserData *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextBrowser( const QTextBrowser & );
    QTextBrowser& operator=( const QTextBrowser & );
#endif
};

#endif // QT_FEATURE_TEXTBROWSER

#endif // QTEXTBROWSER_H

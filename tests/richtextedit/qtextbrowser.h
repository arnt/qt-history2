/****************************************************************************
**
** Definition of the QTextBrowser class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

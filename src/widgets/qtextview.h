/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.h#4 $
**
** Definition of the QTextView class
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

#ifndef QTEXTVIEW_H
#define QTEXTVIEW_H

#include "qlist.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"

class QRichText;
class QTextViewData;
class QTextContainer;
class QStyleSheet;
class QMimeSourceFactory;

class Q_EXPORT QTextView : public QScrollView
{
    Q_OBJECT
public:
    QTextView(QWidget *parent=0, const char *name=0);
    QTextView( const QString& text, const QString& context = QString::null,
	       QWidget *parent=0, const char *name=0);
    ~QTextView();

    virtual void setText( const QString& text, const QString& context = QString::null );
    virtual QString text() const;
    virtual QString context() const;
    
    Qt::TextFormat textFormat() const;
    void setTextFormat( Qt::TextFormat );

    
    QStyleSheet* styleSheet() const;
    void setStyleSheet( QStyleSheet* styleSheet );


    // convenience functions
    void setPaper( const QBrush& pap);
    const QBrush& paper();

    void setPaperColorGroup( const QColorGroup& colgrp);
    const QColorGroup &paperColorGroup() const;

    void setMimeSourceFactory( QMimeSourceFactory* factory );
    QMimeSourceFactory* mimeSourceFactory() const;

    QString documentTitle() const;

    int heightForWidth( int w ) const;

protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void resizeEvent(QResizeEvent*);
    void viewportResizeEvent(QResizeEvent*);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );

protected:

    QRichText& richText() const;
    void paletteChange( const QPalette & );

private slots:
    void doResize();

private:
    void init();
    void createRichText();
    QTextViewData* d;
};



#if 0
class QTextCursor;

class Q_EXPORT QTextEdit : public QTextView
{
    //    Q_OBJECT
public:
    QTextEdit(QWidget *parent=0, const char *name=0);
    ~QTextEdit();

    void setText( const QString& text, const QString& context = QString::null );
    QString text();

protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );
    void viewportResizeEvent(QResizeEvent*);

    void showCursor();
    void hideCursor();

    //private slots:
void cursorTimerDone();

private:
    bool cursor_hidden;
    QTimer* cursorTimer;
    QTextCursor* cursor;

    void updateSelection(int oldY=-1, int newY=-1);

    void updateScreen();
    void* d;
};
#endif



#endif

/****************************************************************************
**
** Definition of the QtTextView class.
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

#ifndef QTEXTVIEW_H
#define QTEXTVIEW_H

#include "qptrlist.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"

class QtRichText;
class QtTextParagraph;
class QtTextViewData;
class QtTextEditData;
class QStyleSheet;
class QMimeSourceFactory;

class Q_EXPORT QtTextView : public QScrollView
{
    Q_OBJECT
public:
    QtTextView(QWidget *parent=0, const char *name=0);
    QtTextView( const QString& text, const QString& context = QString::null,
	       QWidget *parent=0, const char *name=0);
    ~QtTextView();

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

    void setLinkColor( const QColor& );
    const QColor& linkColor() const;

    void setLinkUnderline( bool );
    bool linkUnderline() const;

    void setMimeSourceFactory( QMimeSourceFactory* factory );
    QMimeSourceFactory* mimeSourceFactory() const;

    QString documentTitle() const;

    int heightForWidth( int w ) const;

    void append( const QString& text );

public slots:
   void copy();
   void selectAll();


protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void resizeEvent(QResizeEvent*);
    void viewportResizeEvent(QResizeEvent*);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );
    void showEvent( QShowEvent* );

protected:

    QtRichText& richText() const;
    void paletteChange( const QPalette & );

private slots:
    void doResize();
    void clipboardChanged(); 

private:
    void init();
    void createRichText();
    friend class QtRichText;
    QtTextViewData* d;
//     void paragraphChanged( QtTextParagraph* );
    void updateLayout( int ymax = -1 );
    void clearSelection();
};




class Q_EXPORT QtTextEdit : public QtTextView
{
    Q_OBJECT
public:
    QtTextEdit(QWidget *parent=0, const char *name=0);
    ~QtTextEdit();

    void setText( const QString& text, const QString& context = QString::null );
    QString text();

protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );
    void resizeEvent(QResizeEvent*);
    void viewportResizeEvent(QResizeEvent*);

    void showCursor();
    void hideCursor();

public slots:
    void temporary();


private slots:
    void cursorTimerDone();

private:
    QtTextEditData* d;
};



#endif

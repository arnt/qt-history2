/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.h#6 $
**
** Definition of the QTextView class
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTEXTVIEW_H
#define QTEXTVIEW_H

#ifndef QT_H
#include "qlist.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"
#endif // QT_H

class QRichText;
class QTextViewData;
class QTextContainer;
class QStyleSheet;
class QMimeSourceFactory;

class Q_EXPORT QTextView : public QScrollView
{
    Q_OBJECT
    // ######### setText should not have the default parameter
    Q_PROPERTY( QString text READ text )
    Q_PROPERTY( TextFormat textFormat READ textFormat WRITE setTextFormat )
    Q_PROPERTY( QBrush paper READ paper WRITE setPaper )
    Q_PROPERTY( QColorGroup paperColorGroup READ paperColorGroup WRITE setPaperColorGroup )
    Q_PROPERTY( QColor linkColor READ linkColor WRITE setLinkColor )
    Q_PROPERTY( bool linkUnderline READ linkUnderline WRITE setLinkUnderline )
    Q_PROPERTY( QString documentTitle READ documentTitle )
	
public:
    QTextView(QWidget *parent=0, const char *name=0);
    QTextView( const QString& text, const QString& context = QString::null,
	       QWidget *parent=0, const char *name=0);
    ~QTextView();

    virtual void setText( const QString& text, const QString& context = QString::null );
    virtual QString text() const;
    virtual QString context() const;

    TextFormat textFormat() const;
    void setTextFormat( TextFormat );


    QStyleSheet* styleSheet() const;
    void setStyleSheet( QStyleSheet* styleSheet );


    // convenience functions
    void setPaper( const QBrush& pap);
    // ##### This non const thing is obsolete
    const QBrush& paper();
    const QBrush& paper() const;

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

    bool hasSelectedText() const;
    QString selectedText() const;

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

    QRichText& richText() const;
    void paletteChange( const QPalette & );

private slots:
    void doResize();
    void clipboardChanged();
    void doStartDrag();
    void doAutoScroll();

private:
    void init();
    void createRichText();
    friend class QRichText;
    QTextViewData* d;
    void updateLayout();
    void clearSelection();
    void doSelection( const QPoint& );
};



#endif

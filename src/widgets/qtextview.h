/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.h#6 $
**
** Definition of the QTextView class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTEXTVIEW_H
#define QTEXTVIEW_H

#ifndef QT_H
#include "qscrollview.h"
#include "qstylesheet.h"
#include "qpainter.h"
#endif // QT_H

class QPainter;
class QTextDocument;
class QTextCursor;
class QKeyEvent;
class QResizeEvent;
class QMouseEvent;
class QTimer;
class QTextString;
class QTextCommand;
class QTextParag;
class QTextFormat;
class QFont;
class QColor;

class Q_EXPORT QTextView : public QScrollView
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( TextFormat textFormat READ textFormat WRITE setTextFormat )
    Q_PROPERTY( QBrush paper READ paper WRITE setPaper )
    Q_PROPERTY( QColor linkColor READ linkColor WRITE setLinkColor )
    Q_PROPERTY( bool linkUnderline READ linkUnderline WRITE setLinkUnderline )
    Q_PROPERTY( QString documentTitle READ documentTitle )

public:
    QTextView( const QString& text, const QString& context = QString::null,
	       QWidget *parent=0, const char *name=0);
    QTextView( QWidget *parent = 0, const char *name = 0 );
    virtual ~QTextView();

#if defined(QRICHTEXT_OPEN_API)
    QTextDocument *document() const;
    void setDocument( QTextDocument *doc );
    QTextCursor *textCursor() const;
#endif

    QString text() const;
    QString text( int parag, bool formatted = FALSE ) const;
    TextFormat textFormat() const;
    QString fileName() const;

    void cursorPosition( int &parag, int &index );
    void selection( int &parag_from, int &index_from,
		    int &parag_to, int &index_to );
    virtual bool find( const QString &expr, bool cs, bool wo, bool forward = TRUE,
		       int *parag = 0, int *index = 0 );
    void insert( const QString &text, bool indent = FALSE, bool checkNewLine = FALSE );

    int paragraphs() const;
    int lines() const;
    int linesOfParagraph( int parag ) const;
    int lineOfChar( int parag, int chr );

    bool isModified() const;

    bool italic() const;
    bool bold() const;
    bool underline() const;
    QString family() const;
    int pointSize() const;
    QColor color() const;
    QFont font() const;
    int alignment() const;
    int maxLines() const;

    QStyleSheet* styleSheet() const;
    void setStyleSheet( QStyleSheet* styleSheet );

    void setPaper( const QBrush& pap);
    QBrush paper() const;

    void setLinkColor( const QColor& );
    QColor linkColor() const;

    void setLinkUnderline( bool );
    bool linkUnderline() const;

    void setMimeSourceFactory( QMimeSourceFactory* factory );
    QMimeSourceFactory* mimeSourceFactory() const;

    int heightForWidth( int w ) const;

    void append( const QString& text );

    bool hasSelectedText() const;
    QString selectedText() const;

    QString context() const;

    QString documentTitle() const;

    void scrollToAnchor( const QString& name );
    QString anchorAt(const QPoint& pos);

    void repaintChanged();
    void updateStyles();

public slots:
    virtual void undo();
    virtual void redo();

    virtual void cut();
    virtual void copy();
    virtual void paste();

    virtual void indent();

    virtual void setItalic( bool b );
    virtual void setBold( bool b );
    virtual void setUnderline( bool b );
    virtual void setFamily( const QString &f );
    virtual void setPointSize( int s );
    virtual void setColor( const QColor &c );
    virtual void setFont( const QFont &f );

    virtual void setAlignment( int );

    virtual void setParagType( QStyleSheetItem::DisplayMode, int listStyle );

    virtual void setTextFormat( TextFormat f );
    virtual void setText( const QString &txt ) { setText( txt, QString::null ); }
    virtual void setText( const QString &txt, const QString &context );

    virtual void load( const QString &fn );
    virtual void save( const QString &fn = QString::null );

    virtual void setCursorPosition( int parag, int index );
    virtual void setSelection( int parag_from, int index_from,
			       int parag_to, int index_to );

    virtual void setModified( bool m );
    virtual void selectAll( bool select );

    virtual void setMaxLines( int l );
    virtual void resetFormat();

signals:
    void currentFontChanged( const QFont &f );
    void currentColorChanged( const QColor &c );
    void currentAlignmentChanged( int );
    void textChanged();
    void highlighted( const QString& );
    void linkClicked( const QString& );
    void cursorPositionChanged( QTextCursor *c );
    void selectionChanged();
    
protected:
    void setFormat( QTextFormat *f, int flags );
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void keyPressEvent( QKeyEvent *e );
    void resizeEvent( QResizeEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
#ifndef QT_NO_DRAGANDDROP
    void contentsDragEnterEvent( QDragEnterEvent *e );
    void contentsDragMoveEvent( QDragMoveEvent *e );
    void contentsDragLeaveEvent( QDragLeaveEvent *e );
    void contentsDropEvent( QDropEvent *e );
#endif
    bool eventFilter( QObject *o, QEvent *e );
    bool focusNextPrevChild( bool next );
#if !defined(QRICHTEXT_OPEN_API)
    QTextDocument *document() const;
    QTextCursor *textCursor() const;
    void setDocument( QTextDocument *doc );
#endif

private slots:
    void formatMore();
    void doResize();
    void doAutoScroll();
    void doChangeInterval();
    void blinkCursor();
    void setModified();
    void startDrag();
    void setRealWidth( int w );

private:
    enum MoveDirection {
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	MoveHome,
	MoveEnd,
	MovePgUp,
	MovePgDown
    };
    enum KeyboardAction {
	ActionBackspace,
	ActionDelete,
	ActionReturn
    };

    struct Q_EXPORT UndoRedoInfo {
	enum Type { Invalid, Insert, Delete, Backspace, Return, RemoveSelected };
	UndoRedoInfo( QTextDocument *d ) : type( Invalid ), doc( d )
	{ text = QString::null; id = -1; index = -1; }
	void clear();
	inline bool valid() const { return !text.isEmpty() && id >= 0&& index >= 0; }

    	QString text;
	int id;
	int index;
	Type type;
	QTextDocument *doc;
    };

private:
    virtual bool isReadOnly() const { return TRUE; }
    virtual bool linksEnabled() const { return FALSE; }
    void init();
    void ensureCursorVisible();
    void drawCursor( bool visible );
    void placeCursor( const QPoint &pos, QTextCursor *c = 0 );
    void moveCursor( int direction, bool shift, bool control );
    void moveCursor( int direction, bool control );
    void removeSelectedText();
    void doKeyboardAction( int action );
    void checkUndoRedoInfo( UndoRedoInfo::Type t );
    void updateCurrentFormat();
    void handleReadOnlyKeyEvent( QKeyEvent *e );
    void makeParagVisible( QTextParag *p );

private:
    QTextDocument *doc;
    QTextCursor *cursor;
    bool drawAll;
    bool mousePressed;
    QTimer *formatTimer, *scrollTimer, *changeIntervalTimer, *blinkTimer, *dragStartTimer, *resizeTimer;
    QTextParag *lastFormatted;
    int interval;
    UndoRedoInfo undoRedoInfo;
    QTextFormat *currentFormat;
    int currentAlignment;
    bool inDoubleClick;
    QPoint oldMousePos, mousePos;
    QPixmap *buf_pixmap;
    bool cursorVisible, blinkCursorVisible;
    bool readOnly, modified, mightStartDrag;
    QPoint dragStartPos;
    int mLines;
    bool firstResize;
    QString onLink;

};

inline QTextDocument *QTextView::document() const
{
    return doc;
}

inline QTextCursor *QTextView::textCursor() const
{
    return cursor;
}

#endif

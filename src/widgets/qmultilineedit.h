/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmultilineedit.h#15 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
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

#ifndef QMULTILINEEDIT_H
#define QMULTILINEEDIT_H

#ifndef QT_H
#include "qtableview.h"
#include "qstring.h"
#include "qlist.h"
#endif // QT_H

struct QMultiLineData;
class QValidator;

class Q_EXPORT QMultiLineEdit : public QTableView
{
    Q_OBJECT
    Q_BUILDER( "Texteditor", "qmultilineedit.xpm", DefaultInspector )
public:
    QMultiLineEdit( QWidget *parent=0, const char *name=0 );
   ~QMultiLineEdit();

    QString textLine( int line ) const;
    int numLines() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void	setFont( const QFont &font );
    virtual void insertLine( const QString &s, int line = -1 );
    virtual void insertAt( const QString &s, int line, int col, bool mark = FALSE );
    virtual void removeLine( int line );

    void 	cursorPosition( int *line, int *col ) const;
    virtual void	setCursorPosition( int line, int col, bool mark = FALSE );
    void	getCursorPosition( int *line, int *col ) const;
    bool	atBeginning() const;
    bool	atEnd() const;

    virtual void	setFixedVisibleLines( int lines );

    int 	maxLineWidth() const;

    void setAlignment( int flags );
    int alignment() const;

    virtual void setValidator( const QValidator * );
    const QValidator * validator() const;

    void setEdited( bool );
    bool edited() const;

    void cursorLeft( bool mark, long steps );
    void cursorRight( bool mark, long steps );
    void cursorWordForward( bool mark );
    void cursorWordBackward( bool mark );

    enum EchoMode { Normal, NoEcho, Password };
    virtual void setEchoMode( EchoMode );
    EchoMode echoMode() const;

    virtual void setMaxLineLength(int);
    int maxLineLength() const;
    virtual void setMaxLines(int);
    int maxLines() const;
    virtual void setHMargin(int);
    int hMargin() const;
    virtual void setSelection( int row_from, int col_from, int row_to, int col_t );

q_properties:
    bool	autoUpdate()	const;
    virtual void	setAutoUpdate( bool );

    bool	isReadOnly() const;
    bool	isOverwriteMode() const;
    QString text() const;

public slots:
    virtual void       setText( const QString &);
    virtual void       setReadOnly( bool );
    virtual void       setOverwriteMode( bool );

    void       clear();
    void       append( const QString &);
    void       deselect();
    void       selectAll();
    void       paste();
    void       copyText() const;
    void       copy() const;
    void       cut();
    void       insert( const QString& );

signals:
    void	textChanged();
    void	returnPressed();

protected:
    void	paintCell( QPainter *, int row, int col );

    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void 	wheelEvent( QWheelEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	timerEvent( QTimerEvent * );
    void	leaveEvent( QEvent * );
    void	resizeEvent( QResizeEvent * );

    void	dragMoveEvent( QDragMoveEvent* );
    void	dropEvent( QDropEvent* );
    void	dragLeaveEvent( QDragLeaveEvent* );

    bool	hasMarkedText() const;
    QString	markedText() const;
    int		textWidth( int );
    int		textWidth( const QString &);

    QPoint	cursorPoint() const;

protected:
    virtual void insert( const QString&, bool mark );
    virtual void newLine();
    virtual void killLine();
    virtual void pageUp( bool mark=FALSE );
    virtual void pageDown( bool mark=FALSE );
    virtual void cursorLeft( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorRight( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorUp( bool mark=FALSE );
    virtual void cursorDown( bool mark=FALSE );
    virtual void backspace();
    virtual void del();
    virtual void home( bool mark=FALSE );
    virtual void end( bool mark=FALSE );

    bool	getMarkedRegion( int *line1, int *col1,
				 int *line2, int *col2 ) const;
    int		lineLength( int row ) const;
    QString	*getString( int row ) const;
    QString     stringShown( int row ) const;

protected:
    bool	cursorOn;	
    void	insertChar( QChar );

private slots:
    void	clipboardChanged();

private:
    struct QMultiLineEditRow {
	QMultiLineEditRow( QString string, int width ):s(string), w(width){};
	QString s;
	int w;
    };
    QList<QMultiLineEditRow> *contents;
    QMultiLineData *mlData;

    bool	readOnly;
    bool	dummy;
    bool	markIsOn;
    bool	dragScrolling ;
    bool	dragMarking;
    bool	textDirty;
    bool	wordMark;
    bool	overWrite;

    int		cursorX;
    int		cursorY;
    int		markAnchorX;
    int		markAnchorY;
    int		markDragX;
    int		markDragY;
    int		curXPos;	// cell coord of cursor
    int		blinkTimer;
    int		scrollTimer;

    int		mapFromView( int xPos, int row );
    int		mapToView( int xIndex, int row );

    void	pixelPosToCursorPos(QPoint p, int* x, int* y) const;
    void	setCursorPixelPosition(QPoint p, bool clear_mark=TRUE);

    void	setWidth( int );
    void	updateCellWidth();
    bool 	partiallyInvisible( int row );
    void	makeVisible();
    void	setBottomCell( int row );

    void 	newMark( int posx, int posy, bool copy=TRUE );
    void 	markWord( int posx, int posy );
    int 	charClass( QChar );
    void	turnMarkOff();
    bool	inMark( int posx, int posy ) const;
    bool	beforeMark( int posx, int posy ) const;
    bool	afterMark( int posx, int posy ) const;

    void	repaintDelayed( bool erase = TRUE );

    void	doDrag();
    void	startAutoScroll();
    void	stopAutoScroll();

    void	cursorLeft( bool mark, bool clear_mark, bool wrap );
    void	cursorRight( bool mark, bool clear_mark, bool wrap );
    void	cursorUp( bool mark, bool clear_mark );
    void	cursorDown( bool mark, bool clear_mark );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMultiLineEdit( const QMultiLineEdit & );
    QMultiLineEdit &operator=( const QMultiLineEdit & );
#endif
};

inline bool QMultiLineEdit::isReadOnly() const { return readOnly; }

inline bool QMultiLineEdit::isOverwriteMode() const { return overWrite; }

inline void QMultiLineEdit::setOverwriteMode( bool on )
{
    overWrite = on;
 }

inline int QMultiLineEdit::lineLength( int row ) const
{
    return contents->at( row )->s.length();
}

inline bool QMultiLineEdit::atEnd() const
{
    return cursorY == (int)contents->count() - 1
	&& cursorX == lineLength( cursorY ) ;
}

inline bool QMultiLineEdit::atBeginning() const
{
    return cursorY == 0 && cursorX == 0;
}

inline QString *QMultiLineEdit::getString( int row ) const
{
    return &(contents->at( row )->s);
}

inline int QMultiLineEdit::numLines() const
{
    return contents->count();
}
#endif // QMULTILINED_H

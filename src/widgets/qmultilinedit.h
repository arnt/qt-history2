/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmultilinedit.h#13 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QMLINED_H
#define QMLINED_H

#include "qtablevw.h"
#include "qstring.h"
#include "qlist.h"

class QMultiLineEdit : public QTableView
{
    Q_OBJECT
public:
    QMultiLineEdit( QWidget *parent=0, const char *name=0 );
   ~QMultiLineEdit();

    const char *textLine( int line ) const;
    QString text() const;

    void insert( const char *s, int line = -1 );
    void remove( int );
    void clear();

    int numLines();

    bool	isReadOnly();

    bool	atBeginning() const;
    bool	atEnd() const;
    void	setFont( const QFont &font );

public slots:
    void	setText( const char * );
    void	selectAll();
    void	setReadOnly( bool );

signals:
    void	textChanged();

protected:

    // table view stuff
    void	paintCell( QPainter *, int row, int col );

    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	timerEvent( QTimerEvent * );

    bool	hasMarkedText() const;
    QString	markedText() const;
    int		textWidth( int );
    int		textWidth( QString * );


protected:
    void	insertChar( char );
    void 	newLine();
    void 	killLine();
    void	pageUp();
    void	pageDown();
    void	cursorLeft( bool mark=FALSE, int steps = 1 );
    void	cursorRight( bool mark=FALSE, int steps = 1 );
    void	cursorUp( bool mark=FALSE, int steps = 1 );
    void	cursorDown( bool mark=FALSE, int steps = 1 );
    void	backspace();
    void	del();
    void	home( bool mark=FALSE );
    void	end( bool mark=FALSE );

    int		lineLength( int row ) const;
    QString	*getString( int row ) const;


private slots:
    void	clipboardChanged();

private:
    QList<QString> *contents;
    bool	    readOnly;
    bool	    cursorOn;
    bool	    dummy;
    bool	    markIsOn;
    bool	    dragScrolling ;
    bool	    dragMarking;

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

    void	updateCellWidth();
    bool 	partiallyInvisible( int row );
    void	makeVisible();
    void	setBottomCell( int row );

    void	paste();

    // SHOULD THESE BECOME PROTECTED:
    void	newMark( int posx, int posy, bool copy=TRUE );
    void	turnMarkOff();
    void	markWord( int posx, int posy );
    void	copyText();



private:	// Disabled copy constructor and operator=
    QMultiLineEdit( const QMultiLineEdit & ) {}
    QMultiLineEdit &operator=( const QMultiLineEdit & ) { return *this; }
};

inline bool QMultiLineEdit::isReadOnly() { return readOnly; }

inline int QMultiLineEdit::lineLength( int row ) const
{
    return contents->at( row )->length();
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
    return contents->at( row );
}

inline int QMultiLineEdit::numLines()
{
    return contents->count();
}

#endif // QMLINED_H


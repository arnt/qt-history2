/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmlined.h#32 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QMLINED_H
#define QMLINED_H

#ifndef QT_H
#include "qtablevw.h"
#include "qstring.h"
#include "qlist.h"
#endif // QT_H

struct QMultiLineData;

class QMultiLineEdit : public QTableView
{
    Q_OBJECT
public:
    QMultiLineEdit( QWidget *parent=0, const char *name=0 );
   ~QMultiLineEdit();

    const char *textLine( int line ) const;
    QString text() const;

    int numLines() const;

    bool	isReadOnly() const;
    bool	isOverwriteMode() const;

    void	setFont( const QFont &font );
    virtual void insertLine( const char *s, int line = -1 );
    virtual void insertAt( const char *s, int line, int col );
    virtual void removeLine( int line );

    void 	cursorPosition( int *line, int *col ) const;
    void	setCursorPosition( int line, int col, bool mark = FALSE );
    void	getCursorPosition( int *line, int *col );
    bool	atBeginning() const;
    bool	atEnd() const;

    bool	autoUpdate()	const;
    void	setAutoUpdate( bool );

public slots:
    void       clear();
    void       setText( const char * );
    void       append( const char * );
    void       deselect();
    void       selectAll();
    void       setReadOnly( bool );
    void       setOverwriteMode( bool );
    void       paste();
    void       copyText();
    void       cut();

signals:
    void	textChanged();
    void	returnPressed();

protected:
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

    QPoint	cursorPoint() const;

protected:
    virtual void insertChar( char );
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

private slots:
    void	clipboardChanged();

private:
    QList<QString> *contents;
    QMultiLineData *mlData;

    bool	readOnly;
    bool	cursorOn;
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

    void	setWidth( int );
    void	updateCellWidth();
    bool 	partiallyInvisible( int row );
    void	makeVisible();
    void	setBottomCell( int row );

    void 	newMark( int posx, int posy, bool copy=TRUE );
    void 	markWord( int posx, int posy );
    int 	charClass( char );
    void	turnMarkOff();

private:	// Disabled copy constructor and operator=
    QMultiLineEdit( const QMultiLineEdit & );
    QMultiLineEdit &operator=( const QMultiLineEdit & );
};

inline bool QMultiLineEdit::isReadOnly() const { return readOnly; }

inline bool QMultiLineEdit::isOverwriteMode() const { return overWrite; }

inline void QMultiLineEdit::setOverwriteMode( bool on ) 
{ 
    overWrite = on;
 }

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

inline int QMultiLineEdit::numLines() const
{
    return contents->count();
}
#endif // QMLINED_H


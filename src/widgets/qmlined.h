/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmlined.h#7 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
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

    const char *text() const;

    void insert( QString s, int row = -1 );
    void remove( int );
    
    int count();

    bool	inputEnabled();

    bool	atBeginning();
    bool	atEnd();
public slots:
    void	setText( const char * );
    void	selectAll();
    void	enableInput( bool );

signals:
    void	textChanged();

protected:

    // table view stuff
    void	paintCell( QPainter *, int row, int col );


    void	mousePressEvent( QMouseEvent * );
    //    void	mouseMoveEvent( QMouseEvent * );
    //    void	mouseReleaseEvent( QMouseEvent * );
    //    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	timerEvent( QTimerEvent * );

    bool	hasMarkedText() const;
    QString	markedText() const;
    int		textWidth( int );
    int		textWidth( QString * );

private slots:
    void	clipboardChanged();

private:
    QList<QString> *contents;
    uint	    isInputEnabled : 1;
    uint	    cursorOn : 1;
    uint	    dummy : 1;

    int		cursorX;
    int		cursorY;
    int		curXPos;	// cell coord of cursor
    enum { BORDER = 3 };

    int		mapFromView( int xPos, int row );
    int		mapToView( int xIndex, int row );
    int		lineLength( int row );
    QString	*getString( int row );

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

    void	updateCellWidth();
    bool 	partiallyInvisible( int row );
    void	makeVisible();
    void	setBottomCell( int row );

    void	paste();
private:	// Disabled copy constructor and operator=
    QMultiLineEdit( const QMultiLineEdit & ) {}
    QMultiLineEdit &operator=( const QMultiLineEdit & ) { return *this; }
};

inline bool QMultiLineEdit::inputEnabled() { return isInputEnabled; }

inline int QMultiLineEdit::lineLength( int row )
{
    return contents->at( row )->length();
}

inline bool QMultiLineEdit::atEnd() 
{ 
    return cursorY == (int)contents->count() - 1 
	&& cursorX == lineLength( cursorY ) ; 
}

inline bool QMultiLineEdit::atBeginning() 
{ 
    return cursorY == 0 && cursorX == 0; 
}

inline QString *QMultiLineEdit::getString( int row )
{
    return contents->at( row );
}

inline int QMultiLineEdit::count()
{
    return contents->count();
}

#endif // QMLINED_H


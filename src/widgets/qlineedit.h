/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlineedit.h#4 $
**
** Definition of QLineEdit class
**
** Author  : Eirik Eng
** Created : 941011
**
** Copyright (c) 1994 by Troll Tech AS.	 All rights reserved.
**
***********************************************************************/

#ifndef QLINED_H
#define QLINED_H

#include "qwidget.h"
#include "qstring.h"


class QLineEdit : public QWidget
{
    Q_OBJECT
public:
    QLineEdit( QWidget *parent=0, const char *name=0 );
   ~QLineEdit();

    void    setText( const char * );
    char   *text() const;
    void    setMaxLength( int );
    int	    maxLength() const;

signals:
    void    textChanged(char*);

protected:
    bool    keyPressEvent( QKeyEvent * );
    bool    focusInEvent( QEvent * );
    void    focusOutEvent( QEvent * );
    void    paintEvent( QPaintEvent * );
    void    timerEvent( QTimerEvent * );
    void    resizeEvent( QResizeEvent * );
    void    mousePressEvent( QMouseEvent * );

private:
    void    init();
    void    paint( bool frame = FALSE );
    void    pixmapPaint();
    void    paintText( QPainter *, const QSize &, bool frame = FALSE );
    bool    cursorLeft();
    bool    cursorRight();
    bool    backspace();
    bool    remove();
    bool    home();
    bool    end();

    QString t;
    QPixMap *pm;
    uint    cursorPos;
    uint    offset;
    uint    maxLen;
    uint    cursorOn:1;
    uint    inTextFocus:1;
};


#endif // QLINED_H

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.h#3 $
**
** Definition of QButton class
**
** Author  : Haavard Nord
** Created : 940206
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QButton class is an abstract button. It is super class for QPushButton,
** QCheckBox etc.  The subclasses implement virtual functions to specify user
** interface functionality.
*****************************************************************************/

#ifndef QBUTTON_H
#define QBUTTON_H

#include "qwidget.h"


class QPixMapDict;


class QButton : public QWidget			// button class
{
    Q_OBJECT
public:
    QButton( QView *parent=0 );

    char    *text()		const;		// button text
    void     setText( const char * );

signals:
    void    pressed();
    void    released();
    void    clicked();

protected:
    bool    isDown() const { return buttonDown; }
    bool    isUp()   const { return !buttonDown; }

    void    switchOn();				// switch button on
    void    switchOff();			// switch button off
    bool    isOn()   const { return buttonOn; }

    void    setOnOffButton( bool );		// set to on/off type button
    bool    isOnOffButton()  const { return onOffButton; }

    virtual bool hitButton( const QPoint &pos ) const;
    virtual void drawButton( QPainter * );

    void    mousePressEvent( QMouseEvent * );
    void    mouseReleaseEvent( QMouseEvent * );
    void    mouseMoveEvent( QMouseEvent * );
    void    paintEvent( QPaintEvent * );

    static bool	    acceptPixmap( int, int );
    static QPixMap *findPixmap( const char * );
    static void	    savePixmap( const char *, const QPixMap * );

private:
    static void	    delPixmaps();
    static QPixMapDict *pmdict;
    static long	    pmsize;
    QString btext;
    uint    onOffButton	: 1;
    uint    buttonDown	: 1;
    uint    buttonOn	: 1;
};


#endif // QBUTTON_H

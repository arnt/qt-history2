/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbt.h#8 $
**
** Definition of QPushButton class
**
** Author  : Haavard Nord
** Created : 940221
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPUSHBT_H
#define QPUSHBT_H

#include "qbutton.h"


class QPushButton : public QButton
{
    Q_OBJECT
public:
    QPushButton( QWidget *parent=0, const char *name=0 );
    QPushButton( const char *label, QWidget *parent=0, const char *name=0 );

    void    setAutoDefault( bool autoDef );
    bool    isAutoDefault() const { return autoDefButton; }

    void    setDefault( bool def );
    bool    isDefault() const { return defButton; }

    void    resizeFitLabel();

  // Reimplemented move,resize etc. because of auto-sized default push buttons
    void    move( int x, int y );		// move push button
    void    move( const QPoint &p );
    void    resize( int w, int h );		// resize push button
    void    resize( const QSize & );
    void    changeGeometry( int x, int y, int w, int h );
    void    changeGeometry( const QRect & );	// move and resize

signals:
    void    becameDefault();

protected:
    void    drawButton( QPainter * );
    virtual void drawButtonFace( QPainter * );

private:
    void    init();

    uint    autoDefButton : 1;
    uint    defButton	  : 1;
    uint    lastDown	  : 1;
    uint    lastDef	  : 1;
};


#endif // QPUSHBT_H

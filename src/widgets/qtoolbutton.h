/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbutton.h#3 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QTOOLBUTTON_H
#define QTOOLBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#include "qstring.h"
#include "qpixmap.h"
#include "qiconset.h"
#endif // QT_H


class QToolButtonPrivate;

class QToolBar;


class QToolButton: public QButton
{
    Q_OBJECT
public:
    QToolButton( QWidget * parent = 0, const char * name = 0 );
    QToolButton( const QPixmap & pm, const char * textLabel,
		 const char * grouptext,
		 QObject * receiver, const char * slot,
		 QToolBar * parent, const char * name = 0 );
    ~QToolButton();

    QSize sizeHint() const;

    void setIconSet( const QIconSet & );
    QIconSet iconSet() const;
    
    bool usesBigPixmap() const { return ubp; }
    bool usesTextLabel() const { return utl; }
    const char * textLabel() const { return tl; }

public slots:
    virtual void setUsesBigPixmap( bool enable );
    virtual void setUsesTextLabel( bool enable );
    virtual void setTextLabel( const char *, bool = TRUE );

    void setToggleButton( bool enable );

    void setOn( bool enable );
    void toggle();

protected:
    void drawButton( QPainter * );
    void drawButtonLabel( QPainter * );

    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );

    bool uses3D() const;

private:
    void init();

    QPixmap bp;
    int bpID;
    QPixmap sp;
    int spID;

    QString tl;

    QToolButtonPrivate * d;
    QIconSet * s;

    uint utl: 1;
    uint ubp: 1;
};


#endif

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbutton.h#12 $
**
** Definition of a buttom customized for tool bar use
**
** Created : 979899
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

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


class Q_EXPORT QToolButton: public QButton
{
    Q_OBJECT
public:
    QToolButton( QWidget * parent = 0, const char *name = 0 );
    QToolButton( const QPixmap & pm, const QString &textLabel,
		 const QString& grouptext,
		 QObject * receiver, const char* slot,
		 QToolBar * parent, const char* name = 0 );
    QToolButton( QIconSet s, const QString &textLabel,
		 const QString& grouptext,
		 QObject * receiver, const char* slot,
		 QToolBar * parent, const char* name = 0 );
    ~QToolButton();

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void setIconSet( const QIconSet & );
    QIconSet iconSet() const;

    bool usesBigPixmap() const { return ubp; }
    bool usesTextLabel() const { return utl; }
    QString textLabel() const { return tl; }

public slots:
    virtual void setUsesBigPixmap( bool enable );
    virtual void setUsesTextLabel( bool enable );
    virtual void setTextLabel( const QString &, bool = TRUE );

    virtual void setToggleButton( bool enable );

    virtual void setOn( bool enable );
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

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtooltip.h#6 $
**
** Tool Tips (or Balloon Help) for any widget or rectangle
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#include "object.h"
#include "qrect.h"

class QWidget;
class QTipManager;


class QToolTipGroup: public QObject
{
    Q_OBJECT
public:
    QToolTipGroup( QObject * parent, const char * name );
    ~QToolTipGroup();

signals:
    showTip( const char * );
    removeTip();

    friend QTipManager;
};


class QToolTip
{
public:
    QToolTip( QWidget * );

    static void add( QWidget *, const char * );
    static void add( QWidget *, const char *,
		     QToolTipGroup *, const char * );
    static void remove( QWidget * );

    static void add( QWidget *, const QRect &, const char * );
    static void add( QWidget *, const QRect &, const char *,
		     QToolTipGroup *, const char * );
    static void remove( QWidget *, const QRect & );

protected:
    virtual void maybeTip( const QPoint & ) = 0;
    void tip( const QRect &, const char * );
    void clear();
};

#endif

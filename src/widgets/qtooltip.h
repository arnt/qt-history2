/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtooltip.h#16 $
**
** Tool Tips (or Balloon Help) for any widget or rectangle
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#include "qobject.h"
#include "qrect.h"
#include "qtimer.h"
#include "qintdict.h"

class QWidget;
class QTipManager;
class QLabel;

class QToolTipGroup: public QObject
{
    Q_OBJECT
public:
    QToolTipGroup( QObject * parent, const char * name = 0 );
    ~QToolTipGroup();

signals:
    void showTip( const char * );
    void removeTip();

private:
    friend QTipManager;
};


class QToolTip
{
public:
    QToolTip( QWidget *, QToolTipGroup * = 0 );

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
    void tip( const QRect &, const char *, const char * );
    void clear();

public:
    QWidget * parentWidget() const { return p; }
    QToolTipGroup * group() const { return g; }

private:
    QWidget * p;
    QToolTipGroup * g;
};

#endif

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtooltip.h#23 $
**
** Definition of Tool Tips (or Balloon Help) for any widget or rectangle
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#ifndef QT_H
#include "qwidget.h"
#include "qtimer.h"
#endif // QT_H


class QTipManager;
class QLabel;


class QToolTipGroup: public QObject
{
    Q_OBJECT
public:
    QToolTipGroup( QObject *parent, const char *name = 0 );
   ~QToolTipGroup();

signals:
    void showTip( const char * );
    void removeTip();

private:
    friend class QTipManager;
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

    static QFont    font();
    static void	    setFont( const QFont & );
    static QPalette palette();
    static void	    setPalette( const QPalette & );

protected:
    virtual void maybeTip( const QPoint & ) = 0;
    void    tip( const QRect &, const char * );
    void    tip( const QRect &, const char *, const char * );
    void    clear();

public:
    QWidget	  *parentWidget() const { return p; }
    QToolTipGroup *group()	  const { return g; }

private:
    QWidget	    *p;
    QToolTipGroup   *g;
    static QFont    *ttFont;
    static QPalette *ttPalette;

    static void initialize();
    static void cleanup();

    friend class QTipManager;
};


#endif // QTOOLTIP_H

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtooltip.h#31 $
**
** Definition of Tool Tips (or Balloon Help) for any widget or rectangle
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#ifndef QT_H
#include "qwidget.h"
#include "qtimer.h"
#endif // QT_H

#ifdef QT_FEATURE_WIDGETS


class QTipManager;
class QLabel;
class QIconViewToolTip;

class Q_EXPORT QToolTipGroup: public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool delay READ delay WRITE setDelay )
    Q_PROPERTY( bool enabled READ enabled WRITE setEnabled )

public:
    QToolTipGroup( QObject *parent, const char *name = 0 );
   ~QToolTipGroup();

    bool delay() const;
    bool enabled() const;

public slots:
    void setDelay( bool );
    void setEnabled( bool );

signals:
    void showTip( const QString &);
    void removeTip();

private:
    uint del:1;
    uint ena:1;

    friend class QTipManager;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QToolTipGroup( const QToolTipGroup & );
    QToolTipGroup& operator=( const QToolTipGroup & );
#endif
};


class Q_EXPORT QToolTip: public Qt
{
public:
    QToolTip( QWidget *, QToolTipGroup * = 0 );

    static void add( QWidget *, const QString &);
    static void add( QWidget *, const QString &,
		     QToolTipGroup *, const QString& );
    static void remove( QWidget * );

    static void add( QWidget *, const QRect &, const QString &);
    static void add( QWidget *, const QRect &, const QString &,
		     QToolTipGroup *, const QString& );
    static void remove( QWidget *, const QRect & );

    static void hide();

    static QFont    font();
    static void	    setFont( const QFont & );
    static QPalette palette();
    static void	    setPalette( const QPalette & );

    static void	    setEnabled( bool );
    static bool	    enabled();

protected:
    virtual void maybeTip( const QPoint & ) = 0;
    void    tip( const QRect &, const QString &);
    void    tip( const QRect &, const QString& , const QString &);
    void    clear();

public:
    QWidget	  *parentWidget() const { return p; }
    QToolTipGroup *group()	  const { return g; }

private:
    void    tip( const QRect &, const QRect &, const QString &);
    QWidget	    *p;
    QToolTipGroup   *g;
    static QFont    *ttFont;
    static QPalette *ttPalette;

    static void initialize();
    static void cleanup();

    friend class QTipManager;
    friend class QIconViewToolTip;
};


#endif // QT_FEATURE_WIDGETS

#endif // QTOOLTIP_H

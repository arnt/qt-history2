/****************************************************************************
**
** Definition of Tool Tips (or Balloon Help) for any widget or rectangle.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_TOOLTIP


class QTipManager;
class QIconViewToolTip;
class QListViewToolTip;

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
    //### add virtual d'tor for 4.0

    static void add( QWidget *, const QString &);
    static void add( QWidget *, const QString &,
		     QToolTipGroup *, const QString& );
    static void remove( QWidget * );

    static void add( QWidget *, const QRect &, const QString &);
    static void add( QWidget *, const QRect &, const QString &,
		     QToolTipGroup *, const QString& );
    static void remove( QWidget *, const QRect & );

    static QString textFor( QWidget *, const QPoint & pos = QPoint() );

    static void hide();

    static QFont    font();
    static void	    setFont( const QFont & );
    static QPalette palette();
    static void	    setPalette( const QPalette & );

#ifndef QT_NO_COMPAT
    static void	    setEnabled( bool enable ) { setGloballyEnabled( enable ); }
    static bool	    enabled() { return isGloballyEnabled(); }
#endif
    static void	    setGloballyEnabled( bool );
    static bool	    isGloballyEnabled();
    static void	    setWakeUpDelay(int);

protected:
    virtual void maybeTip( const QPoint & ) = 0;
    void    tip( const QRect &, const QString &);
    void    tip( const QRect &, const QString& , const QString &);
    void    tip( const QRect &, const QString &, const QRect & );
    void    tip( const QRect &, const QString&, const QString &, const QRect &);

    void    clear();

public:
    QWidget	  *parentWidget() const { return p; }
    QToolTipGroup *group()	  const { return g; }

private:
    QWidget	    *p;
    QToolTipGroup   *g;
    static QFont    *ttFont;
    static QPalette *ttPalette;

    friend class QTipManager;
};


#endif // QT_NO_TOOLTIP

#endif // QTOOLTIP_H

/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbutton.h#20 $
**
** Definition of a buttom customized for tool bar use
**
** Created : 979899
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
class QStyle;

class Q_EXPORT QToolButton: public QButton
{
    Q_OBJECT

    Q_PROPERTY( QIconSet onIconSet READ onIconSet WRITE setOnIconSet )
    Q_PROPERTY( QIconSet offIconSet READ offIconSet WRITE setOffIconSet )
    Q_PROPERTY( bool usesBigPixmap READ usesBigPixmap WRITE setUsesBigPixmap )
    Q_PROPERTY( bool usesTextLabel READ usesTextLabel WRITE setUsesTextLabel )
    Q_PROPERTY( QString textLabel READ textLabel WRITE setTextLabel )
    Q_PROPERTY( int popupDelay READ popupDelay WRITE setPopupDelay )
    Q_PROPERTY( bool autoRaise READ autoRaise WRITE setAutoRaise )

    Q_OVERRIDE( bool on WRITE setOn )

public:
    QToolButton( QWidget * parent, const char *name = 0 );
    QToolButton( const QPixmap & pm, const QString &textLabel, //### fjern 3.0
		 const QString& grouptext,
		 QObject * receiver, const char* slot,
		 QToolBar * parent, const char* name = 0 );
    QToolButton( const QIconSet& s, const QString &textLabel,
		 const QString& grouptext,
		 QObject * receiver, const char* slot,
		 QToolBar * parent, const char* name = 0 );
    QToolButton( ArrowType type, QWidget *parent, const char *name = 0 );
    ~QToolButton();

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

    void setOnIconSet( const QIconSet& );
    void setOffIconSet( const QIconSet& );
    QIconSet onIconSet() const;
    QIconSet offIconSet( ) const;
    virtual void setIconSet( const QIconSet &, bool on = FALSE );
    QIconSet iconSet( bool on = FALSE) const;

    bool usesBigPixmap() const { return ubp; }
    bool usesTextLabel() const { return utl; }
    QString textLabel() const { return tl; }

    void setPopup( QPopupMenu* popup );
    QPopupMenu* popup() const;

    void setPopupDelay( int delay );
    int popupDelay() const;

    void setAutoRaise( bool enable );
    bool autoRaise() const;

public slots:
    virtual void setUsesBigPixmap( bool enable );
    virtual void setUsesTextLabel( bool enable );
    virtual void setTextLabel( const QString &, bool );

    virtual void setToggleButton( bool enable ); //### fjern virtual 3.0

    virtual void setOn( bool enable ); //### fjern virtual 3.0
    void toggle();
    void setTextLabel( const QString & );

protected:
    void drawButton( QPainter * );
    void drawButtonLabel( QPainter * );

    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void moveEvent( QMoveEvent * );

    bool uses3D() const;

private slots:
    void popupTimerDone();
    void popupPressed();

private:
    void init();

    QPixmap bp;
    int bpID;
    QPixmap sp;
    int spID;

    QString tl;

    QToolButtonPrivate * d;
    QIconSet * s, *son;

    uint utl: 1;
    uint ubp: 1;
    uint hasArrow : 1;

    friend class QStyle;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QToolButton( const QToolButton & );
    QToolButton& operator=( const QToolButton & );
#endif
};


#endif

/****************************************************************************
**
** Definition of QToolButton class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

#ifndef QT_NO_TOOLBUTTON

class QToolButtonPrivate;
class QToolBar;
class QPopupMenu;

class Q_GUI_EXPORT QToolButton : public QButton
{
    Q_OBJECT
    Q_ENUMS( TextPosition )

    Q_PROPERTY( QIconSet iconSet READ iconSet WRITE setIconSet )
    Q_PROPERTY( QIconSet onIconSet READ onIconSet WRITE setOnIconSet DESIGNABLE false STORED false )
    Q_PROPERTY( QIconSet offIconSet READ offIconSet WRITE setOffIconSet DESIGNABLE false STORED false )
    Q_PROPERTY( bool usesBigPixmap READ usesBigPixmap WRITE setUsesBigPixmap )
    Q_PROPERTY( bool usesTextLabel READ usesTextLabel WRITE setUsesTextLabel )
    Q_PROPERTY( QString textLabel READ textLabel WRITE setTextLabel )
    Q_PROPERTY( int popupDelay READ popupDelay WRITE setPopupDelay )
    Q_PROPERTY( bool autoRaise READ autoRaise WRITE setAutoRaise )
    Q_PROPERTY( TextPosition textPosition READ textPosition WRITE setTextPosition )

    Q_OVERRIDE( bool toggleButton WRITE setToggleButton )
    Q_OVERRIDE( bool on WRITE setOn )
    Q_OVERRIDE( QPixmap pixmap DESIGNABLE false STORED false )
    Q_OVERRIDE( BackgroundMode backgroundMode DESIGNABLE true)

public:
    enum TextPosition {
	BesideIcon,
	BelowIcon,
	Right = BesideIcon, // obsolete
	Under = BelowIcon // obsolete
    };
    QToolButton( QWidget * parent=0, const char* name=0 );
#ifndef QT_NO_TOOLBAR
    QToolButton( const QIconSet& s, const QString &textLabel,
		 const QString& grouptext,
		 QObject * receiver, const char* slot,
		 QToolBar * parent=0, const char* name=0 );
#endif
    QToolButton( ArrowType type, QWidget *parent=0, const char* name=0 );
    ~QToolButton();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

#ifdef QT_COMPAT
    QT_COMPAT void setOnIconSet( const QIconSet& );
    QT_COMPAT void setOffIconSet( const QIconSet& );
    QT_COMPAT void setIconSet( const QIconSet &, bool on );
    QT_COMPAT QIconSet onIconSet() const;
    QT_COMPAT QIconSet offIconSet( ) const;
    QT_COMPAT QIconSet iconSet( bool on ) const;
#endif
    virtual void setIconSet( const QIconSet & );
    QIconSet iconSet() const;

    bool usesBigPixmap() const { return ubp; }
    bool usesTextLabel() const { return utl; }
    QString textLabel() const { return tl; }

#ifndef QT_NO_POPUPMENU
    void setPopup( QPopupMenu* popup );
    QPopupMenu* popup() const;

    void setPopupDelay( int delay );
    int popupDelay() const;

    void openPopup();
#endif

    void setAutoRaise( bool enable );
    bool autoRaise() const;
    TextPosition textPosition() const;

    void setText( const QString &txt );

public slots:
    virtual void setUsesBigPixmap( bool enable );
    virtual void setUsesTextLabel( bool enable );
    virtual void setTextLabel( const QString &, bool );

    virtual void setToggleButton( bool enable );

    virtual void setOn( bool enable );
    void toggle();
    void setTextLabel( const QString & );
    void setTextPosition( TextPosition pos );

protected:
    void mousePressEvent( QMouseEvent * );
    void drawButton( QPainter * );
    void drawButtonLabel(QPainter *);

    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void moveEvent( QMoveEvent * );
    void changeEvent( QEvent * );

    // ### Make virtual in 4.0, maybe act like QPushButton with
    // regards to setFlat() instead?  Andy
    virtual bool uses3D() const;

    bool eventFilter( QObject *o, QEvent *e );

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

    QToolButtonPrivate *d;
    QIconSet *s;

    uint utl : 1;
    uint ubp : 1;
    uint hasArrow : 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QToolButton( const QToolButton & );
    QToolButton& operator=( const QToolButton & );
#endif
};

#endif // QT_NO_TOOLBUTTON

#endif // QTOOLBUTTON_H

/****************************************************************************
** $Id: $
**
** Definition of QAction class
**
** Created : 000000
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QACTION_H
#define QACTION_H

#ifndef QT_H
#include "qobject.h"
#include "qiconset.h"
#include "qstring.h"
#include "qkeysequence.h"
#endif // QT_H

#ifndef QT_NO_ACTION

class QActionPrivate;
class QActionGroupPrivate;
class QStatusBar;
class QPopupMenu;
class QToolTipGroup;

class Q_EXPORT QAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool toggleAction READ isToggleAction WRITE setToggleAction)
    Q_PROPERTY( bool on READ isOn WRITE setOn )
    Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled )
    Q_PROPERTY( QIconSet iconSet READ iconSet WRITE setIconSet )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QString menuText READ menuText WRITE setMenuText )
    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )
    Q_PROPERTY( QString statusTip READ statusTip WRITE setStatusTip )
    Q_PROPERTY( QString whatsThis READ whatsThis WRITE setWhatsThis )
    Q_PROPERTY( QKeySequence accel READ accel WRITE setAccel )

public:
    QAction( QObject* parent, const char* name = 0, bool toggle = FALSE  );
    QAction( const QString& text, const QIconSet& icon, const QString& menuText, QKeySequence accel,
	     QObject* parent, const char* name = 0, bool toggle = FALSE );
    QAction( const QString& text, const QString& menuText, QKeySequence accel, QObject* parent,
	     const char* name = 0, bool toggle = FALSE );
    ~QAction();

    virtual void setIconSet( const QIconSet& );
    QIconSet iconSet() const;
    virtual void setText( const QString& );
    QString text() const;
    virtual void setMenuText( const QString& );
    QString menuText() const;
    virtual void setToolTip( const QString& );
    QString toolTip() const;
    virtual void setStatusTip( const QString& );
    QString statusTip() const;
    virtual void setWhatsThis( const QString& );
    QString whatsThis() const;
    virtual void setAccel( const QKeySequence& key );
    QKeySequence accel() const;
    virtual void setToggleAction( bool );
    
    bool isToggleAction() const;
    bool isOn() const;
    bool isEnabled() const;
    virtual bool addTo( QWidget* );
    virtual bool removeFrom( QWidget* );
    static void setStatusTipDelay( bool delay );

protected:
    virtual void addedTo( QWidget *actionWidget, QWidget *container );
    virtual void addedTo( int index, QPopupMenu *menu );

public slots:
    void toggle();
    virtual void setOn( bool );
    virtual void setEnabled( bool );

signals:
    void activated();
    void toggled( bool );

private slots:
    void internalActivation();
    void toolButtonToggled( bool );
    void objectDestroyed();
    void menuStatusText( int id );
    void showStatusText( const QString& );
    void clearStatusText();

private:
    void init();

    QActionPrivate* d;

};

class Q_EXPORT QActionGroup : public QAction
{
    Q_OBJECT
    Q_PROPERTY( bool exclusive READ isExclusive WRITE setExclusive )
    Q_PROPERTY( bool usesDropDown READ usesDropDown WRITE setUsesDropDown )

public:
    QActionGroup( QObject* parent, const char* name = 0, bool exclusive = TRUE );
    ~QActionGroup();
    void setExclusive( bool );
    bool isExclusive() const;
    void add( QAction* a);
    void addSeparator();
    bool addTo( QWidget* );
    bool removeFrom( QWidget* );
    void setEnabled( bool );

    void setUsesDropDown( bool enable );
    bool usesDropDown() const;

    void setIconSet( const QIconSet& );
    void setText( const QString& );
    void setMenuText( const QString& );
    void setToolTip( const QString& );
    void setWhatsThis( const QString& );

protected:
    void childEvent( QChildEvent* );
    virtual void addedTo( QWidget *actionWidget, QWidget *container, QAction *a );
    virtual void addedTo( int index, QPopupMenu *menu, QAction *a );
    virtual void addedTo( QWidget *actionWidget, QWidget *container );
    virtual void addedTo( int index, QPopupMenu *menu );

signals:
    void selected( QAction* );

private slots:
    void childToggled( bool );
    void childDestroyed();
    void internalComboBoxActivated( int );
    void internalToggle( QAction* );
    void objectDestroyed();

private:
    QActionGroupPrivate* d;

#ifndef QT_NO_COMPAT
public:
    void insert( QAction* a ) { add( a ); }
#endif

};

#endif

#endif

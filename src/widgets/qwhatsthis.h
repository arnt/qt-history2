/****************************************************************************
**
** Definition of QWhatsThis class.
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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_WHATSTHIS

#include "qcursor.h"

class QToolButton;
class QPopupMenu;
class QStyleSheet;

class Q_GUI_EXPORT Q4WhatsThis: public Qt
{
    Q4WhatsThis();
    Q4WhatsThis(const Q4WhatsThis &);
public:
    static void setFont( const QFont &font );
    static QFont font();

    static void setPalette( const QPalette &palette );
    static QPalette palette();

    static void add( QWidget *w, const QString &s); // deprecated
    static QToolButton * whatsThisButton( QWidget * parent ); // deprecated

    static void enterWhatsThisMode();
    static bool inWhatsThisMode();
    static void leaveWhatsThisMode();

    static void showText(int x, int y, const QString &text);
    static void showText(int x, int y, const QString &text,
			 const QObject *receiver, const char *member);
    static inline void showText(const QPoint &pos, const QString &text)
	{ showText(pos.x(), pos.y(), text); }
    static void showText(const QPoint &pos, const QString &text,
			 const QObject *receiver, const char *member)
	{ showText(pos.x(), pos.y(), text, receiver, member); }
};

#if 1 // def QT_COMPAT

class Q_GUI_EXPORT QWhatsThis: public Qt
{
public:
    QWhatsThis( QWidget *);
    virtual ~QWhatsThis();

    virtual QString text( const QPoint & );
    virtual bool clicked( const QString& href );

    // the common static functions
    static void setFont( const QFont &font );

    static void add( QWidget *, const QString &);
    static void remove( QWidget * );
    static QString textFor( QWidget *, const QPoint & pos = QPoint(), bool includeParents = FALSE );

    static QToolButton * whatsThisButton( QWidget * parent );

    static void enterWhatsThisMode();
    static bool inWhatsThisMode();
    static void leaveWhatsThisMode( const QString& = QString::null, const QPoint& pos = QCursor::pos(), QWidget* w = 0 );

    static void display( const QString& text, const QPoint& pos = QCursor::pos(), QWidget* w = 0 );
};
#endif // QT_COMPAT

#endif // QT_NO_WHATSTHIS

#endif // QWHATSTHIS_H

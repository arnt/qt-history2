/****************************************************************************
** $Id$
**
** Definition of QInputContext class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

/****************************************************************************
**
** Copyright (C) 1992-2004 Trolltech AS. All rights reserved.
**
** This file is part of the input method module of the Qt Toolkit.
**
** Licensees holding valid Qt Preview licenses may use this file in
** accordance with the Qt Preview License Agreement provided with the
** Software.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QINPUTCONTEXT_H
#define QINPUTCONTEXT_H

#ifndef QT_NO_IM

#include "qobject.h"
#include "qglobal.h"
#include "qevent.h"
#include "qstring.h"
#include "qlist.h"
#include "qaction.h"

class QWidget;
class QFont;
class QPopupMenu;
class QInputContextPrivate;


class Q_GUI_EXPORT QInputContext : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QInputContext)
public:
    QInputContext(QObject* parent = 0);
    virtual ~QInputContext();

    virtual QString identifierName() = 0;
    virtual QString language() = 0;

#if defined(Q_WS_X11)
    virtual bool x11FilterEvent( QWidget *keywidget, XEvent *event );
#endif // Q_WS_X11
    virtual bool filterEvent( const QEvent *event );
    virtual void reset();

    virtual void setFocus();
    virtual void unsetFocus();
    virtual void setMicroFocus( const QRect &rect, const QFont &font );
    virtual void mouseHandler( int x, QMouseEvent *event);
    virtual QFont font() const;
    virtual bool isComposing() const;
    virtual bool isPreeditRelocationEnabled();

    enum MenuAction {
	NoSeparator,
	InsertSeparator
    };
    virtual QList<QAction *> actions();
    void addActionsTo( QMenu *menu, MenuAction action = InsertSeparator );

#if defined(Q_WS_X11)
    // these functions are not recommended for ordinary use
    virtual QWidget *focusWidget() const;
    virtual QWidget *holderWidget() const;

    // these functions must not be used by ordinary input method
    virtual void setFocusWidget( QWidget *w );
    virtual void setHolderWidget( QWidget *w );
    virtual void releaseComposingWidget( QWidget *w );
#endif

protected:
    virtual void sendIMEvent( QEvent::Type type,
			      const QString &text = QString::null,
			      int cursorPosition = -1, int selLength = 0 );

private:
    friend class QWidget;
    friend class QInputContextFactory;

private:   // Disabled copy constructor and operator=
    QInputContext( const QInputContext & );
    QInputContext &operator=( const QInputContext & );

};

#endif //Q_NO_IM

#endif // QINPUTCONTEXT_H

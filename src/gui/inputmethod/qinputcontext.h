/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** Definition of QInputContext class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to $TROLLTECH$ under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef QINPUTCONTEXT_H
#define QINPUTCONTEXT_H

#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>
#include <QtGui/qevent.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtGui/qaction.h>

#ifndef QT_NO_IM

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QWidget;
class QFont;
class QPopupMenu;
class QInputContextPrivate;


class Q_GUI_EXPORT QInputContext : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QInputContext)
public:
    explicit QInputContext(QObject* parent = 0);
    virtual ~QInputContext();

    virtual QString identifierName() = 0;
    virtual QString language() = 0;

    virtual void reset() = 0;
    virtual void update();

    virtual void mouseHandler( int x, QMouseEvent *event);
    virtual QFont font() const;
    virtual bool isComposing() const = 0;

    QWidget *focusWidget() const;
    virtual void setFocusWidget( QWidget *w );

    virtual void widgetDestroyed(QWidget *w);

    virtual QList<QAction *> actions();

#if defined(Q_WS_X11)
    virtual bool x11FilterEvent( QWidget *keywidget, XEvent *event );
#endif // Q_WS_X11
    virtual bool filterEvent( const QEvent *event );

    void sendEvent(const QInputMethodEvent &event);

    enum StandardFormat {
        PreeditFormat,
        SelectionFormat
    };
    QTextFormat standardFormat(StandardFormat s) const;
private:
    friend class QWidget;
    friend class QInputContextFactory;
    friend class QApplication;

private:   // Disabled copy constructor and operator=
    QInputContext( const QInputContext & );
    QInputContext &operator=( const QInputContext & );

};

QT_END_HEADER

#endif //Q_NO_IM

#endif // QINPUTCONTEXT_H

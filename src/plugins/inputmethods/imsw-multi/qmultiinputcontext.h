/****************************************************************************
** $Id$
**
** Definition of QMultiInputContext class
**
** Copyright (C) 2004 immodule for Qt Project.  All rights reserved.
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

#ifndef QMULTIINPUTCONTEXT_H
#define QMULTIINPUTCONTEXT_H

#ifndef QT_NO_IM

#include <qobject.h>
#include <qstring.h>
#include <qnamespace.h>
#include <qwidget.h>
#include <qinputcontext.h>
#include <qmap.h>
#include <qpointer.h>
#include <qlist.h>

class QMultiInputContext : public QInputContext
{
    Q_OBJECT
public:
    QMultiInputContext();
    ~QMultiInputContext();

    QString identifierName();
    QString language();

#if defined(Q_WS_X11)
    bool x11FilterEvent( QWidget *keywidget, XEvent *event );
#endif // Q_WS_X11
    bool filterEvent( const QEvent *event );

    void reset();
    void update();
    void mouseHandler( int x, QMouseEvent *event );
    QFont font() const;
    bool isComposing() const;

    QList<QAction *> actions();

    QWidget *focusWidget() const;
    void setFocusWidget(QWidget *w);

    void widgetDestroyed( QWidget *w );

    QInputContext *slave() { return slaves.at(current); }
    const QInputContext *slave() const { return slaves.at(current); }

protected slots:
    void changeSlave(QAction *);
private:
    QWidget *fw;
    int current;
    QList<QInputContext *> slaves;
    QMenu *menu;
    QAction *separator;
};

#endif //Q_NO_IM

#endif // QMULTIINPUTCONTEXT_H

/****************************************************************************
**
** Definition of Q3WhatsThis class.
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

#ifndef Q3WHATSTHIS_H
#define Q3WHATSTHIS_H

#ifndef QT_H
#include "qwhatsthis.h"
#include "qobject.h"
#include "qcursor.h"
#endif // QT_H

class QToolButton;
class Q3PopupMenu;
class QStyleSheet;

class Q_COMPAT_EXPORT Q3WhatsThis: public QObject
{
    Q_OBJECT
public:
    Q3WhatsThis(QWidget *);
    ~Q3WhatsThis();
    bool eventFilter(QObject *, QEvent *);

    static inline void enterWhatsThisMode();
    static inline bool inWhatsThisMode();
    static inline void leaveWhatsThisMode();

    static inline void add(QWidget *w, const QString &s) { QWhatsThis::add(w, s); }
    static inline void remove(QWidget *w) { QWhatsThis::remove(w); }
    static QToolButton * whatsThisButton(QWidget * parent) { return QWhatsThis::whatsThisButton(parent); }
    static inline void leaveWhatsThisMode(const QString& text = QString::null, const QPoint& pos = QCursor::pos(), QWidget* w = 0)
        { QWhatsThis::showText(pos, text, w); }
    static inline void display(const QString& text, const QPoint& pos = QCursor::pos(), QWidget* w = 0)
        { QWhatsThis::showText(pos, text, w); }

    virtual QString text(const QPoint &);
    virtual bool clicked(const QString& href);

private slots:
    void hyperLinkClicked(const QString &);
};

#endif // QWHATSTHIS_H

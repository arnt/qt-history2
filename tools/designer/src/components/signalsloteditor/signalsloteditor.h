/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SIGNALSLOTEDITOR_H
#define SIGNALSLOTEDITOR_H

#include "signalsloteditor_global.h"

#include <connectionedit.h>
#include <abstractformeditor.h>
#include <abstractformwindow.h>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class DomConnections;
class DomConnection;

class SignalSlotConnection : public Connection
{
    Q_OBJECT

public:
    SignalSlotConnection(ConnectionEdit *edit);

    void setSignal(const QString &signal);
    void setSlot(const QString &slot);

    QString sender() const;
    QString receiver() const;
    inline QString signal() const { return m_signal; }
    inline QString slot() const { return m_slot; }

    DomConnection *toUi() const;

private:
    QString m_signal, m_slot;
};

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditor : public ConnectionEdit
{
public:
    SignalSlotEditor(AbstractFormWindow *form_window, QWidget *parent);
    static void registerExtensions(AbstractFormEditor *core);

    DomConnections *toUi() const;
    void fromUi(DomConnections *connections, QWidget *parent);

    AbstractFormWindow *formWindow() const { return m_form_window; }

    static QWidget *widgetByName(QWidget *topLevel, const QString &name);

protected:
    virtual QWidget *widgetAt(const QPoint &pos) const;

private:
    virtual Connection *createConnection(QWidget *source, QWidget *destination);

    AbstractFormWindow *m_form_window;
};

#endif // SIGNALSLOTEDITOR_H

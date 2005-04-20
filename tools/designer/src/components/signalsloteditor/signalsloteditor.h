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
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class DomConnections;
class DomConnection;
class SignalSlotDialog;

class SignalSlotConnection : public Connection
{
public:
    SignalSlotConnection(ConnectionEdit *edit, QWidget *source = 0, QWidget *target = 0);

    void setSignal(const QString &signal);
    void setSlot(const QString &slot);

    QString sender() const;
    QString receiver() const;
    inline QString signal() const { return m_signal; }
    inline QString slot() const { return m_slot; }

    DomConnection *toUi() const;

    virtual void updateVisibility();

private:
    QString m_signal, m_slot;
};

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditor : public ConnectionEdit
{
    Q_OBJECT

public:
    SignalSlotEditor(QDesignerFormWindowInterface *form_window, QWidget *parent);
    static void registerExtensions(QDesignerFormEditorInterface *core);

    virtual void setSignal(SignalSlotConnection *con, const QString &member);
    virtual void setSlot(SignalSlotConnection *con, const QString &member);

    DomConnections *toUi() const;
    void fromUi(DomConnections *connections, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const { return m_form_window; }

    static QWidget *widgetByName(QWidget *topLevel, const QString &name);

    void showSignalSlotDialog(bool show);

protected:
    virtual QWidget *widgetAt(const QPoint &pos) const;

private:
    virtual Connection *createConnection(QWidget *source, QWidget *destination);
    virtual void modifyConnection(Connection *con);

    QDesignerFormWindowInterface *m_form_window;
    SignalSlotDialog *m_dialog;

    friend class SetMemberCommand;
};

#endif // SIGNALSLOTEDITOR_H

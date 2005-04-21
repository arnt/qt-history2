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
class QAbstractItemModel;

namespace qdesigner { namespace components { namespace signalsloteditor {

class SignalSlotDialog;
class SignalSlotConnection;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditor : public ConnectionEdit
{
    Q_OBJECT

public:
    SignalSlotEditor(QDesignerFormWindowInterface *form_window, QWidget *parent);

    virtual void setSignal(SignalSlotConnection *con, const QString &member);
    virtual void setSlot(SignalSlotConnection *con, const QString &member);
    virtual void setSource(Connection *con, const QString &obj_name);
    virtual void setTarget(Connection *con, const QString &obj_name);

    DomConnections *toUi() const;
    void fromUi(DomConnections *connections, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const { return m_form_window; }

    static QWidget *widgetByName(QWidget *topLevel, const QString &name);

    QAbstractItemModel *model() const;

protected:
    virtual QWidget *widgetAt(const QPoint &pos) const;

private:
    virtual Connection *createConnection(QWidget *source, QWidget *destination);
    virtual void modifyConnection(Connection *con);

    QDesignerFormWindowInterface *m_form_window;
    QAbstractItemModel *m_model;

    friend class SetMemberCommand;
};

} // namespace signalsloteditor
} // namespace components
} // namespace qdesigner

#endif // SIGNALSLOTEDITOR_H

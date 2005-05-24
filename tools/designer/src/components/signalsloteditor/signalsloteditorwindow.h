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

#ifndef SIGNALSLOTEDITORWINDOW_H
#define SIGNALSLOTEDITORWINDOW_H

#include <QtCore/QPointer>
#include <QtGui/QWidget>

class Connection;
class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QModelIndex;
class QTreeView;
class QToolButton;

namespace qdesigner_internal {

class SignalSlotEditor;
class ConnectionModel;

class SignalSlotEditorWindow : public QWidget
{
    Q_OBJECT
public:
    SignalSlotEditorWindow(QDesignerFormEditorInterface *core, QWidget *parent = 0);

public slots:
    void setActiveFormWindow(QDesignerFormWindowInterface *form);

private slots:
    void updateDialogSelection(Connection *con);
    void updateEditorSelection(const QModelIndex &index);

    void addConnection();
    void removeConnection();
    void updateUi();

private:
    QTreeView *m_view;
    QPointer<SignalSlotEditor> m_editor;
    QToolButton *m_add_button, *m_remove_button;

    bool m_handling_selection_change;
};

} // namespace qdesigner_internal

#endif // SIGNALSLOTEDITORWINDOW_H


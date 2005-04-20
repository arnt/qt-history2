#ifndef SIGNALSLOTEDITORWINDOW_H
#define SIGNALSLOTEDITORWINDOW_H

#include <QtCore/QPointer>
#include <QtGui/QWidget>

class Connection;
class QDesignerFormWindowInterface;
class QModelIndex;
class QTreeView;

namespace qdesigner { namespace components { namespace signalsloteditor {

class SignalSlotEditor;
class ConnectionModel;

class SignalSlotEditorWindow : public QWidget
{
    Q_OBJECT
public:
    SignalSlotEditorWindow(QWidget *parent = 0);

public slots:
    void setActiveFormWindow(QDesignerFormWindowInterface *form);

private slots:
    void updateDialogSelection(Connection *con);
    void updateEditorSelection(const QModelIndex &index);

private:
    QTreeView *m_view;
    QPointer<SignalSlotEditor> m_editor;

    bool m_handling_selection_change;
};

} // namespace signalsloteditor
} // namespace components
} // namespace qdesigner

#endif // SIGNALSLOTEDITORWINDOW_H


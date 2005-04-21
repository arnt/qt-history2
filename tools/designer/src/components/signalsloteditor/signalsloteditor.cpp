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

#include <qtundo.h>
#include "signalsloteditor.h"
#include "default_membersheet.h"
#include "signalsloteditor_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractmetadatabase.h>
#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/ui4.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowcursor.h>

#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QItemDelegate>
#include <QtGui/QItemEditorFactory>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QStandardItemModel>
#include <QtGui/QHeaderView>
#include <QtGui/QApplication>

#include <QtCore/qdebug.h>

namespace qdesigner { namespace components { namespace signalsloteditor {

/*******************************************************************************
** Tools
*/

QStringList objectNameList(QDesignerFormWindowInterface *form)
{
    QDesignerFormWindowCursorInterface *cursor = form->cursor();
    QStringList result;
    for (int i = 0; i < cursor->widgetCount(); ++i)
        result.append(cursor->widget(i)->objectName());
    result.sort();
    return result;
}

QStringList memberList(QDesignerFormWindowInterface *form, QWidget *widget, MemberType member_type)
{
    QStringList result;

    if (widget == 0)
        return result;

    QDesignerMemberSheetExtension *members
        = qt_extension<QDesignerMemberSheetExtension*>
                (form->core()->extensionManager(), widget);;
    Q_ASSERT(members != 0);

    for (int i = 0; i < members->count(); ++i) {
        if (!members->isVisible(i))
            continue;

        if (member_type == SignalMember && !members->isSignal(i))
            continue;

        if (member_type == SlotMember && !members->isSlot(i))
            continue;

        result.append(members->signature(i));
    }

    return result;
}

bool signalMatchesSlot(const QString &signal, const QString &slot)
{
    bool result = true;

    do {
        int signal_idx = signal.indexOf(QLatin1Char('('));
        int slot_idx = slot.indexOf(QLatin1Char('('));
        if (signal_idx == -1 || slot_idx == -1)
            break;

        ++signal_idx; ++slot_idx;

        if (slot.at(slot_idx) == QLatin1Char(')'))
            break;

        while (signal_idx < signal.size() && slot_idx < slot.size()) {
            QChar signal_c = signal.at(signal_idx);
            QChar slot_c = slot.at(slot_idx);

            if (signal_c == QLatin1Char(',') && slot_c == QLatin1Char(')'))
                break;

            if (signal_c == QLatin1Char(')') && slot_c == QLatin1Char(')'))
                break;

            if (signal_c != slot_c) {
                result = false;
                break;
            }

            ++signal_idx; ++slot_idx;
        }
    } while (false);

    return result;
}

ClassList classList(const QString &obj_name, MemberType member_type,
                            const QString &peer, QDesignerFormWindowInterface *form)
{
    ClassList result;

    QWidget *w = qFindChild<QWidget*>(form, obj_name);

    if (w == 0)
        return result;

    QDesignerMemberSheetExtension *members
        = qt_extension<QDesignerMemberSheetExtension*>
                (form->core()->extensionManager(), w);;
    Q_ASSERT(members != 0);

    QString class_name;
    QStringList member_list;
    for (int i = members->count(); i >= 0; --i) {
        if (!members->isVisible(i))
            continue;

        if (member_type == SignalMember && !members->isSignal(i))
            continue;

        if (member_type == SlotMember && !members->isSlot(i))
            continue;

        QString signal = member_type == SignalMember ? members->signature(i) : peer;
        QString slot = member_type == SignalMember ? peer : members->signature(i);
        if (!signalMatchesSlot(signal, slot))
            continue;

        QString s = members->declaredInClass(i);
        if (s != class_name) {
            if (!member_list.isEmpty())
                result.append(ClassInfo(class_name, member_list));
            class_name = s;
            member_list.clear();
        }
        member_list.append(members->signature(i));
    }
    if (!member_list.isEmpty())
        result.append(ClassInfo(class_name, member_list));

    return result;
}

/*******************************************************************************
** OldSignalSlotDialog
*/

class OldSignalSlotDialog : public QDialog
{
    Q_OBJECT
public:
    OldSignalSlotDialog(QDesignerFormEditorInterface *core, QWidget *sender, QWidget *receiver, QWidget *parent = 0);

    QString signal() const;
    QString slot() const;

    void setSignalSlot(const QString &signal, const QString &slot);

private slots:
    void selectSignal(QListWidgetItem *item);
    void selectSlot(QListWidgetItem *item);
    void populateSignalList();
    void populateSlotList(const QString &signal = QString());

private:
    QListWidget *m_signal_list, *m_slot_list;
    QPushButton *m_ok_button;
    QWidget *m_source, *m_destination;
    QDesignerFormEditorInterface *m_core;
    QCheckBox *m_show_all_checkbox;
};

static QString realObjectName(QDesignerFormEditorInterface *core, QWidget *widget)
{
    if (widget == 0)
        return QString();

    QString object_name = widget->objectName();
    QDesignerMetaDataBaseInterface *mdb = core->metaDataBase();
    QDesignerMetaDataBaseItemInterface *item = mdb->item(widget);
    if (item != 0)
        object_name = item->name();
    return object_name;
}

static QString realClassName(QDesignerFormEditorInterface *core, QWidget *widget)
{
    QString class_name = QLatin1String(widget->metaObject()->className());
    QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
    int idx = wdb->indexOfObject(widget);
    if (idx != -1)
        class_name = wdb->item(idx)->name();
    return class_name;
}

static QString widgetLabel(QDesignerFormEditorInterface *core, QWidget *widget)
{
    return QString::fromUtf8("%1 (%2)")
            .arg(realObjectName(core, widget))
            .arg(realClassName(core, widget));
}

void OldSignalSlotDialog::populateSlotList(const QString &signal)
{
    QString selectedName;
    QList<QListWidgetItem *> list = m_slot_list->selectedItems();
    if (list.size() > 0) {
        QListWidgetItem *item = list.at(0);
        selectedName = item->text();
    }
    m_slot_list->clear();

    bool show_all = m_show_all_checkbox->isChecked();

    QStringList signatures;

    if (QDesignerMemberSheetExtension *members
            = qt_extension<QDesignerMemberSheetExtension*>
                    (m_core->extensionManager(), m_destination)) {
        for (int i=0; i<members->count(); ++i) {
            if (!members->isVisible(i))
                continue;

            if (!show_all && members->inheritedFromWidget(i))
                continue;

            if (members->isSlot(i)) {
                if (!signalMatchesSlot(signal, members->signature(i)))
                    continue;

                signatures.append(members->signature(i));
            }
        }
    }

    signatures.sort();

    foreach (QString sig, signatures) {
        QListWidgetItem *item = new QListWidgetItem(m_slot_list);
        item->setText(sig);
        if (sig == selectedName)
            m_slot_list->setItemSelected(item, true);
    }

    if (m_slot_list->selectedItems().isEmpty())
        m_ok_button->setEnabled(false);
}

void OldSignalSlotDialog::populateSignalList()
{
    QString selectedName;
    QList<QListWidgetItem *> list = m_signal_list->selectedItems();
    if (list.size() > 0) {
        QListWidgetItem *item = list.at(0);
        selectedName = item->text();
    }
    m_signal_list->clear();

    bool show_all = m_show_all_checkbox->isChecked();

    QStringList signatures;

    if (QDesignerMemberSheetExtension *members = qt_extension<QDesignerMemberSheetExtension*>(m_core->extensionManager(), m_source)) {
        for (int i=0; i<members->count(); ++i) {
            if (!members->isVisible(i))
                continue;

            if (!show_all && members->inheritedFromWidget(i))
                continue;

            if (members->isSignal(i)) {
                signatures.append(members->signature(i));
            }
        }
    }

    signatures.sort();

    bool found_selected = false;
    foreach (QString sig, signatures) {
        QListWidgetItem *item = new QListWidgetItem(m_signal_list);
        item->setText(sig);
        if (!selectedName.isEmpty() && sig == selectedName) {
            m_signal_list->setItemSelected(item, true);
            found_selected = true;
        }
    }

    if (!found_selected)
        selectedName.clear();

    populateSlotList(selectedName);
    if (!found_selected) {
        m_slot_list->setEnabled(false);
    }
}

// ### use designer
OldSignalSlotDialog::OldSignalSlotDialog(QDesignerFormEditorInterface *core, QWidget *source, QWidget *destination,
                                    QWidget *parent)
    : QDialog(parent)
{
    m_source = source;
    m_destination = destination;
    m_core = core;

    m_signal_list = new QListWidget(this);
    connect(m_signal_list,
                SIGNAL(itemClicked(QListWidgetItem*)),
            this,
                SLOT(selectSignal(QListWidgetItem*)));
    m_slot_list = new QListWidget(this);
    connect(m_slot_list,
                SIGNAL(itemClicked(QListWidgetItem*)),
            this,
                SLOT(selectSlot(QListWidgetItem*)));
    m_slot_list->setEnabled(false);

    QPushButton *cancel_button = new QPushButton(tr("Cancel"), this);
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    m_ok_button = new QPushButton(tr("OK"), this);
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    m_ok_button->setEnabled(false);
    m_show_all_checkbox = new QCheckBox(tr("Show all signals and slots"), this);
    m_show_all_checkbox->setChecked(false);
    connect(m_show_all_checkbox, SIGNAL(toggled(bool)), this, SLOT(populateSignalList()));

    QLabel *source_label = new QLabel(this);
    source_label->setText(widgetLabel(core, source));
    QLabel *destination_label = new QLabel(this);
    destination_label->setText(widgetLabel(core, destination));

    QVBoxLayout *l1 = new QVBoxLayout(this);

    QHBoxLayout *l2 = new QHBoxLayout();
    l1->addLayout(l2);

    QVBoxLayout *l3 = new QVBoxLayout();
    l2->addLayout(l3);

    l3->addWidget(source_label);
    l3->addWidget(m_signal_list);

    QVBoxLayout *l4 = new QVBoxLayout();
    l2->addLayout(l4);

    l4->addWidget(destination_label);
    l4->addWidget(m_slot_list);

    QHBoxLayout *l5 = new QHBoxLayout();
    l1->addLayout(l5);

    l5->addWidget(m_show_all_checkbox);
    l5->addStretch();
#ifdef Q_WS_MAC
    l5->addWidget(cancel_button);
    l5->addWidget(m_ok_button);
#else
    l5->addWidget(m_ok_button);
    l5->addWidget(cancel_button);
#endif

    setWindowTitle(tr("Configure Connection"));

    populateSignalList();
}

static QListWidgetItem *findItem(const QListWidget &list_widget, const QString &text)
{
    QListWidgetItem *result = 0;
    for (int i = 0; i < list_widget.count(); ++i) {
        QListWidgetItem *item = list_widget.item(i);
        if (item->text() == text) {
            result = item;
            break;
        }
    }
    return result;
}

void OldSignalSlotDialog::setSignalSlot(const QString &signal, const QString &slot)
{
    QListWidgetItem *sig_item = findItem(*m_signal_list, signal);

    if (sig_item == 0) {
        m_show_all_checkbox->setChecked(true);
        sig_item = findItem(*m_signal_list, signal);
    }

    if (sig_item != 0) {
        selectSignal(sig_item);
        QListWidgetItem *slot_item = findItem(*m_slot_list, slot);
        if (slot_item == 0) {
            m_show_all_checkbox->setChecked(true);
            slot_item = findItem(*m_slot_list, slot);
        }
        if (slot_item != 0)
            selectSlot(slot_item);
    }
}

void OldSignalSlotDialog::selectSignal(QListWidgetItem *item)
{
    if (item == 0) {
        m_signal_list->clearSelection();
        populateSlotList();
        m_slot_list->setEnabled(false);
        m_ok_button->setEnabled(false);
    } else {
        m_signal_list->setItemSelected(item, true);
        m_signal_list->scrollToItem(item);
        populateSlotList(item->text());
        m_slot_list->setEnabled(true);
        m_ok_button->setEnabled(!m_slot_list->selectedItems().isEmpty());
    }
}

void OldSignalSlotDialog::selectSlot(QListWidgetItem *item)
{
    if (item == 0) {
        m_slot_list->clearSelection();
    } else {
        m_slot_list->setItemSelected(item, true);
        m_slot_list->scrollToItem(item);
    }
    m_ok_button->setEnabled(true);
}

QString OldSignalSlotDialog::signal() const
{
    QList<QListWidgetItem*> item_list = m_signal_list->selectedItems();
    if (item_list.size() != 1)
        return QString();
    return item_list.at(0)->text();
}

QString OldSignalSlotDialog::slot() const
{
    QList<QListWidgetItem*> item_list = m_slot_list->selectedItems();
    if (item_list.size() != 1)
        return QString();
    return item_list.at(0)->text();
}

/*******************************************************************************
** SignalSlotConnection
*/

SignalSlotConnection::SignalSlotConnection(ConnectionEdit *edit, QWidget *source, QWidget *target)
    : Connection(edit, source, target)
{
}

DomConnection *SignalSlotConnection::toUi() const
{
    DomConnection *result = new DomConnection;

    result->setElementSender(sender());
    result->setElementSignal(signal());
    result->setElementReceiver(receiver());
    result->setElementSlot(slot());

    DomConnectionHints *hints = new DomConnectionHints;
    QList<DomConnectionHint*> list;

    QPoint sp = endPointPos(EndPoint::Source);
    QPoint tp = endPointPos(EndPoint::Target);

    DomConnectionHint *hint = new DomConnectionHint;
    hint->setAttributeType(QLatin1String("sourcelabel"));
    hint->setElementX(sp.x());
    hint->setElementY(sp.y());
    list.append(hint);

    hint = new DomConnectionHint;
    hint->setAttributeType(QLatin1String("destinationlabel"));
    hint->setElementX(tp.x());
    hint->setElementY(tp.y());
    list.append(hint);

    hints->setElementHint(list);
    result->setElementHints(hints);

    return result;
}

void SignalSlotConnection::setSignal(const QString &signal)
{


    m_signal = signal;
    setLabel(EndPoint::Source, m_signal);
}

void SignalSlotConnection::setSlot(const QString &slot)
{
    m_slot = slot;
    setLabel(EndPoint::Target, m_slot);
}

QString SignalSlotConnection::sender() const
{
    SignalSlotEditor *edit = qobject_cast<SignalSlotEditor*>(this->edit());
    Q_ASSERT(edit != 0);

    return realObjectName(edit->formWindow()->core(), widget(EndPoint::Source));
}

QString SignalSlotConnection::receiver() const
{
    SignalSlotEditor *edit = qobject_cast<SignalSlotEditor*>(this->edit());
    Q_ASSERT(edit != 0);

    return realObjectName(edit->formWindow()->core(), widget(EndPoint::Target));
}

void SignalSlotConnection::updateVisibility()
{
    Connection::updateVisibility();
    if (isVisible() && (signal().isEmpty() || slot().isEmpty()))
        setVisible(false);
}

/*******************************************************************************
** Commands
*/

class SetMemberCommand : public QtCommand, public CETypes
{
    Q_OBJECT
public:
    SetMemberCommand(SignalSlotConnection *con, EndPoint::Type type,
                        const QString &member, SignalSlotEditor *editor);
    virtual void redo();
    virtual void undo();
private:
    SignalSlotConnection *m_con;
    QString m_old_member, m_new_member;
    EndPoint::Type m_type;
    SignalSlotEditor *m_editor;
};

SetMemberCommand::SetMemberCommand(SignalSlotConnection *con, EndPoint::Type type,
                                    const QString &member, SignalSlotEditor *editor)
{
    m_con = con;
    m_editor = editor;
    m_type = type;
    m_old_member = type == EndPoint::Source ? con->signal() : con->slot();
    m_new_member = member;

    setDescription(tr("Change %1").arg(type == EndPoint::Source ? tr("signal") : tr("slot")));
}

void SetMemberCommand::redo()
{
    m_con->update();
    if (m_type == EndPoint::Source)
        m_con->setSignal(m_new_member);
    else
        m_con->setSlot(m_new_member);
    m_con->update();
    emit m_editor->connectionChanged(m_con);
}

void SetMemberCommand::undo()
{
    m_con->update();
    if (m_type == EndPoint::Source)
        m_con->setSignal(m_old_member);
    else
        m_con->setSlot(m_old_member);
    m_con->update();
    emit m_editor->connectionChanged(m_con);
}

/*******************************************************************************
** SignalSlotEditor
*/

SignalSlotEditor::SignalSlotEditor(QDesignerFormWindowInterface *form_window, QWidget *parent)
    : ConnectionEdit(parent, form_window)
{
    m_form_window = form_window;
    m_model = new ConnectionModel(this, this);

    QDesignerFormEditorInterface *core = form_window->core();
    QDesignerMemberSheetFactory *factory = new QDesignerMemberSheetFactory(core->extensionManager());
    core->extensionManager()->registerExtensions(factory, Q_TYPEID(QDesignerMemberSheetExtension));
}

QAbstractItemModel *SignalSlotEditor::model() const
{
    return m_model;
}

void SignalSlotEditor::modifyConnection(Connection *con)
{
    SignalSlotConnection *sigslot_con = static_cast<SignalSlotConnection*>(con);

    OldSignalSlotDialog *dialog = new OldSignalSlotDialog(m_form_window->core(),
                                                    sigslot_con->widget(EndPoint::Source),
                                                    sigslot_con->widget(EndPoint::Target));
    dialog->setSignalSlot(sigslot_con->signal(), sigslot_con->slot());
    if (dialog->exec() == QDialog::Accepted) {
        sigslot_con->setSignal(dialog->signal());
        sigslot_con->setSlot(dialog->slot());
    }
}

Connection *SignalSlotEditor::createConnection(QWidget *source, QWidget *destination)
{
    SignalSlotConnection *con = 0;

    Q_ASSERT(source != 0);
    Q_ASSERT(destination != 0);

    OldSignalSlotDialog *dialog = new OldSignalSlotDialog(m_form_window->core(), source, destination);

    if (dialog->exec() == QDialog::Accepted) {
        con = new SignalSlotConnection(this, source, destination);
        con->setSignal(dialog->signal());
        con->setSlot(dialog->slot());
    }

    delete dialog;

    return con;
}

DomConnections *SignalSlotEditor::toUi() const
{
    DomConnections *result = new DomConnections;
    QList<DomConnection*> list;
    for (int i = 0; i < connectionCount(); ++i) {
        SignalSlotConnection *con = static_cast<SignalSlotConnection*>(connection(i));
        Q_ASSERT(con != 0);
        list.append(con->toUi());
    }
    result->setElementConnection(list);
    return result;
}

QWidget *SignalSlotEditor::widgetByName(QWidget *topLevel, const QString &name)
{
    Q_ASSERT(topLevel);
    if (topLevel->objectName() == name)
        return topLevel;

    return qFindChild<QWidget*>(topLevel, name);
}

void SignalSlotEditor::fromUi(DomConnections *connections, QWidget *parent)
{
    if (connections == 0)
        return;

    setBackground(parent);

    QList<DomConnection*> list = connections->elementConnection();
    foreach (DomConnection *dom_con, list) {
        QWidget *source = widgetByName(parent, dom_con->elementSender());
        if (source == 0) {
            qWarning("SignalSlotEditor::fromUi(): no source widget called \"%s\"",
                        dom_con->elementSender().toUtf8().constData());
            continue;
        }
        QWidget *destination = widgetByName(parent, dom_con->elementReceiver());
        if (destination == 0) {
            qWarning("SignalSlotEditor::fromUi(): no destination widget called \"%s\"",
                        dom_con->elementReceiver().toUtf8().constData());
            continue;
        }

        QPoint sp = QPoint(-1, -1), tp = QPoint(-1, -1);
        DomConnectionHints *dom_hints = dom_con->elementHints();
        if (dom_hints != 0) {
            QList<DomConnectionHint*> list = dom_hints->elementHint();
            foreach (DomConnectionHint *hint, list) {
                QString attr_type = hint->attributeType();
                QPoint p = QPoint(hint->elementX(), hint->elementY());
                if (attr_type == QLatin1String("sourcelabel"))
                    sp = p;
                else if (attr_type == QLatin1String("destinationlabel"))
                    tp = p;
            }
        }

        if (sp == QPoint(-1, -1))
            sp = widgetRect(source).center();
        if (tp == QPoint(-1, -1))
            tp = widgetRect(destination).center();

        SignalSlotConnection *con = new SignalSlotConnection(this);
        con->setEndPoint(EndPoint::Source, source, sp);
        con->setEndPoint(EndPoint::Target, destination, tp);
        con->setSignal(dom_con->elementSignal());
        con->setSlot(dom_con->elementSlot());
        addConnection(con);
    }
}

static bool skipWidget(QWidget *w)
{
    QString name = QLatin1String(w->metaObject()->className());
    if (name == QLatin1String("QDesignerWidget"))
        return true;
    if (name == QLatin1String("QLayoutWidget"))
        return true;
    if (name == QLatin1String("qdesigner::components::formeditor::FormWindow"))
        return true;
    if (name == QLatin1String("Spacer"))
        return true;
    return false;
}

QWidget *SignalSlotEditor::widgetAt(const QPoint &pos) const
{
    QWidget *widget = ConnectionEdit::widgetAt(pos);

    if (widget == m_form_window->mainContainer())
        return widget;

    for (; widget != 0; widget = widget->parentWidget()) {
        QDesignerMetaDataBaseItemInterface *item = m_form_window->core()->metaDataBase()->item(widget);
        if (item == 0)
            continue;
        if (skipWidget(widget))
            continue;
        break;
    }

    return widget;
}

void SignalSlotEditor::setSignal(SignalSlotConnection *con, const QString &member)
{
    if (member == con->signal())
        return;

    m_form_window->beginCommand(tr("Change signal"));
    undoStack()->push(new SetMemberCommand(con, EndPoint::Source, member, this));
    if (!signalMatchesSlot(member, con->slot()))
        undoStack()->push(new SetMemberCommand(con, EndPoint::Target, QString(), this));
    m_form_window->endCommand();
}

void SignalSlotEditor::setSlot(SignalSlotConnection *con, const QString &member)
{
    if (member == con->slot())
        return;

    m_form_window->beginCommand(tr("Change slot"));
    undoStack()->push(new SetMemberCommand(con, EndPoint::Target, member, this));
    if (!signalMatchesSlot(con->signal(), member))
        undoStack()->push(new SetMemberCommand(con, EndPoint::Source, QString(), this));
    m_form_window->endCommand();
}

void SignalSlotEditor::setSource(Connection *_con, const QString &obj_name)
{
    SignalSlotConnection *con = static_cast<SignalSlotConnection*>(_con);

    if (con->sender() == obj_name)
        return;

    m_form_window->beginCommand(tr("Change sender"));
    ConnectionEdit::setSource(con, obj_name);

    QWidget *w = con->widget(EndPoint::Source);
    QStringList member_list = memberList(m_form_window, w, SignalMember);

    if (!member_list.contains(con->signal()))
        undoStack()->push(new SetMemberCommand(con, EndPoint::Source, QString(), this));

    m_form_window->endCommand();
}

void SignalSlotEditor::setTarget(Connection *_con, const QString &obj_name)
{
    SignalSlotConnection *con = static_cast<SignalSlotConnection*>(_con);

    if (con->receiver() == obj_name)
        return;

    m_form_window->beginCommand(tr("Change receiver"));
    ConnectionEdit::setTarget(con, obj_name);

    QWidget *w = con->widget(EndPoint::Target);
    QStringList member_list = memberList(m_form_window, w, SlotMember);

    if (!member_list.contains(con->slot()))
        undoStack()->push(new SetMemberCommand(con, EndPoint::Target, QString(), this));

    m_form_window->endCommand();
}

void SignalSlotEditor::addEmptyConnection()
{
    SignalSlotConnection *con = new SignalSlotConnection(this);
    undoStack()->push(new AddConnectionCommand(this, con));
}

} // namespace signalsloteditor
} // namespace components
} // namespace qdesigner

#include "signalsloteditor.moc"

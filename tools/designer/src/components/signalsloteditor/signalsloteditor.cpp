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

#include "signalsloteditor.h"
#include "default_membersheet.h"

#include <abstractformeditor.h>
#include <abstractmetadatabase.h>
#include <abstractwidgetdatabase.h>
#include <qextensionmanager.h>
#include <ui4.h>

#include <QDialog>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <qdebug.h>

/*******************************************************************************
** SignalSlotDialog
*/

class SignalSlotDialog : public QDialog
{
    Q_OBJECT
public:
    SignalSlotDialog(AbstractFormEditor *core, QWidget *sender, QWidget *receiver, QWidget *parent = 0);

    QString signal() const;
    QString slot() const;

private slots:
    void signalClicked(QListWidgetItem *item, Qt::MouseButton button,
                        Qt::KeyboardModifiers modifiers);
    void slotClicked(QListWidgetItem *item, Qt::MouseButton button,
                        Qt::KeyboardModifiers modifiers);

private:
    void populateSlotList(const QString &signal = QString());

    QListWidget *m_signal_list, *m_slot_list;
    QPushButton *m_ok_button;
    QWidget *m_source, *m_destination;
    AbstractFormEditor *m_core;
};

static QString realObjectName(AbstractFormEditor *core, QWidget *widget)
{
    QString object_name = widget->objectName();
    AbstractMetaDataBase *mdb = core->metaDataBase();
    AbstractMetaDataBaseItem *item = mdb->item(widget);
    if (item != 0)
        object_name = item->name();
    return object_name;
}

static QString realClassName(AbstractFormEditor *core, QWidget *widget)
{
    QString class_name = widget->metaObject()->className();
    AbstractWidgetDataBase *wdb = core->widgetDataBase();
    int idx = wdb->indexOfObject(widget);
    if (idx != -1)
        class_name = wdb->item(idx)->name();
    return class_name;
}

static QString widgetLabel(AbstractFormEditor *core, QWidget *widget)
{
    return QString("%1 (%2)")
            .arg(realObjectName(core, widget))
            .arg(realClassName(core, widget));
}

static bool signalMatchesSlot(const QString &signal, const QString &slot)
{
    int signal_idx = signal.indexOf(QLatin1Char('('));
    int slot_idx = slot.indexOf(QLatin1Char('('));
    Q_ASSERT(signal_idx != -1);
    Q_ASSERT(slot_idx != -1);

    ++signal_idx; ++slot_idx;

    if (slot.at(slot_idx) == QLatin1Char(')'))
        return true;

    while (signal_idx < signal.size() && slot_idx < slot.size()) {
        QChar signal_c = signal.at(signal_idx);
        QChar slot_c = slot.at(slot_idx);

        if (signal_c == QLatin1Char(',') && slot_c == QLatin1Char(')'))
            return true;

        if (signal_c == QLatin1Char(')') && slot_c == QLatin1Char(')'))
            return true;

        if (signal_c != slot_c)
            return false;

        ++signal_idx; ++slot_idx;
    }

    return false;
}

void SignalSlotDialog::populateSlotList(const QString &signal)
{
    m_slot_list->clear();

    QStringList signatures;

    if (IMemberSheet *members = qt_extension<IMemberSheet*>(m_core->extensionManager(), m_destination)) {
        for (int i=0; i<members->count(); ++i) {
            if (!members->isVisible(i))
                continue;

            if (members->isSlot(i)) {
                if (!signal.isEmpty() && !signalMatchesSlot(signal, members->signature(i)))
                    continue;

                signatures.append(members->signature(i));
            }
        }
    }

    signatures.sort();

    foreach (QString sig, signatures) {
        QListWidgetItem *item = new QListWidgetItem(m_slot_list);
        item->setText(sig);
    }
}

SignalSlotDialog::SignalSlotDialog(AbstractFormEditor *core, QWidget *source, QWidget *destination,
                                    QWidget *parent)
    : QDialog(parent)
{
    m_source = source;
    m_destination = destination;
    m_core = core;

    m_signal_list = new QListWidget(this);
    connect(m_signal_list,
                SIGNAL(clicked(QListWidgetItem*, Qt::MouseButton, Qt::KeyboardModifiers)),
            this,
                SLOT(signalClicked(QListWidgetItem*, Qt::MouseButton, Qt::KeyboardModifiers)));
    m_slot_list = new QListWidget(this);
    connect(m_slot_list,
                SIGNAL(clicked(QListWidgetItem*, Qt::MouseButton, Qt::KeyboardModifiers)),
            this,
                SLOT(slotClicked(QListWidgetItem*, Qt::MouseButton, Qt::KeyboardModifiers)));
    m_slot_list->setEnabled(false);

    QPushButton *cancel_button = new QPushButton(tr("Cancel"), this);
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    m_ok_button = new QPushButton(tr("OK"), this);
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    m_ok_button->setEnabled(false);

    QLabel *source_label = new QLabel(this);
    source_label->setText(widgetLabel(core, source));
    QLabel *destination_label = new QLabel(this);
    destination_label->setText(widgetLabel(core, destination));

    QVBoxLayout *l1 = new QVBoxLayout(this);

    QHBoxLayout *l2 = new QHBoxLayout(l1);
    QVBoxLayout *l3 = new QVBoxLayout(l2);
    l3->addWidget(source_label);
    l3->addWidget(m_signal_list);
    QVBoxLayout *l4 = new QVBoxLayout(l2);
    l4->addWidget(destination_label);
    l4->addWidget(m_slot_list);

    QHBoxLayout *l5 = new QHBoxLayout(l1);
    l5->addStretch();
    l5->addWidget(cancel_button);
    l5->addWidget(m_ok_button);

    QStringList signatures;

    if (IMemberSheet *members = qt_extension<IMemberSheet*>(core->extensionManager(), source)) {
        for (int i=0; i<members->count(); ++i) {
            if (!members->isVisible(i))
                continue;

            if (members->isSignal(i)) {
                signatures.append(members->signature(i));
            }
        }
    }

    signatures.sort();

    foreach (QString sig, signatures) {
        QListWidgetItem *item = new QListWidgetItem(m_signal_list);
        item->setText(sig);
    }

    populateSlotList();
    setWindowTitle(tr("Configure Connection"));
}

void SignalSlotDialog::signalClicked(QListWidgetItem *item, Qt::MouseButton button,
                                        Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(button);
    Q_UNUSED(modifiers);

    if (item == 0) {
        populateSlotList();
        m_slot_list->setEnabled(false);
        m_ok_button->setEnabled(false);
    } else {
        populateSlotList(item->text());
        m_slot_list->setEnabled(true);
        m_ok_button->setEnabled(false);
    }
}

void SignalSlotDialog::slotClicked(QListWidgetItem *item, Qt::MouseButton button,
                                    Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(item);
    Q_UNUSED(button);
    Q_UNUSED(modifiers);

    m_ok_button->setEnabled(true);
}

QString SignalSlotDialog::signal() const
{
    QList<QListWidgetItem*> item_list = m_signal_list->selectedItems();
    if (item_list.size() != 1)
        return QString();
    return item_list.at(0)->text();
}

QString SignalSlotDialog::slot() const
{
    QList<QListWidgetItem*> item_list = m_slot_list->selectedItems();
    if (item_list.size() != 1)
        return QString();
    return item_list.at(0)->text();
}

/*******************************************************************************
** SignalSlotConnection
*/

SignalSlotConnection::SignalSlotConnection(ConnectionEdit *edit)
    : Connection(edit)
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
    Connection::HintList hint_list = this->hints();
    QList<DomConnectionHint*> list;
    foreach (ConnectionHint h, hint_list) {
        DomConnectionHint *hint = new DomConnectionHint;
        hint->setElementX(h.pos.x());
        hint->setElementY(h.pos.y());
        switch (h.type) {
            case ConnectionHint::EndPoint:
                hint->setAttributeType("endpoint");
                break;
            case ConnectionHint::SourceLabel:
                hint->setAttributeType("sourcelabel");
                break;
            case ConnectionHint::DestinationLabel:
                hint->setAttributeType("destinationlabel");
                break;
        }
        list.append(hint);
    }

    hints->setElementHint(list);
    result->setElementHints(hints);

    return result;
}

void SignalSlotConnection::setSignal(const QString &signal)
{
    m_signal = signal;
    setSourceLabel(DisplayRole, m_signal);
}

void SignalSlotConnection::setSlot(const QString &slot)
{
    m_slot = slot;
    setDestinationLabel(DisplayRole, m_slot);
}

QString SignalSlotConnection::sender() const
{
    SignalSlotEditor *edit = qt_cast<SignalSlotEditor*>(this->edit());
    Q_ASSERT(edit != 0);

    return realObjectName(edit->formWindow()->core(), source());
}

QString SignalSlotConnection::receiver() const
{
    SignalSlotEditor *edit = qt_cast<SignalSlotEditor*>(this->edit());
    Q_ASSERT(edit != 0);

    return realObjectName(edit->formWindow()->core(), destination());
}

/*******************************************************************************
** SignalSlotEditor
*/

SignalSlotEditor::SignalSlotEditor(AbstractFormWindow *form_window, QWidget *parent)
    : ConnectionEdit(parent)
{
    m_form_window = form_window;
}

Connection *SignalSlotEditor::createConnection(QWidget *source, QWidget *destination)
{
    SignalSlotConnection *con = 0;

    Q_ASSERT(source != 0);
    Q_ASSERT(destination != 0);

    SignalSlotDialog *dialog = new SignalSlotDialog(m_form_window->core(), source, destination);

    if (dialog->exec() == QDialog::Accepted) {
        con = new SignalSlotConnection(this);
        con->setSource(source);
        con->setDestination(destination);
        con->setSignal(dialog->signal());
        con->setSlot(dialog->slot());
    }

    delete dialog;

    return con;
}

void SignalSlotEditor::registerExtensions(AbstractFormEditor *core)
{
    QDesignerMemberSheetFactory *factory
        = new QDesignerMemberSheetFactory(core->extensionManager());
    core->extensionManager()->registerExtensions(factory, Q_TYPEID(IMemberSheet));
}

DomConnections *SignalSlotEditor::toUi() const
{
    DomConnections *result = new DomConnections;
    QList<DomConnection*> list;
    for (int i = 0; i < connectionCount(); ++i) {
        SignalSlotConnection *con = qt_cast<SignalSlotConnection*>(connection(i));
        Q_ASSERT(con != 0);
        list.append(con->toUi());
    }
    result->setElementConnection(list);
    return result;
}

void SignalSlotEditor::fromUi(DomConnections *connections, QWidget *parent)
{
    QList<DomConnection*> list = connections->elementConnection();
    foreach (DomConnection *dom_con, list) {
        QWidget *source = qFindChild<QWidget*>(parent, dom_con->elementSender());
        if (source == 0) {
            qWarning("SignalSlotEditor::fromUi(): no source widget called \"%s\"",
                        dom_con->elementSender().latin1());
            continue;
        }
        QWidget *destination
            = qFindChild<QWidget*>(parent, dom_con->elementReceiver());
        if (destination == 0) {
            qWarning("SignalSlotEditor::fromUi(): no destination widget called \"%s\"",
                        dom_con->elementReceiver().latin1());
            continue;
        }

        Connection::HintList hint_list;
        DomConnectionHints *dom_hints = dom_con->elementHints();
        if (dom_hints != 0) {
            QList<DomConnectionHint*> list = dom_hints->elementHint();
            foreach (DomConnectionHint *hint, list) {
                QString attr_type = hint->attributeType();
                ConnectionHint::Type hint_type;
                if (attr_type == "sourcelabel")
                    hint_type = ConnectionHint::SourceLabel;
                else if (attr_type == "destinationlabel")
                    hint_type = ConnectionHint::DestinationLabel;
                else
                    hint_type = ConnectionHint::EndPoint;

                hint_list.append(ConnectionHint(hint_type, QPoint(hint->elementX(), hint->elementY())));
            }
        }

        SignalSlotConnection *con = new SignalSlotConnection(this);
        con->setSource(source);
        con->setDestination(destination);
        initConnection(con, hint_list);
        con->setSignal(dom_con->elementSignal());
        con->setSlot(dom_con->elementSlot());
    }
}

static bool skipWidget(QWidget *w)
{
    QString name = w->metaObject()->className();
    if (name == "QDesignerWidget")
        return true;
    if (name == "QLayoutWidget")
        return true;
    if (name == "FormWindow")
        return true;
    if (name == "Spacer")
        return true;
    return false;
}

QWidget *SignalSlotEditor::widgetAt(const QPoint &pos) const
{
    QWidget *widget = ConnectionEdit::widgetAt(pos);
    for (; widget != 0; widget = widget->parentWidget()) {
        AbstractMetaDataBaseItem *item = m_form_window->core()->metaDataBase()->item(widget);
        if (item == 0)
            continue;
        if (skipWidget(widget))
            continue;
        return widget;
    }
    return widget;
}

#include "signalsloteditor.moc"

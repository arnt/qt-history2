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

#include "qdesigner_propertycommand_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QSize>
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtGui/QAction>

namespace  {
QSize checkSize(const QSize &size)
{
    return size.boundedTo(QSize(0xFFFFFF, 0xFFFFFF));
}

QSize diffSize(QDesignerFormWindowInterface *fw)
{
    const QWidget *container = fw->core()->integration()->containerWindow(fw);
    if (!container)
        return QSize();

    const QSize diff = container->size() - fw->size(); // decoration offset of container window
    return diff;
}

void checkSizes(QDesignerFormWindowInterface *fw, const QSize &size, QSize *formSize, QSize *containerSize)
{
    const QWidget *container = fw->core()->integration()->containerWindow(fw);
    if (!container)
        return;

    const  QSize diff = diffSize(fw); // decoration offset of container window

    QSize newFormSize = checkSize(size).expandedTo(fw->mainContainer()->minimumSizeHint()); // don't try to resize to smaller size than minimumSizeHint
    QSize newContainerSize = newFormSize + diff;

    newContainerSize = newContainerSize.expandedTo(container->minimumSizeHint());
    newContainerSize = newContainerSize.expandedTo(container->minimumSize());

    newFormSize = newContainerSize - diff;

    if (formSize)
        *formSize = newFormSize;
    if (containerSize)
        *containerSize = newContainerSize;
}

QVariant setFormProperty(QDesignerFormWindowInterface *fw, QWidget *w, const QString &propertyName, const QVariant &value)
{
    QDesignerFormWindowCursorInterface *cursor = fw->cursor();
    if (w && cursor->isWidgetSelected(w)) {
        if (cursor->isWidgetSelected(fw->mainContainer())) {
            if (propertyName == QLatin1String("minimumSize")) {
                QWidget *container = fw->core()->integration()->containerWindow(fw);
                if (container) {
                    const QSize diff = diffSize(fw);
                    const QSize size = checkSize(value.toSize());
                    container->setMinimumSize((size + diff).expandedTo(QSize(16, 16)));
                    return size;
                }
            } else if (propertyName == QLatin1String("maximumSize")) {
                QWidget *container = fw->core()->integration()->containerWindow(fw);
                if (container) {
                    QSize fs, cs;
                    checkSizes(fw, value.toSize(), &fs, &cs);
                    container->setMaximumSize(cs);
                    fw->mainContainer()->setMaximumSize(fs);
                    return fs;
                }
            } else if (propertyName == QLatin1String("geometry")) {
                QWidget *container = fw->core()->integration()->containerWindow(fw);
                if (container) {
                    QRect r = value.toRect();
                    QSize fs, cs;
                    checkSizes(fw, r.size(), &fs, &cs);
                    container->resize(cs);
                    r.setSize(fs);
                    return r;
                }
            }
        }
    }
    return value;
}
}

namespace qdesigner_internal {
// ---- SetPropertyCommand ----
SetPropertyCommand::SetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
      m_index(-1),
      m_propertySheet(0),
      m_changed(false)
{
}

QObject *SetPropertyCommand::object() const
{
    return m_object;
}

QWidget *SetPropertyCommand::widget() const
{
    return qobject_cast<QWidget*>(m_object);
}

QWidget *SetPropertyCommand::parentWidget() const
{
    if (QWidget *w = widget()) {
        return w->parentWidget();
    }

    return 0;
}

void SetPropertyCommand::init(QObject *object, const QString &propertyName, const QVariant &newValue)
{
    Q_ASSERT(object);

    m_object = object;
    m_parentWidget = parentWidget();
    m_propertyName = propertyName;
    m_newValue = newValue;

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(m_propertySheet);

    m_index = m_propertySheet->indexOf(m_propertyName);
    Q_ASSERT(m_index != -1);

    m_changed = m_propertySheet->isChanged(m_index);
    m_oldValue = m_propertySheet->property(m_index);

    setText(QApplication::translate("Command", "changed '%1' of '%2'").arg(m_propertyName).arg(object->objectName()));
}

void SetPropertyCommand::redo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    const QVariant value = setFormProperty(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, m_newValue);

    m_propertySheet->setProperty(m_index, value);
    m_changed = m_propertySheet->isChanged(m_index);
    m_propertySheet->setChanged(m_index, true);

    if (m_propertyName == QLatin1String("geometry") && widget()) {
        checkParent(widget(), parentWidget());
    } else if (m_propertyName == QLatin1String("objectName")) {
        updateBuddies(m_oldValue.toString(), value.toString());
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == object())
            propertyEditor->setPropertyValue( m_propertyName, value, true);
        else
            propertyEditor->setObject(propertyEditor->object()); // this is needed when f.ex. undo
                                                                 // changes parent's palette, but
                                                                 // the child is the active widget.
    }

    QAction *act = qobject_cast<QAction *>(m_object);
    if (m_propertyName == QLatin1String("objectName") ||
                m_propertyName == QLatin1String("icon") && act ||
                m_propertyName == QLatin1String("currentTabName")) {
        if (QDesignerObjectInspectorInterface *oi = formWindow()->core()->objectInspector())
            oi->setFormWindow(formWindow());
    }

    if (m_propertyName == QLatin1String("objectName") && act) {
        // emit act->changed(); cannot emit, signal is protected
        act->setData(QVariant(true)); // it triggers signal "changed" in QAction
        act->setData(QVariant(false));
    }
}

void SetPropertyCommand::undo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    QVariant value = setFormProperty(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, m_oldValue);

    m_propertySheet->setProperty(m_index, value);
    m_propertySheet->setChanged(m_index, m_changed);

    if (m_propertyName == QLatin1String("geometry") && widget()) {
        checkParent(widget(), parentWidget());
    } else if (m_propertyName == QLatin1String("objectName")) {
        updateBuddies(m_newValue.toString(), value.toString());
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == widget())
            propertyEditor->setPropertyValue( m_propertyName, value, m_changed);
        else
            propertyEditor->setObject(propertyEditor->object()); // this is needed when f.ex. undo
                                                                 // changes parent's palette, but
                                                                 // the child is the active widget.
    }

    QAction *act = qobject_cast<QAction *>(m_object);
    if (m_propertyName == QLatin1String("objectName") ||
                m_propertyName == QLatin1String("icon") && act ||
                m_propertyName == QLatin1String("currentTabName")) {
        if (QDesignerObjectInspectorInterface *oi = formWindow()->core()->objectInspector())
            oi->setFormWindow(formWindow());
    }

    if (m_propertyName == QLatin1String("objectName") && act) {
        // emit act->changed(); cannot emit, signal is protected
        act->setData(QVariant(true)); // it triggers signal "changed" in QAction
        act->setData(QVariant(false));
    }
}


// ---- ResetPropertyCommand ----
ResetPropertyCommand::ResetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
      m_index(-1),
      m_propertySheet(0),
      m_changed(false)
{
}

QObject *ResetPropertyCommand::object() const
{
    return m_object;
}

QObject *ResetPropertyCommand::parentObject() const
{
    return m_parentObject;
}

void ResetPropertyCommand::init(QObject *object, const QString &propertyName)
{
    Q_ASSERT(object);

    m_object = object;
    m_parentObject = object->parent();
    m_propertyName = propertyName;

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(m_propertySheet);

    m_index = m_propertySheet->indexOf(m_propertyName);
    Q_ASSERT(m_index != -1);

    m_changed = m_propertySheet->isChanged(m_index);
    m_oldValue = m_propertySheet->property(m_index);

    setText(QApplication::translate("Command", "reset '%1' of '%2'").arg(m_propertyName).arg(m_object->objectName()));
}

void ResetPropertyCommand::redo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    QVariant new_value;

    if (m_propertySheet->reset(m_index)) {
        new_value = m_propertySheet->property(m_index);
    } else {
        int item_idx =  formWindow()->core()->widgetDataBase()->indexOfObject(m_object);
        if (item_idx == -1) {
            new_value = m_oldValue; // We simply don't know the value in this case
        } else {
            QDesignerWidgetDataBaseItemInterface *item
                = formWindow()->core()->widgetDataBase()->item(item_idx);
            const QList<QVariant> default_prop_values = item->defaultPropertyValues();
            if (m_index < default_prop_values.size())
                new_value = default_prop_values.at(m_index);
            else
                new_value = m_oldValue; // Again, we just don't know
        }

        m_propertySheet->setProperty(m_index, new_value);
    }

    m_propertySheet->setChanged(m_index, false);

    setFormProperty(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, new_value);

    QWidget *widget = qobject_cast<QWidget *>(m_object);
    QWidget *parentWidget = qobject_cast<QWidget *>(m_parentObject);
    if (m_propertyName == QLatin1String("geometry") && widget) {
        checkParent(widget, parentWidget);
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == object())
            propertyEditor->setPropertyValue(m_propertyName, new_value, false);
    }
}

void ResetPropertyCommand::undo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    const QVariant value = setFormProperty(formWindow(), qobject_cast<QWidget *>(m_object), m_propertyName, m_oldValue);

    m_propertySheet->setProperty(m_index, value);
    m_propertySheet->setChanged(m_index, m_changed);

    QWidget *widget = qobject_cast<QWidget *>(m_object);
    QWidget *parentWidget = qobject_cast<QWidget *>(m_parentObject);
    if (m_propertyName == QLatin1String("geometry") && widget) {
        checkParent(widget, parentWidget);
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == object())
            propertyEditor->setPropertyValue(m_propertyName, value, m_changed);
    }

}

} // namespace qdesigner_internal

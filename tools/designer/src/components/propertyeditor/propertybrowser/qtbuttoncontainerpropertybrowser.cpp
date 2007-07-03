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

#include "qtbuttoncontainerpropertybrowser.h"
#include <QSet>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QMap>
#include "qtbuttoncontainer.h"

//////////////////////////////////

class QtButtonContainerPropertyBrowserPrivate
{
    QtButtonContainerPropertyBrowser *q_ptr;
    Q_DECLARE_PUBLIC(QtButtonContainerPropertyBrowser)
public:

    void init(QWidget *parent);

    void propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex);
    void propertyRemoved(QtBrowserItem *index);
    void propertyChanged(QtBrowserItem *index);
    QWidget *createEditor(QtProperty *property, QWidget *parent) const
        { return q_ptr->createEditor(property, parent); }

    void slotEditorDestroyed();
    void slotUpdate();

    struct WidgetItem
    {
        WidgetItem() : widget(0), label(0), widgetLabel(0),
                buttonContainer(0), layout(0), line(0), parent(0) { }
        QWidget *widget; // can be null
        QLabel *label;
        QLabel *widgetLabel;
        QtButtonContainer *buttonContainer;
        QGridLayout *layout;
        QFrame *line;
        WidgetItem *parent;
        QList<WidgetItem *> children;
    };
private:
    void updateLater();
    void updateItem(WidgetItem *item);
    void insertRow(QGridLayout *layout, int row);
    void removeRow(QGridLayout *layout, int row);

    bool hasHeader(WidgetItem *item) const;

    QMap<QtBrowserItem *, WidgetItem *> m_indexToItem;
    QMap<WidgetItem *, QtBrowserItem *> m_itemToIndex;
    QMap<QWidget *, WidgetItem *> m_widgetToItem;
    QGridLayout *m_mainLayout;
    QList<WidgetItem *> m_children;
    QList<WidgetItem *> m_recreateQueue;
};

void QtButtonContainerPropertyBrowserPrivate::init(QWidget *parent)
{
    m_mainLayout = new QGridLayout();
    m_mainLayout->setMargin(0);
    parent->setLayout(m_mainLayout);
    QLayoutItem *item = new QSpacerItem(0, 0,
                QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_mainLayout->addItem(item, 0, 0);
}

void QtButtonContainerPropertyBrowserPrivate::slotEditorDestroyed()
{
    QWidget *editor = qobject_cast<QWidget *>(q_ptr->sender());
    if (!editor)
        return;
    if (!m_widgetToItem.contains(editor))
        return;
    m_widgetToItem[editor]->widget = 0;
    m_widgetToItem.remove(editor);
}

void QtButtonContainerPropertyBrowserPrivate::slotUpdate()
{
    QListIterator<WidgetItem *> itItem(m_recreateQueue);
    while (itItem.hasNext()) {
        WidgetItem *item = itItem.next();

        WidgetItem *par = item->parent;
        QWidget *w = 0;
        QGridLayout *l = 0;
        int oldRow = -1;
        if (!par) {
            w = q_ptr;
            l = m_mainLayout;
            oldRow = m_children.indexOf(item);
        } else {
            w = par->buttonContainer->container();
            l = par->layout;
            oldRow = par->children.indexOf(item);
            if (hasHeader(par))
                oldRow += 2;
        }

        if (item->widget) {
            item->widget->setParent(w);
        } else if (item->widgetLabel) {
            item->widgetLabel->setParent(w);
        } else {
            item->widgetLabel = new QLabel(w);
        }
        int span = 1;
        if (item->widget)
            l->addWidget(item->widget, oldRow, 1, 1, 1);
        else if (item->widgetLabel)
            l->addWidget(item->widgetLabel, oldRow, 1, 1, 1);
        else
            span = 2;
        item->label = new QLabel(w);
        item->label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        l->addWidget(item->label, oldRow, 0, 1, span);

        updateItem(item);
    }
    m_recreateQueue.clear();
}

void QtButtonContainerPropertyBrowserPrivate::updateLater()
{
    QTimer::singleShot(0, q_ptr, SLOT(slotUpdate()));
}

void QtButtonContainerPropertyBrowserPrivate::propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex)
{
    WidgetItem *afterItem = m_indexToItem.value(afterIndex);
    WidgetItem *parentItem = m_indexToItem.value(index->parent());

    WidgetItem *newItem = new WidgetItem();
    newItem->parent = parentItem;

    QGridLayout *layout = 0;
    QWidget *parentWidget = 0;
    int row = -1;
    if (!afterItem) {
        row = 0;
        if (parentItem)
            parentItem->children.insert(0, newItem);
        else
            m_children.insert(0, newItem);
    } else {
        if (parentItem) {
            row = parentItem->children.indexOf(afterItem) + 1;
            parentItem->children.insert(row, newItem);
        } else {
            row = m_children.indexOf(afterItem) + 1;
            m_children.insert(row, newItem);
        }
    }
    if (parentItem && hasHeader(parentItem))
        row += 2;

    if (!parentItem) {
        layout = m_mainLayout;
        parentWidget = q_ptr;;
    } else {
        if (!parentItem->buttonContainer) {
            m_recreateQueue.removeAll(parentItem);
            WidgetItem *par = parentItem->parent;
            QWidget *w = 0;
            QGridLayout *l = 0;
            int oldRow = -1;
            if (!par) {
                w = q_ptr;
                l = m_mainLayout;
                oldRow = m_children.indexOf(parentItem);
            } else {
                w = par->buttonContainer->container();
                l = par->layout;
                oldRow = par->children.indexOf(parentItem);
                if (hasHeader(par))
                    oldRow += 2;
            }
            parentItem->buttonContainer = new QtButtonContainer(w);
            QFrame *container = new QFrame();
            container->setFrameShape(QFrame::Panel);
            container->setFrameShadow(QFrame::Raised);
            parentItem->layout = new QGridLayout();
            parentItem->buttonContainer->setContainer(container);
            container->setLayout(parentItem->layout);
            if (parentItem->label) {
                l->removeWidget(parentItem->label);
                delete parentItem->label;
                parentItem->label = 0;
            }
            if (parentItem->widget) {
                l->removeWidget(parentItem->widget);
                parentItem->widget->setParent(parentItem->buttonContainer->container());
                parentItem->layout->addWidget(parentItem->widget, 0, 0, 1, 2);
                parentItem->line = new QFrame(parentItem->buttonContainer->container());
            } else if (parentItem->widgetLabel) {
                l->removeWidget(parentItem->widgetLabel);
                delete parentItem->widgetLabel;
                parentItem->widgetLabel = 0;
            }
            if (parentItem->line) {
                parentItem->line->setFrameShape(QFrame::HLine);
                parentItem->line->setFrameShadow(QFrame::Sunken);
                parentItem->layout->addWidget(parentItem->line, 1, 0, 1, 2);
            }
            l->addWidget(parentItem->buttonContainer, oldRow, 0, 1, 2);
            updateItem(parentItem);
        }
        layout = parentItem->layout;
        parentWidget = parentItem->buttonContainer->container();
    }

    newItem->label = new QLabel(parentWidget);
    newItem->label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    newItem->widget = createEditor(index->property(), parentWidget);
    if (!newItem->widget) {
        newItem->widgetLabel = new QLabel(parentWidget);
    } else {
        QObject::connect(newItem->widget, SIGNAL(destroyed()), q_ptr, SLOT(slotEditorDestroyed()));
        m_widgetToItem[newItem->widget] = newItem;
    }

    insertRow(layout, row);
    int span = 1;
    if (newItem->widget)
        layout->addWidget(newItem->widget, row, 1);
    else if (newItem->widgetLabel)
        layout->addWidget(newItem->widgetLabel, row, 1);
    else
        span = 2;
    layout->addWidget(newItem->label, row, 0, 1, span);

    m_itemToIndex[newItem] = index;
    m_indexToItem[index] = newItem;

    updateItem(newItem);
}

void QtButtonContainerPropertyBrowserPrivate::propertyRemoved(QtBrowserItem *index)
{
    WidgetItem *item = m_indexToItem.value(index);

    m_indexToItem.remove(index);
    m_itemToIndex.remove(item);

    WidgetItem *parentItem = item->parent;

    int row = -1;

    if (parentItem) {
        row = parentItem->children.indexOf(item);
        parentItem->children.removeAt(row);
        if (hasHeader(parentItem))
            row += 2;
    } else {
        row = m_children.indexOf(item);
        m_children.removeAt(row);
    }

    if (item->widget)
        delete item->widget;
    if (item->label)
        delete item->label;
    if (item->widgetLabel)
        delete item->widgetLabel;
    if (item->buttonContainer)
        delete item->buttonContainer;

    if (!parentItem) {
        removeRow(m_mainLayout, row);
    } else if (parentItem->children.count() != 0) {
        removeRow(parentItem->layout, row);
    } else {
        WidgetItem *par = parentItem->parent;
        QWidget *w = 0;
        QGridLayout *l = 0;
        int oldRow = -1;
        if (!par) {
            w = q_ptr;
            l = m_mainLayout;
            oldRow = m_children.indexOf(parentItem);
        } else {
            w = par->buttonContainer->container();
            l = par->layout;
            oldRow = par->children.indexOf(parentItem);
            if (hasHeader(par))
                oldRow += 2;
        }

        if (parentItem->widget) {
            parentItem->widget->hide();
            parentItem->widget->setParent(0);
        } else if (parentItem->widgetLabel) {
            parentItem->widgetLabel->hide();
            parentItem->widgetLabel->setParent(0);
        } else {
            //parentItem->widgetLabel = new QLabel(w);
        }
        l->removeWidget(parentItem->buttonContainer);
        delete parentItem->buttonContainer;
        parentItem->buttonContainer = 0;
        parentItem->line = 0;
        parentItem->layout = 0;
        if (!m_recreateQueue.contains(parentItem))
            m_recreateQueue.append(parentItem);
        updateLater();
    }
    m_recreateQueue.removeAll(item);

    delete item;
}

void QtButtonContainerPropertyBrowserPrivate::insertRow(QGridLayout *layout, int row)
{
    QMap<QLayoutItem *, QRect> itemToPos;
    int idx = 0;
    while (idx < layout->count()) {
        int r, c, rs, cs;
        layout->getItemPosition(idx, &r, &c, &rs, &cs);
        if (r >= row) {
            itemToPos[layout->takeAt(idx)] = QRect(r + 1, c, rs, cs);
        } else {
            idx++;
        }
    }

    QMap<QLayoutItem *, QRect>::ConstIterator it = itemToPos.constBegin();
    while (it != itemToPos.constEnd()) {
        QRect r = it.value();
        layout->addItem(it.key(), r.x(), r.y(), r.width(), r.height());

        it++;
    }
}

void QtButtonContainerPropertyBrowserPrivate::removeRow(QGridLayout *layout, int row)
{
    QMap<QLayoutItem *, QRect> itemToPos;
    int idx = 0;
    while (idx < layout->count()) {
        int r, c, rs, cs;
        layout->getItemPosition(idx, &r, &c, &rs, &cs);
        if (r > row) {
            itemToPos[layout->takeAt(idx)] = QRect(r - 1, c, rs, cs);
        } else {
            idx++;
        }
    }

    QMap<QLayoutItem *, QRect>::ConstIterator it = itemToPos.constBegin();
    while (it != itemToPos.constEnd()) {
        QRect r = it.value();
        layout->addItem(it.key(), r.x(), r.y(), r.width(), r.height());

        it++;
    }
}

bool QtButtonContainerPropertyBrowserPrivate::hasHeader(WidgetItem *item) const
{
    if (item->widget)
        return true;
    return false;
}

void QtButtonContainerPropertyBrowserPrivate::propertyChanged(QtBrowserItem *index)
{
    WidgetItem *item = m_indexToItem.value(index);

    updateItem(item);
}

void QtButtonContainerPropertyBrowserPrivate::updateItem(WidgetItem *item)
{
    QtProperty *property = m_itemToIndex[item]->property();
    if (item->buttonContainer) {
        QFont font = item->buttonContainer->font();
        font.setUnderline(property->isModified());
        item->buttonContainer->setFont(font);
        item->buttonContainer->setTitle(property->propertyName());
        item->buttonContainer->setToolTip(property->toolTip());
        item->buttonContainer->setStatusTip(property->statusTip());
        item->buttonContainer->setWhatsThis(property->whatsThis());
        item->buttonContainer->setEnabled(property->isEnabled());
    }
    if (item->label) {
        QFont font = item->label->font();
        font.setUnderline(property->isModified());
        item->label->setFont(font);
        item->label->setText(property->propertyName());
        item->label->setToolTip(property->toolTip());
        item->label->setStatusTip(property->statusTip());
        item->label->setWhatsThis(property->whatsThis());
        item->label->setEnabled(property->isEnabled());
    }
    if (item->widgetLabel) {
        QFont font = item->widgetLabel->font();
        font.setUnderline(false);
        item->widgetLabel->setFont(font);
        item->widgetLabel->setText(property->valueText());
        item->widgetLabel->setEnabled(property->isEnabled());
    }
    if (item->widget) {
        QFont font = item->widget->font();
        font.setUnderline(false);
        item->widget->setFont(font);
        item->widget->setEnabled(property->isEnabled());
        item->widget->setToolTip(property->valueText());
    }
    //item->setIcon(1, property->valueIcon());
}



/*!
    \class QtButtonContainerPropertyBrowser

    \brief The QtButtonContainerPropertyBrowser class provides a QtButtonContainer
    based property browser.

    A property browser is a widget that enables the user to edit a
    given set of properties. Each property is represented by a label
    specifying the property's name, and an editing widget (e.g. a line
    edit or a combobox) holding its value. A property can have zero or
    more subproperties.

    QtButtonContainerPropertyBrowser provides group boxes for all nested
    properties, i.e. subproperties are enclosed by a group box with
    the parent property's name as its title. For example:

    \image qtgroupboxpropertybrowser.png

    Use the QtAbstractPropertyBrowser API to add, insert and remove
    properties from an instance of the QtButtonContainerPropertyBrowser
    class. The properties themselves are created and managed by
    implementations of the QtAbstractPropertyManager class.

    \sa QtTreePropertyBrowser, QtAbstractPropertyBrowser
*/

/*!
    Creates a property browser with the given \a parent.
*/
QtButtonContainerPropertyBrowser::QtButtonContainerPropertyBrowser(QWidget *parent)
    : QtAbstractPropertyBrowser(parent)
{
    d_ptr = new QtButtonContainerPropertyBrowserPrivate;
    d_ptr->q_ptr = this;

    d_ptr->init(this);
}

/*!
    Destroys this property browser.

    Note that the properties that were inserted into this browser are
    \e not destroyed since they may still be used in other
    browsers. The properties are owned by the manager that created
    them.

    \sa QtProperty, QtAbstractPropertyManager
*/
QtButtonContainerPropertyBrowser::~QtButtonContainerPropertyBrowser()
{
    QMap<QtButtonContainerPropertyBrowserPrivate::WidgetItem *, QtBrowserItem *>::ConstIterator it =
                d_ptr->m_itemToIndex.constBegin();
    while (it != d_ptr->m_itemToIndex.constEnd()) {
        delete it.key();
        it++;
    }
    delete d_ptr;
}

/*!
    \reimp
*/
void QtButtonContainerPropertyBrowser::itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem)
{
    d_ptr->propertyInserted(item, afterItem);
}

/*!
    \reimp
*/
void QtButtonContainerPropertyBrowser::itemRemoved(QtBrowserItem *item)
{
    d_ptr->propertyRemoved(item);
}

/*!
    \reimp
*/
void QtButtonContainerPropertyBrowser::itemChanged(QtBrowserItem *item)
{
    d_ptr->propertyChanged(item);
}











#include "moc_qtbuttoncontainerpropertybrowser.cpp"


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

#include "objectinspector.h"

// sdk
#include <container.h>
#include <qextensionmanager.h>
#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractwidgetdatabase.h>

// Qt
#include <QApplication>
#include <QHeaderView>
#include <QStack>
#include <QScrollBar>
#include <QItemDelegate>
#include <QPair>
#include <QPainter>

#include <qdebug.h>

class ObjectInspectorDelegate: public QItemDelegate
{
public:
    ObjectInspectorDelegate(ObjectInspector *objectInspector)
        : QItemDelegate(objectInspector) {}
        
        
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &opt,
                       const QModelIndex &index) const
    {
        QStyleOptionViewItem option = opt;
    
        option.state &= ~(QStyle::Style_Selected | QStyle::Style_HasFocus);
    
        if (opt.state & QStyle::Style_Selected)
            painter->fillRect(option.rect, QColor(230, 230, 230));
    
        painter->drawLine(option.rect.x(), option.rect.bottom(),
                        option.rect.right(), option.rect.bottom());
    
        painter->drawLine(option.rect.right(), option.rect.y(),
                        option.rect.right(), option.rect.bottom());
    
        QItemDelegate::paint(painter, option, index);
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
    {
        QStyleOptionViewItem option = opt;
    
        option.state &= ~(QStyle::Style_Selected | QStyle::Style_HasFocus);
    
        return QItemDelegate::sizeHint(option, index) + QSize(4,4);
    }
};


ObjectInspector::ObjectInspector(AbstractFormEditor *core, QWidget *parent)
    : QTreeWidget(parent),
      m_core(core),
      m_ignoreUpdate(false)
{
    setItemDelegate(new ObjectInspectorDelegate(this));
    
    // ### move
    setAlternatingRowColors(true);
    setOddRowColor(QColor(250, 248, 235));
    setEvenRowColor(QColor(255, 255, 255));
    
    setColumnCount(2);
    headerItem()->setText(0, tr("Object"));
    headerItem()->setText(1, tr("Class"));

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    header()->setResizeMode(QHeaderView::Stretch, 1);
    
    connect(this, SIGNAL(doubleClicked(QTreeWidgetItem *, int, Qt::MouseButton, Qt::KeyboardModifiers)), 
        this, SLOT(slotSelectionChanged()));
        
    connect(this, SIGNAL(returnPressed(QTreeWidgetItem *, int)), 
        this, SLOT(slotSelectionChanged()));
}

ObjectInspector::~ObjectInspector()
{
}

void ObjectInspector::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = viewOptions();
    QStyleOptionViewItem option = opt;

    if (selectionModel()->isSelected(index))
        painter->fillRect(rect, QColor(230, 230, 230));

    painter->drawLine(rect.x(), rect.bottom(),
                      rect.right(), rect.bottom());

    if (model()->hasChildren(index)) {
        static const int size = 9;
        int left = rect.width() - (indentation() + size) / 2 ;
        int top = rect.y() + (rect.height() - size) / 2;
        painter->drawLine(left + 2, top + 4, left + 6, top + 4);
        if (!isOpen(index))
            painter->drawLine(left + 4, top + 2, left + 4, top + 6);
        QPen oldPen = painter->pen();
        painter->setPen(opt.palette.dark().color());
        painter->drawRect(left, top, size - 1, size - 1);
        painter->setPen(oldPen);
    }
}


AbstractFormEditor *ObjectInspector::core() const
{
    return m_core;
}

void ObjectInspector::setFormWindow(AbstractFormWindow *fw)
{
    if (m_ignoreUpdate)
        return;
    
    m_formWindow = fw;

    int xoffset = horizontalScrollBar()->value();
    int yoffset = verticalScrollBar()->value();

    clear();
    
    if (!fw)
        return;

    AbstractWidgetDataBase *db = fw->core()->widgetDataBase();

    viewport()->setUpdatesEnabled(false);
    
    QStack< QPair<QTreeWidgetItem*, QObject*> > workingList;
    QObject *rootObject = fw->mainContainer();
    workingList.append(qMakePair(new QTreeWidgetItem(this), rootObject));

    while (!workingList.isEmpty()) {
        QTreeWidgetItem *item = workingList.top().first;
        QObject *object = workingList.top().second;
        workingList.pop();

        QString objectName = object->objectName();
        if (objectName.isEmpty())
            objectName = tr("<noname>");
        
        item->setText(0, objectName);

        QString className;
        if (AbstractWidgetDataBaseItem *widgetItem = db->item(db->indexOfObject(object, true))) {
            className = widgetItem->name();
            item->setText(1, className);
            item->setIcon(0, widgetItem->icon());
        }
        
        QVariant ptr;
        qVariantSet(ptr, object, "QObject");
        item->setData(0, 1000, ptr);
            
        if (IContainer *c = qt_extension<IContainer*>(fw->core()->extensionManager(), object)) {
            for (int i=0; i<c->count(); ++i) {
                QObject *page = c->widget(i);

                QTreeWidgetItem *pageItem = new QTreeWidgetItem(item);
                workingList.append(qMakePair(pageItem, page));
            }
        } else {
            QList<QObject*> children = object->children();
            foreach (QObject *child, children) {
                if (!child->isWidgetType() || !fw->isManaged(static_cast<QWidget*>(child)))
                    continue;

                QTreeWidgetItem *childItem = new QTreeWidgetItem(item);
                workingList.append(qMakePair(childItem, child));
            }
        }

        openItem(item);
    }

    horizontalScrollBar()->setValue(xoffset);
    verticalScrollBar()->setValue(yoffset);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

void ObjectInspector::slotSelectionChanged()
{
    if (!m_formWindow)
        return;
        
    m_formWindow->clearSelection(false);
 
    QList<QTreeWidgetItem*> items = selectedItems();
    foreach (QTreeWidgetItem *item, items) {
        QObject *object = 0;
        Q_ASSERT(qVariantGet(item->data(0, 1000), object, "QObject"));
        if (QWidget *widget = qt_cast<QWidget*>(object))
            m_formWindow->selectWidget(widget);
    }
}

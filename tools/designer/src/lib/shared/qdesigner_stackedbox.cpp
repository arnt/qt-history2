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

#include "qdesigner_stackedbox_p.h"
#include "qdesigner_command_p.h"
#include "qdesigner_propertycommand_p.h"
#include "orderdialog_p.h"
#include "promotiontaskmenu_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QToolButton>
#include <QtGui/QAction>
#include <QtGui/qevent.h>
#include <QtGui/QMenu>
#include <QtGui/QStackedWidget>
#include <QtCore/QDebug>


namespace {
     QToolButton *createToolButton(QWidget *parent, Qt::ArrowType at, const QString &name) {
         QToolButton *rc =  new QToolButton();
         rc->setAttribute(Qt::WA_NoChildEventsForParent, true);
         rc->setParent(parent);
         rc->setObjectName(name);
         rc->setArrowType(at);
         rc->setAutoRaise(true);
         rc->setContextMenuPolicy(Qt::PreventContextMenu);
         rc->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
         rc->setFixedSize(QSize(15, 15));
         return rc;
     }
}

// ---------------  QStackedWidgetPreviewEventFilter
QStackedWidgetPreviewEventFilter::QStackedWidgetPreviewEventFilter(QStackedWidget *parent) :
    QObject(parent),
    m_stackedWidget(parent),
    m_prev(createToolButton(m_stackedWidget, Qt::LeftArrow,  QLatin1String("__qt__passive_prev"))),
    m_next(createToolButton(m_stackedWidget, Qt::RightArrow, QLatin1String("__qt__passive_next")))
{
    connect(m_prev, SIGNAL(clicked()), this, SLOT(prevPage()));
    connect(m_next, SIGNAL(clicked()), this, SLOT(nextPage()));

    updateButtons();
    m_stackedWidget->installEventFilter(this);
}

void QStackedWidgetPreviewEventFilter::install(QStackedWidget *stackedWidget)
{
    new QStackedWidgetPreviewEventFilter(stackedWidget);
}

void QStackedWidgetPreviewEventFilter::updateButtons()
{
    m_prev->move(m_stackedWidget->width() - 31, 1);
    m_prev->show();
    m_prev->raise();

    m_next->move(m_stackedWidget->width() - 16, 1);
    m_next->show();
    m_next->raise();
}

void QStackedWidgetPreviewEventFilter::prevPage()
{
    const int count = m_stackedWidget->count();
    if (count > 1) {
        int newIndex = m_stackedWidget->currentIndex() - 1;
        if (newIndex < 0)
            newIndex = count - 1;
        gotoPage(newIndex);
    }
}

void QStackedWidgetPreviewEventFilter::nextPage()
{
    const int count = m_stackedWidget->count();
    if (count > 1)
        gotoPage((m_stackedWidget->currentIndex() + 1) % count);
}

bool QStackedWidgetPreviewEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (!watched->isWidgetType() || watched != m_stackedWidget)
        return QObject::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::LayoutRequest:
        updateButtons();
        break;
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved:
    case QEvent::Resize:
    case QEvent::Show:
        updateButtons();
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void QStackedWidgetPreviewEventFilter::gotoPage(int page)
{
    m_stackedWidget->setCurrentIndex(page);
    updateButtons();
}

// ---------------  QStackedWidgetEventFilter
QStackedWidgetEventFilter::QStackedWidgetEventFilter(QStackedWidget *parent) :
    QStackedWidgetPreviewEventFilter(parent),
    m_actionPreviousPage(new QAction(tr("Previous Page"), this)),
    m_actionNextPage(new QAction(tr("Next Page"), this)),
    m_actionDeletePage(new QAction(tr("Delete"), this)),
    m_actionInsertPage(new QAction(tr("Before Current Page"), this)),
    m_actionInsertPageAfter(new QAction(tr("After Current Page"), this)),
    m_actionChangePageOrder(new QAction(tr("Change Page Order..."), this)),
    m_pagePromotionTaskMenu(new qdesigner_internal::PromotionTaskMenu(0, qdesigner_internal::PromotionTaskMenu::ModeSingleWidget, this))
{
    connect(m_actionPreviousPage, SIGNAL(triggered()), this, SLOT(prevPage()));
    connect(m_actionNextPage, SIGNAL(triggered()), this, SLOT(nextPage()));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));
    connect(m_actionInsertPageAfter, SIGNAL(triggered()), this, SLOT(addPageAfter()));
    connect(m_actionChangePageOrder, SIGNAL(triggered()), this, SLOT(changeOrder()));

    connect(stackedWidget(), SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
}

void QStackedWidgetEventFilter::install(QStackedWidget *stackedWidget)
{
    new QStackedWidgetEventFilter(stackedWidget);
}

QStackedWidgetEventFilter *QStackedWidgetEventFilter::eventFilterOf(const QStackedWidget *stackedWidget)
{
    QList<QStackedWidgetEventFilter*> filters = qFindChildren<QStackedWidgetEventFilter*>(stackedWidget);
    if (filters.empty())
        return 0;
    return filters.front();
}

QMenu *QStackedWidgetEventFilter::addStackedWidgetContextMenuActions(const QStackedWidget *stackedWidget, QMenu *popup)
{
    QStackedWidgetEventFilter *filter = eventFilterOf(stackedWidget);
    if (!filter)
        return 0;
    return filter->addContextMenuActions(popup);
}

void QStackedWidgetEventFilter::removeCurrentPage()
{
    if (stackedWidget()->currentIndex() == -1)
        return;

    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
        qdesigner_internal::DeleteStackedWidgetPageCommand *cmd = new qdesigner_internal::DeleteStackedWidgetPageCommand(fw);
        cmd->init(stackedWidget());
        fw->commandHistory()->push(cmd);
    }
}

void QStackedWidgetEventFilter::changeOrder()
{
    QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget());

    if (!fw)
        return;

    qdesigner_internal::OrderDialog dlg(fw, stackedWidget());

    QList<QWidget*> wList;
    for(int i=0; i<stackedWidget()->count(); ++i)
        wList.append(stackedWidget()->widget(i));

    dlg.setPageList(&wList);

    if (dlg.exec() == QDialog::Accepted)    {
        fw->beginCommand(tr("Change Page Order"));
        for(int i=0; i<wList.count(); ++i) {
            if (wList.at(i) == stackedWidget()->widget(i))
                continue;
            qdesigner_internal::MoveStackedWidgetCommand *cmd = new qdesigner_internal::MoveStackedWidgetCommand(fw);
            cmd->init(stackedWidget(), wList.at(i), i);
            fw->commandHistory()->push(cmd);
        }
        fw->endCommand();
    }
}

void QStackedWidgetEventFilter::addPage()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
        qdesigner_internal::AddStackedWidgetPageCommand *cmd = new qdesigner_internal::AddStackedWidgetPageCommand(fw);
        cmd->init(stackedWidget(), qdesigner_internal::AddStackedWidgetPageCommand::InsertBefore);
        fw->commandHistory()->push(cmd);
    }
}

void QStackedWidgetEventFilter::addPageAfter()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
        qdesigner_internal::AddStackedWidgetPageCommand *cmd = new qdesigner_internal::AddStackedWidgetPageCommand(fw);
        cmd->init(stackedWidget(), qdesigner_internal::AddStackedWidgetPageCommand::InsertAfter);
        fw->commandHistory()->push(cmd);
    }
}

void QStackedWidgetEventFilter::slotCurrentChanged(int index)
{
    if (stackedWidget()->widget(index)) {
        if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
            fw->clearSelection();
            fw->selectWidget(stackedWidget(), true);
        }
    }
}

void QStackedWidgetEventFilter::gotoPage(int page) {
    // Are we on a form or in a preview?
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
        qdesigner_internal::SetPropertyCommand *cmd = new  qdesigner_internal::SetPropertyCommand(fw);
        cmd->init(stackedWidget(), QLatin1String("currentIndex"), page);
        fw->commandHistory()->push(cmd);
        fw->emitSelectionChanged(); // Magically prevent an endless loop triggered by auto-repeat.
        updateButtons();
    } else {
        QStackedWidgetPreviewEventFilter::gotoPage(page);
    }
}

QMenu *QStackedWidgetEventFilter::addContextMenuActions(QMenu *popup)
{
    QMenu *pageMenu = 0;
    const int count = stackedWidget()->count();
    m_actionDeletePage->setEnabled(count > 1);
    if (count) {
        const QString pageSubMenuLabel = tr("Page %1 of %2").arg(stackedWidget()->currentIndex() + 1).arg(count);
        pageMenu = popup->addMenu(pageSubMenuLabel);
        pageMenu->addAction(m_actionDeletePage);
        // Set up promotion menu for current widget.
        if (QWidget *page =  stackedWidget()->currentWidget ()) {
            m_pagePromotionTaskMenu->setWidget(page);
            m_pagePromotionTaskMenu->addActions(QDesignerFormWindowInterface::findFormWindow(stackedWidget()),
                                                qdesigner_internal::PromotionTaskMenu::SuppressGlobalEdit,
                                                pageMenu);
        }
    }
    QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
    insertPageMenu->addAction(m_actionInsertPageAfter);
    insertPageMenu->addAction(m_actionInsertPage);
    popup->addAction(m_actionNextPage);
    popup->addAction(m_actionPreviousPage);
    if (count > 1) {
        popup->addAction(m_actionChangePageOrder);
    }
    popup->addSeparator();
    return pageMenu;
}

// --------  QStackedWidgetPropertySheet

static const char *pagePropertyName = "currentPageName";

QStackedWidgetPropertySheet::QStackedWidgetPropertySheet(QStackedWidget *object, QObject *parent) :
    QDesignerPropertySheet(object, parent),
    m_stackedWidget(object)
{
    createFakeProperty(QLatin1String(pagePropertyName), QString());
}

void QStackedWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
    if (propertyName(index) == QLatin1String(pagePropertyName)) {
        if (QWidget *w = m_stackedWidget->currentWidget())
            w->setObjectName(value.toString());
    } else {
        QDesignerPropertySheet::setProperty(index, value);
    }
}

QVariant QStackedWidgetPropertySheet::property(int index) const
{
    if (propertyName(index) == QLatin1String(pagePropertyName)) {
        if (const QWidget *w = m_stackedWidget->currentWidget())
            return w->objectName();
        return QString();
    }
    return QDesignerPropertySheet::property(index);
}

bool QStackedWidgetPropertySheet::reset(int index)
{
    if (propertyName(index) == QLatin1String(pagePropertyName)) {
        setProperty(index, QString());
        return true;
    }
    return QDesignerPropertySheet::reset(index);
}

bool QStackedWidgetPropertySheet::checkProperty(const QString &propertyName)
{
    return propertyName != QLatin1String(pagePropertyName);
}

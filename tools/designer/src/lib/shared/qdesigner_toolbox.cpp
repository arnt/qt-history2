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

#include "qdesigner_toolbox_p.h"
#include "qdesigner_command_p.h"
#include "orderdialog_p.h"
#include "promotiontaskmenu_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QMenu>

QDesignerToolBox::QDesignerToolBox(QWidget *parent) :
    QToolBox(parent),
    m_actionDeletePage(new QAction(tr("Delete Page"), this)),
    m_actionInsertPage(new QAction(tr("Before Current Page"), this)),
    m_actionInsertPageAfter(new QAction(tr("After Current Page"), this)),
    m_actionChangePageOrder(new QAction(tr("Change Page Order..."), this)),
    m_pagePromotionTaskMenu(new qdesigner_internal::PromotionTaskMenu(0, qdesigner_internal::PromotionTaskMenu::ModeSingleWidget, this))
{
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));
    connect(m_actionInsertPageAfter, SIGNAL(triggered()), this, SLOT(addPageAfter()));
    connect(m_actionChangePageOrder, SIGNAL(triggered()), this, SLOT(changeOrder()));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
}

QString QDesignerToolBox::currentItemText() const
{
    return itemText(currentIndex());
}

void QDesignerToolBox::setCurrentItemText(const QString &itemText)
{
    setItemText(currentIndex(), itemText);
}

QString QDesignerToolBox::currentItemName() const
{
    if (currentIndex() == -1)
        return QString();

    return widget(currentIndex())->objectName();
}

void QDesignerToolBox::setCurrentItemName(const QString &itemName)
{
    if (currentIndex() == -1)
        return;

    widget(currentIndex())->setObjectName(itemName);
}

QIcon QDesignerToolBox::currentItemIcon() const
{
    return itemIcon(currentIndex());
}

void QDesignerToolBox::setCurrentItemIcon(const QIcon &itemIcon)
{
    setItemIcon(currentIndex(), itemIcon);
}

QString QDesignerToolBox::currentItemToolTip() const
{
    return itemToolTip(currentIndex());
}

void QDesignerToolBox::setCurrentItemToolTip(const QString &itemToolTip)
{
    setItemToolTip(currentIndex(), itemToolTip);
}

void QDesignerToolBox::removeCurrentPage()
{
    if (currentIndex() == -1 || !widget(currentIndex()))
        return;

    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        qdesigner_internal::DeleteToolBoxPageCommand *cmd = new qdesigner_internal::DeleteToolBoxPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBox::addPage()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        qdesigner_internal::AddToolBoxPageCommand *cmd = new qdesigner_internal::AddToolBoxPageCommand(fw);
        cmd->init(this, qdesigner_internal::AddToolBoxPageCommand::InsertBefore);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBox::changeOrder()
{
    QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this);

    if (!fw)
        return;

    qdesigner_internal::OrderDialog dlg(fw, this);

    QList<QWidget*> wList;
    for(int i=0; i<count(); ++i) {
        wList.append(widget(i));
    }
    dlg.setPageList(&wList);

    if (dlg.exec() == QDialog::Accepted)   {
        fw->beginCommand(tr("Change Page Order"));
        for(int i=0; i<wList.count(); ++i) {
            if (wList.at(i) == widget(i))
                continue;
            qdesigner_internal::MoveToolBoxPageCommand *cmd = new qdesigner_internal::MoveToolBoxPageCommand(fw);
            cmd->init(this, wList.at(i), i);
            fw->commandHistory()->push(cmd);
        }
        fw->endCommand();
    }
}

void QDesignerToolBox::addPageAfter()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        qdesigner_internal::AddToolBoxPageCommand *cmd = new qdesigner_internal::AddToolBoxPageCommand(fw);
        cmd->init(this, qdesigner_internal::AddToolBoxPageCommand::InsertAfter);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBox::itemInserted(int index)
{
    if (count() > 1 && widget(index))
        widget(index)->setBackgroundRole(widget(index>0?0:1)->backgroundRole());
}


QPalette::ColorRole QDesignerToolBox::currentItemBackgroundRole() const
{
    return widget(0) ? widget(0)->backgroundRole() : QPalette::Window;
}

void QDesignerToolBox::setCurrentItemBackgroundRole(QPalette::ColorRole role)
{
    for (int i = 0; i < count(); ++i) {
        QWidget *w = widget(i);
        w->setBackgroundRole(role);
        w->update();
    }
}

void QDesignerToolBox::slotCurrentChanged(int index)
{
    if (widget(index)) {
        if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
            fw->clearSelection();
            fw->selectWidget(this, true);
        }
    }
}

QMenu *QDesignerToolBox::addContextMenuActions(QMenu *popup)
{
    QMenu *pageMenu = 0;
    if (count()) {
        const QString pageSubMenuLabel = tr("Page %1 of %2").arg(currentIndex() + 1).arg(count());
        pageMenu = popup->addMenu(pageSubMenuLabel);
        pageMenu->addAction(m_actionDeletePage);
        // Set up promotion menu for current widget.
        if (QWidget *page =  currentWidget ()) {
            m_pagePromotionTaskMenu->setWidget(page);
            m_pagePromotionTaskMenu->addActions(QDesignerFormWindowInterface::findFormWindow(this), 
                                                qdesigner_internal::PromotionTaskMenu::SuppressGlobalEdit, 
                                                pageMenu);
        }
    }
    QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
    insertPageMenu->addAction(m_actionInsertPageAfter);
    insertPageMenu->addAction(m_actionInsertPage);
    if (count() > 1) {
        popup->addAction(m_actionChangePageOrder);
    }
    popup->addSeparator();
    return pageMenu;
}

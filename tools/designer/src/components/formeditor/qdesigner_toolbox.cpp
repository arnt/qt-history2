
#include "qdesigner_toolbox.h"
#include "formwindow.h"
#include "command.h"

#include <qaction.h>

QDesignerToolBox::QDesignerToolBox(QWidget *parent)
    : QToolBox(parent)
{
    m_actionDeletePage = new QAction(this);
    m_actionDeletePage->setText(tr("Delete Page"));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));

    m_actionInsertPage = new QAction(this);
    m_actionInsertPage->setText(tr("Add Page"));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));
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
        return QString::null;

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

    if (FormWindow *fw = FormWindow::findFormWindow(this)) {
        DeleteToolBoxPageCommand *cmd = new DeleteToolBoxPageCommand(fw);
        cmd->init(this);

        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBox::addPage()
{
    if (FormWindow *fw = FormWindow::findFormWindow(this)) {
        AddToolBoxPageCommand *cmd = new AddToolBoxPageCommand(fw);
        cmd->init(this);
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
    return widget(0) ? widget(0)->backgroundRole() : QPalette::Background;
}

void QDesignerToolBox::setCurrentItemBackgroundRole(QPalette::ColorRole role)
{
    for (int i = 0; i < count(); ++i) {
        QWidget *w = widget(i);
        w->setBackgroundRole(role);
        w->update();
    }
}


#include "qdesigner_stackedbox.h"
#include "formwindow.h"
#include "command.h"
#include "qtundo.h"
#include "abstractformeditor.h"
#include "abstractpropertyeditor.h"
#include <abstractmetadatabase.h>

#include <qextensionmanager.h>
#include <propertysheet.h>

#include <qevent.h>
#include <qtoolbutton.h>
#include <qaction.h>

QDesignerStackedWidget::QDesignerStackedWidget(QWidget *parent)
    : QStackedWidget(parent)
{
    prev = new QToolButton(Qt::LeftArrow, this);
    prev->setObjectName("designer_wizardstack_button");
    prev->setAutoRaise(true);
    prev->setAutoRepeat(true);
    prev->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(prev, SIGNAL(clicked()), this, SLOT(prevPage()));
    removeWidget(prev);

    next = new QToolButton(Qt::RightArrow, this);
    next->setObjectName("designer_wizardstack_button");
    next->setAutoRaise(true);
    next->setAutoRepeat(true);
    next->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(next, SIGNAL(clicked()), this, SLOT(nextPage()));
    removeWidget(next);

    updateButtons();

    m_actionPreviousPage = new QAction(tr("Previous Page"));
    connect(m_actionPreviousPage, SIGNAL(triggered()), this, SLOT(prevPage()));

    m_actionNextPage = new QAction(tr("Next Page"));
    connect(m_actionNextPage, SIGNAL(triggered()), this, SLOT(nextPage()));

    m_actionDeletePage = new QAction(tr("Delete Page"));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));

    m_actionInsertPage = new QAction(tr("Add Page"));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));
}

void QDesignerStackedWidget::removeCurrentPage()
{
    if (currentIndex() == -1)
        return;

    if (FormWindow *fw = FormWindow::findFormWindow(this)) {
        DeleteStackedWidgetPageCommand *cmd = new DeleteStackedWidgetPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerStackedWidget::addPage()
{
    if (FormWindow *fw = FormWindow::findFormWindow(this)) {
        AddStackedWidgetPageCommand *cmd = new AddStackedWidgetPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerStackedWidget::updateButtons()
{
    if (prev) {
        prev->setGeometry(width() - 31, 1, 15, 15);
        prev->show();
        prev->raise();
    }

    if (next) {
        next->setGeometry(width() - 16, 1, 15, 15);
        next->show();
        next->raise();
    }
}

void QDesignerStackedWidget::prevPage()
{
    if (FormWindow *fw = FormWindow::findFormWindow(this)) {
        int newIndex = currentIndex() - 1;
        if (newIndex < 0)
            newIndex = count() -1;
    
#if 0 // ### port me [command]
        QString pn(tr("Raise previous page of '%2'").arg(objectName()));
        SetPropertyCommand *cmd =
            new SetPropertyCommand(pn, fw, this,
                                    "currentIndex", currentIndex(), newIndex);
        fw->commandHistory()->push(cmd);
        if (IPropertySheet* sheet = Q_IDE_EXTENSION(fw->core(), this, IPropertySheet)) {
            sheet->setChanged(sheet->indexOf("currentIndex"), true);
        }
        updateButtons();
#endif
    }
}

void QDesignerStackedWidget::nextPage()
{
    if (FormWindow *fw = FormWindow::findFormWindow(this)) {

        int newIndex = (currentIndex() + 1) % count();
    
#if 0 // ### port me [command]
        QString pn(tr("Raise next page of '%2'").arg(objectName()));
        SetPropertyCommand *cmd =
            new SetPropertyCommand(pn, fw, this, fw->core()->propertyEditor(),
                                    "currentIndex", currentIndex(),
                                    newIndex, QString::null, QString::null);
        fw->commandHistory()->push(cmd);
#endif
        setCurrentIndex(newIndex);
    
        if (IPropertySheet* sheet = qt_extension<IPropertySheet*>(fw->core()->extensionManager(), this)) {
            sheet->setChanged(sheet->indexOf("currentIndex"), true);
        }
        updateButtons();
    }
}

void QDesignerStackedWidget::childEvent(QChildEvent *e)
{
    QStackedWidget::childEvent(e);

    if (e->added() && (e->child() == prev || e->child() == next)) {
        removeWidget(static_cast<QWidget*>(e->child()));
        updateButtons();
    }
}

void QDesignerStackedWidget::resizeEvent(QResizeEvent *e)
{
    QStackedWidget::resizeEvent(e);
    updateButtons();
}

void QDesignerStackedWidget::showEvent(QShowEvent *e)
{
    QStackedWidget::showEvent(e);
    updateButtons();
}

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

#include "qdesigner_tabwidget.h"
#include "command.h"
#include "formwindow.h"

#include <qapplication.h>
#include <qtabbar.h>
#include <qaction.h>
#include <qevent.h>
#include <qdebug.h>

#if 0 // ### port me [DnD]
class TabDrag : public QStoredDrag
{
public:
    TabDrag(long addr, QWidget *parent = 0)
        : QStoredDrag("designer/tab", parent)
    { setEncodedData(QByteArray::number(addr)); }

    static bool canDecode(QDragMoveEvent *e)
    { return e->provides("designer/tab"); }

    static bool decode(QDropEvent *e, long *addr)
    {
        QByteArray data = e->data("designer/tab"); 
        if (data.isEmpty())
            return false;

        e->accept();
        *addr = data.toLong();
        return true;
    }
};
#endif

QDesignerTabWidget::QDesignerTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    dropIndicator = 0;
    dragPage = 0;
    mousePressed = false;

    tabBar()->setAcceptDrops(true);
    tabBar()->installEventFilter(this);

    m_actionInsertPage = new QAction(this);
    m_actionInsertPage->setText(tr("Add Page"));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));

    m_actionDeletePage = new QAction(this);
    m_actionDeletePage->setText(tr("Delete Page"));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));
    
    connect(this, SIGNAL(currentChanged(int)),
        this, SLOT(slotCurrentChanged(int)));
}

QDesignerTabWidget::~QDesignerTabWidget()
{
}

QString QDesignerTabWidget::currentTabName() const
{
    return currentWidget()
        ? currentWidget()->objectName()
        : QString::null;
}

void QDesignerTabWidget::setCurrentTabName(const QString &tabName)
{
    if (QWidget *w = currentWidget())
        w->setObjectName(tabName);
}

QString QDesignerTabWidget::currentTabText() const
{
    return tabText(currentIndex());
}

void QDesignerTabWidget::setCurrentTabText(const QString &tabText)
{
    setTabText(currentIndex(), tabText);
}

QString QDesignerTabWidget::currentTabToolTip() const
{
    return tabToolTip(currentIndex());
}

void QDesignerTabWidget::setCurrentTabToolTip(const QString &tabToolTip)
{
    setTabToolTip(currentIndex(), tabToolTip);
}

QIcon QDesignerTabWidget::currentTabIcon() const
{
    return tabIcon(currentIndex());
}

void QDesignerTabWidget::setCurrentTabIcon(const QIcon &tabIcon)
{
    setTabIcon(currentIndex(), tabIcon);
}

bool QDesignerTabWidget::eventFilter(QObject *, QEvent *)
{
#if 0 // ### port me [DnD]
    if (o != tabBar())
        return false;

    switch (e->type()) {
        case QEvent::MouseButtonDblClick:
            break;

        case QEvent::MouseButtonPress: {
            QMouseEvent *mev = static_cast<QMouseEvent*>(e);
            if (mev->button() & Qt::LeftButton) {
                mousePressed = true;
                pressPoint = mev->pos();
            }
        }
        break;

        case QEvent::MouseButtonRelease:
            mousePressed = false;
            break;

        case QEvent::MouseMove:
            if (mousePressed && canMove(static_cast<QMouseEvent*>(e))) {
                mousePressed = false;
                TabDrag *drg = new TabDrag((long) this, this);

                int index = currentIndex();
                dragPage = currentWidget();
                dragLabel = currentTabText();

                removeTab(index);

                if (!drg->dragMove()) {
                    insertTab(index, dragPage, dragLabel);
                    setCurrentIndex(index);
                }

                if (dropIndicator)
                    dropIndicator->hide();
            }
            break;

        case QEvent::DragLeave:
            if (dropIndicator)
                dropIndicator->hide();
            break;

        case QEvent::DragMove: {
            QDragEnterEvent *de = static_cast<QDragEnterEvent*>(e);
            if (TabDrag::canDecode(de)) {
                long addr = 0;
                TabDrag::decode(de, &addr);
                if (addr == (long)this)
                    de->accept();
                else
                    return false;
            }

            int index = 0;
            QRect rect;
            for (; index < count(); index++) {
                QRect rc = tabBar()->tabRect(index);
                if (rc.contains(de->pos())) {
                    rect = rc;
                    break;
                }
            }

            if (index == count() -1) {
                QRect rect2 = rect;
                rect2.setLeft(rect2.left() + rect2.width() / 2);
                if (rect2.contains(de->pos()))
                    index++;
            }

            if (!dropIndicator) {
                dropIndicator = new QWidget(this);
                QPalette p = dropIndicator->palette();
                p.setColor(backgroundRole(), Qt::red);
                dropIndicator->setPalette(p);
            }

            QPoint pos;
            if (index == count())
                pos = tabBar()->mapToParent(QPoint(rect.x() + rect.width(), rect.y()));
            else
                pos = tabBar()->mapToParent(QPoint(rect.x(), rect.y()));

            dropIndicator->setGeometry(pos.x(), pos.y() , 3, rect.height());
            dropIndicator->show();
        }
        break;

        case QEvent::Drop: {
            QDragEnterEvent *de = (QDragEnterEvent*) e;
            if (TabDrag::canDecode(de)) {
                long addr;
                TabDrag::decode(de, &addr);
                if (addr == (long)this) {
                    int newIndex = 0;
                    for (; newIndex < count(); newIndex++) {
                        QRect rc = tabBar()->tabRect(newIndex);
                        if (rc.contains(de->pos()))
                            break;
                    }

                    if (newIndex == count() -1) {
                        QRect rect2 = tabBar()->tabRect(newIndex);
                        rect2.setLeft(rect2.left() + rect2.width() / 2);
                        if (rect2.contains(de->pos()))
                            newIndex++;
                    }

                    int oldIndex = 0;
                    for (; oldIndex < count(); oldIndex++) {
                        QRect rc = tabBar()->tabRect(oldIndex);
                        if (rc.contains(pressPoint))
                            break;
                    }

#if 0 // ### port me [command]
                    FormWindow *fw = FormWindow::findFormWindow(this);
                    MoveTabPageCommand *cmd =
                        new MoveTabPageCommand(tr("Move Tab Page"), fw, this,
                                                dragPage, dragLabel, newIndex, oldIndex);
                    fw->commandHistory()->push(cmd);
#endif
                    de->accept();
                }
            }
        }
        break;

        default:
            break;
    }
#endif

    return false;
}

void QDesignerTabWidget::removeCurrentPage()
{
    if (!currentWidget())
        return;

    if (FormWindow *fw = FormWindow::findFormWindow(this)) {
        DeleteTabPageCommand *cmd = new DeleteTabPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTabWidget::addPage()
{
    if (FormWindow *fw = FormWindow::findFormWindow(this)) {
        AddTabPageCommand *cmd = new AddTabPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

bool QDesignerTabWidget::canMove(QMouseEvent *e) const
{
    QPoint pt = pressPoint - e->pos();
    return pt.manhattanLength() > QApplication::startDragDistance();
}

void QDesignerTabWidget::slotCurrentChanged(int)
{
#if 0 // ### enable me
    if (QWidget *page = widget(index)) {
        if (FormWindow *fw = FormWindow::findFormWindow(this)) {
            fw->clearSelection();
            fw->selectWidget(this, true);
        }
    }
#endif
}

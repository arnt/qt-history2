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

#include "formwindowmanager.h"
#include "formwindow_dnditem.h"
#include "widgetdatabase.h"
#include "iconloader.h"
#include "widgetselection.h"
#include "qdesigner_resource.h"
#include "connectionedit.h"

#include <abstractwidgetfactory.h>
#include <abstractformeditor.h>
#include <abstractmetadatabase.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <qdesigner_promotedwidget.h>
#include <qdesigner_command.h>
#include <layoutinfo.h>

#include <QtGui/QAction>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtGui/QBitmap>
#include <QtGui/QPainter>
#include <QtGui/QSizeGrip>
#include <QtGui/QAbstractButton>
#include <QtGui/QToolBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QClipboard>

#include <QtCore/qdebug.h>

static QString whatsThisFrom(const QString &str)
{
    Q_UNUSED(str); /// ### implement me!
    return str;
}

FormWindowManager::FormWindowManager(AbstractFormEditor *core, QObject *parent)
    : AbstractFormWindowManager(parent),
      m_core(core),
      m_activeFormWindow(0)
{
    lastWasAPassiveInteractor = false;

    m_layoutChilds = false;
    m_layoutSelected = false;
    m_breakLayout = false;

    setupActions();
    qApp->installEventFilter(this);

    // DnD stuff
    m_last_widget_under_mouse = 0;
    m_last_form_under_mouse = 0;
    m_source_form = 0;
}

FormWindowManager::~FormWindowManager()
{
    qDeleteAll(m_formWindows);
}

AbstractFormEditor *FormWindowManager::core() const
{
    return m_core;
}

AbstractFormWindow *FormWindowManager::activeFormWindow() const
{
    return m_activeFormWindow;
}

int FormWindowManager::formWindowCount() const
{
    return m_formWindows.size();
}

AbstractFormWindow *FormWindowManager::formWindow(int index) const
{
    return m_formWindows.at(index);
}

static bool isMouseMoveOrRelease(QEvent *e)
{
    return e->type() == QEvent::MouseButtonRelease
            || e->type() == QEvent::MouseMove;
}

bool FormWindowManager::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_core->topLevel()
            && !m_drag_item_list.isEmpty()
            && isMouseMoveOrRelease(e)) {
        // We're dragging
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        me->accept();

        if (me->type() == QEvent::MouseButtonRelease)
            endDrag(me->globalPos());
        else
            setItemsPos(me->globalPos());
        return true;
    }

    QWidget *widget = static_cast<QWidget*>(o);

    if (!o->isWidgetType()) {
        return false;
    }

    if (qobject_cast<WidgetHandle*>(widget)) { // ### remove me
        return false;
    }

    FormWindow *fw = FormWindow::findFormWindow(widget);
    if (fw == 0) {
        return false;
    }

    if (isPassiveInteractor(widget)) {
        return false;
    }

    if (QWidget *managedWidget = findManagedWidget(fw, widget)) {
       switch (e->type()) {
        case QEvent::Hide: {
            if (widget == managedWidget && fw->isWidgetSelected(managedWidget))
                fw->hideSelection(widget);
        } break;

        case QEvent::WindowActivate: {
            if (fw->isMainContainer(managedWidget)) {
                core()->formWindowManager()->setActiveFormWindow(fw);
            }
        } break;

        case QEvent::WindowDeactivate: {
            fw->repaintSelection();
        } break;

        default: {
            if (fw->handleEvent(widget, managedWidget, e)) {
                return true;
            }
        } break;

        } // end switch
    }

    return false;
}

void FormWindowManager::addFormWindow(AbstractFormWindow *w)
{
    FormWindow *formWindow = qobject_cast<FormWindow*>(w);
    if (!formWindow || m_formWindows.contains(formWindow))
        return;

    connect(formWindow, SIGNAL(selectionChanged()), this, SLOT(slotUpdateActions()));
    connect(formWindow->commandHistory(), SIGNAL(commandExecuted()), this, SLOT(slotUpdateActions()));
    connect(formWindow, SIGNAL(toolChanged(int)), this, SLOT(slotUpdateActions()));

    m_formWindows.append(formWindow);
    emit formWindowAdded(formWindow);
}

void FormWindowManager::removeFormWindow(AbstractFormWindow *w)
{
    FormWindow *formWindow = qobject_cast<FormWindow*>(w);

    int idx = m_formWindows.indexOf(formWindow);
    if (!formWindow || idx == -1)
        return;

    formWindow->disconnect(this);
    m_formWindows.removeAt(idx);
    emit formWindowRemoved(formWindow);

    if (formWindow == m_activeFormWindow)
        setActiveFormWindow(0);
}

void FormWindowManager::setActiveFormWindow(AbstractFormWindow *w)
{
    FormWindow *formWindow = qobject_cast<FormWindow*>(w);

    if (formWindow == m_activeFormWindow)
        return;

    FormWindow *old = m_activeFormWindow;

    m_activeFormWindow = formWindow;

    slotUpdateActions();

    if (m_activeFormWindow) {
        m_activeFormWindow->repaintSelection();
        if (old)
            old->repaintSelection();
    }

    emit activeFormWindowChanged(m_activeFormWindow);

    if (m_activeFormWindow) {
        m_activeFormWindow->emitSelectionChanged();
        m_activeFormWindow->commandHistory()->setCurrent();
    }
}

QWidget *FormWindowManager::findManagedWidget(FormWindow *fw, QWidget *w)
{
    while (w && w != fw) {
        if (fw->isManaged(w))
            break;
        w = w->parentWidget();
    }

    QWidget *parent = w->parentWidget();
    if (parent != 0 && qobject_cast<QDesignerPromotedWidget*>(parent) != 0)
        w = parent;

    return w;
}

void FormWindowManager::setupActions()
{
    m_actionCut = new QAction(createIconSet("editcut.png"), tr("Cu&t"));
    m_actionCut->setShortcut(Qt::CTRL + Qt::Key_X);
    m_actionCut->setStatusTip(tr("Cuts the selected widgets and puts them on the clipboard"));
    m_actionCut->setWhatsThis(whatsThisFrom("Edit|Cut"));
    connect(m_actionCut, SIGNAL(triggered()), this, SLOT(slotActionCutActivated()));
    m_actionCut->setEnabled(false);

    m_actionCopy = new QAction(createIconSet("editcopy.png"), tr("&Copy"));
    m_actionCopy->setShortcut(Qt::CTRL + Qt::Key_C);
    m_actionCopy->setStatusTip(tr("Copies the selected widgets to the clipboard"));
    m_actionCopy->setWhatsThis(whatsThisFrom("Edit|Copy"));
    connect(m_actionCopy, SIGNAL(triggered()), this, SLOT(slotActionCopyActivated()));
    m_actionCopy->setEnabled(false);

    m_actionPaste = new QAction(createIconSet("editpaste.png"), tr("&Paste"));
    m_actionPaste->setShortcut(Qt::CTRL + Qt::Key_V);
    m_actionPaste->setStatusTip(tr("Pastes the clipboard's contents"));
    m_actionPaste->setWhatsThis(whatsThisFrom("Edit|Paste"));
    connect(m_actionPaste, SIGNAL(triggered()), this, SLOT(slotActionPasteActivated()));
    m_actionPaste->setEnabled(false);

    m_actionDelete = new QAction(tr("&Delete"));
    m_actionDelete->setShortcut(Qt::Key_Delete);
    m_actionDelete->setStatusTip(tr("Deletes the selected widgets"));
    m_actionDelete->setWhatsThis(whatsThisFrom("Edit|Delete"));
    connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(slotActionDeleteActivated()));
    m_actionDelete->setEnabled(false);

    m_actionSelectAll = new QAction(tr("Select &All"));
    m_actionSelectAll->setShortcut(Qt::CTRL + Qt::Key_A);
    m_actionSelectAll->setStatusTip(tr("Selects all widgets"));
    m_actionSelectAll->setWhatsThis(whatsThisFrom("Edit|Select All"));
    connect(m_actionSelectAll, SIGNAL(triggered()), this, SLOT(slotActionSelectAllActivated()));
    m_actionSelectAll->setEnabled(false);

    m_actionRaise = new QAction(createIconSet("editraise.png"), tr("Bring to &Front"));
    m_actionRaise->setStatusTip(tr("Raises the selected widgets"));
    m_actionRaise->setWhatsThis(tr("Raises the selected widgets"));
    connect(m_actionRaise, SIGNAL(triggered()), this, SLOT(slotActionRaiseActivated()));
    m_actionRaise->setEnabled(false);

    m_actionLower = new QAction(createIconSet("editlower.png"), tr("Send to &Back"));
    m_actionLower->setStatusTip(tr("Lowers the selected widgets"));
    m_actionLower->setWhatsThis(tr("Lowers the selected widgets"));
    connect(m_actionLower, SIGNAL(triggered()), this, SLOT(slotActionLowerActivated()));
    m_actionLower->setEnabled(false);


    m_actionAdjustSize = new QAction(createIconSet("adjustsize.png"), tr("Adjust &Size"));
    m_actionAdjustSize->setShortcut(Qt::CTRL + Qt::Key_J);
    m_actionAdjustSize->setStatusTip(tr("Adjusts the size of the selected widget"));
    m_actionAdjustSize->setWhatsThis(whatsThisFrom("Layout|Adjust Size"));
    connect(m_actionAdjustSize, SIGNAL(triggered()), this, SLOT(slotActionAdjustSizeActivated()));
    m_actionAdjustSize->setEnabled(false);

    m_actionHorizontalLayout = new QAction(createIconSet("edithlayout.png"), tr("Lay Out &Horizontally"));
    m_actionHorizontalLayout->setShortcut(Qt::CTRL + Qt::Key_H);
    m_actionHorizontalLayout->setStatusTip(tr("Lays out the selected widgets horizontally"));
    m_actionHorizontalLayout->setWhatsThis(whatsThisFrom("Layout|Lay Out Horizontally"));
    connect(m_actionHorizontalLayout, SIGNAL(triggered()), this, SLOT(slotActionHorizontalLayoutActivated()));
    m_actionHorizontalLayout->setEnabled(false);

    m_actionVerticalLayout = new QAction(createIconSet("editvlayout.png"), tr("Lay Out &Vertically"));
    m_actionVerticalLayout->setShortcut(Qt::CTRL + Qt::Key_L);
    m_actionVerticalLayout->setStatusTip(tr("Lays out the selected widgets vertically"));
    m_actionVerticalLayout->setWhatsThis(whatsThisFrom("Layout|Lay Out Vertically"));
    connect(m_actionVerticalLayout, SIGNAL(triggered()), this, SLOT(slotActionVerticalLayoutActivated()));
    m_actionVerticalLayout->setEnabled(false);

    m_actionGridLayout = new QAction(createIconSet("editgrid.png"), tr("Lay Out in a &Grid"));
    m_actionGridLayout->setShortcut(Qt::CTRL + Qt::Key_G);
    m_actionGridLayout->setStatusTip(tr("Lays out the selected widgets in a grid"));
    m_actionGridLayout->setWhatsThis(whatsThisFrom("Layout|Lay Out in a Grid"));
    connect(m_actionGridLayout, SIGNAL(triggered()), this, SLOT(slotActionGridLayoutActivated()));
    m_actionGridLayout->setEnabled(false);

    m_actionSplitHorizontal = new QAction(createIconSet("editvlayoutsplit.png"),
                                             tr("Lay Out Horizontally in S&plitter"));
    m_actionSplitHorizontal->setStatusTip(tr("Lays out the selected widgets horizontally in a splitter"));
    m_actionSplitHorizontal->setWhatsThis(whatsThisFrom("Layout|Lay Out Horizontally in Splitter"));
    connect(m_actionSplitHorizontal, SIGNAL(triggered()), this, SLOT(slotActionSplitHorizontalActivated()));
    m_actionSplitHorizontal->setEnabled(false);

    m_actionSplitVertical = new QAction(createIconSet("edithlayoutsplit.png"),
                                             tr("Lay Out Vertically in Sp&litter"));
    m_actionSplitVertical->setStatusTip(tr("Lays out the selected widgets vertically in a splitter"));
    m_actionSplitVertical->setWhatsThis(whatsThisFrom("Layout|Lay Out Vertically in Splitter"));
    connect(m_actionSplitVertical, SIGNAL(triggered()), this, SLOT(slotActionSplitVerticalActivated()));
    m_actionSplitVertical->setEnabled(false);

    m_actionBreakLayout = new QAction(createIconSet("editbreaklayout.png"), tr("&Break Layout"));
    m_actionBreakLayout->setShortcut(Qt::CTRL + Qt::Key_B);
    m_actionBreakLayout->setStatusTip(tr("Breaks the selected layout"));
    m_actionBreakLayout->setWhatsThis(whatsThisFrom("Layout|Break Layout"));
    connect(m_actionBreakLayout, SIGNAL(triggered()), this, SLOT(slotActionBreakLayoutActivated()));
    m_actionBreakLayout->setEnabled(false);

    m_actionUndo = new QAction(tr("Undo"));
    m_actionUndo->setShortcut(Qt::CTRL + Qt::Key_Z);
    m_actionUndo->setEnabled(false);
    m_actionRedo = new QAction(tr("Redo"));
    m_actionRedo->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    m_actionRedo->setEnabled(false);
}

void FormWindowManager::slotActionCutActivated()
{
    m_activeFormWindow->cut();
}

void FormWindowManager::slotActionCopyActivated()
{
    m_activeFormWindow->copy();
    slotUpdateActions();
}

void FormWindowManager::slotActionPasteActivated()
{
    m_activeFormWindow->paste();
}

void FormWindowManager::slotActionDeleteActivated()
{
    m_activeFormWindow->deleteWidgets();
}

void FormWindowManager::slotActionLowerActivated()
{
    m_activeFormWindow->lowerWidgets();
}

void FormWindowManager::slotActionRaiseActivated()
{
    m_activeFormWindow->lowerWidgets();
}

void FormWindowManager::slotActionHorizontalLayoutActivated()
{
    if (m_layoutChilds)
        layoutContainerHorizontal();
    else if (m_layoutSelected)
        m_activeFormWindow->layoutHorizontal();
}

void FormWindowManager::slotActionVerticalLayoutActivated()
{
    if (m_layoutChilds)
        layoutContainerVertical();
    else if (m_layoutSelected)
        m_activeFormWindow->layoutVertical();
}

void FormWindowManager::slotActionGridLayoutActivated()
{
    if (m_layoutChilds)
        layoutContainerGrid();
    else if (m_layoutSelected)
        m_activeFormWindow->layoutGrid();
}

void FormWindowManager::slotActionSplitHorizontalActivated()
{
    if (m_layoutChilds)
        ; // no way to do that
    else if (m_layoutSelected)
        m_activeFormWindow->layoutHorizontalSplit();
}

void FormWindowManager::slotActionSplitVerticalActivated()
{
    if (m_layoutChilds)
        ; // no way to do that
    else if (m_layoutSelected)
        m_activeFormWindow->layoutVerticalSplit();
}

void FormWindowManager::slotActionBreakLayoutActivated()
{
    if (!m_breakLayout)
        return;
    QWidget *w = m_activeFormWindow->mainContainer();
    if (m_activeFormWindow->currentWidget())
        w = m_activeFormWindow->currentWidget();
    if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
         w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout) {
        m_activeFormWindow->breakLayout(w);
        return;
    } else {
        QList<QWidget*> widgets = m_activeFormWindow->selectedWidgets();
        for (int i = 0; i < widgets.size(); ++i) {
            QWidget *w = widgets.at(i);
            if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
                 w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout)
                break;
        }
        if (w) {
            m_activeFormWindow->breakLayout(w);
            return;
        }
    }

    w = m_activeFormWindow->mainContainer();
    if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
         w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout)
        m_activeFormWindow->breakLayout(w);
}

void FormWindowManager::slotActionAdjustSizeActivated()
{
    Q_ASSERT(m_activeFormWindow != 0);

    m_activeFormWindow->beginCommand(tr("Adjust Size"));

    AbstractFormWindowCursor *cursor = m_activeFormWindow->cursor();

    for (int index = 0; index < cursor->selectedWidgetCount(); ++index) {
        QWidget *widget = cursor->selectedWidget(index);
        AdjustWidgetSizeCommand *cmd = new AdjustWidgetSizeCommand(m_activeFormWindow);
        cmd->init(widget);
        m_activeFormWindow->commandHistory()->push(cmd);
    }

    m_activeFormWindow->endCommand();
}

void FormWindowManager::slotActionSelectAllActivated()
{
    m_activeFormWindow->selectAll();
}

void FormWindowManager::slotUpdateActions()
{
    m_layoutChilds = false;
    m_layoutSelected = false;
    m_breakLayout = false;

    if (!m_activeFormWindow
            || m_activeFormWindow->currentTool() != 0) { // ### FormWindow::currentTool() doesn't make sense anymore!
        m_actionCut->setEnabled(false);
        m_actionCopy->setEnabled(false);
        m_actionPaste->setEnabled(false);
        m_actionDelete->setEnabled(false);
        m_actionSelectAll->setEnabled(false);
        m_actionAdjustSize->setEnabled(false);
        m_actionHorizontalLayout->setEnabled(false);
        m_actionVerticalLayout->setEnabled(false);
        m_actionSplitHorizontal->setEnabled(false);
        m_actionSplitVertical->setEnabled(false);
        m_actionGridLayout->setEnabled(false);
        m_actionBreakLayout->setEnabled(false);
        m_actionLower->setEnabled(false);
        m_actionRaise->setEnabled(false);
        m_actionAdjustSize->setEnabled(false);

        if (!m_activeFormWindow) {
            m_actionUndo->setText(tr("Undo"));
            m_actionUndo->setEnabled(false);
            m_actionRedo->setText(tr("Redo"));
            m_actionRedo->setEnabled(false);
        } else {
            m_actionUndo->setEnabled(m_activeFormWindow->commandHistory()->canUndo());
            m_actionUndo->setText(tr("Undo ") + m_activeFormWindow->commandHistory()->undoDescription());
            m_actionRedo->setEnabled(m_activeFormWindow->commandHistory()->canRedo());
            m_actionRedo->setText(tr("Redo ") + m_activeFormWindow->commandHistory()->redoDescription());
        }
        return;
    }

    QList<QWidget*> widgets = m_activeFormWindow->selectedWidgets();
    int selectedWidgets = widgets.size();

    bool enable = selectedWidgets > 0;

    m_actionCut->setEnabled(enable);
    m_actionCopy->setEnabled(enable);
    m_actionPaste->setEnabled(qApp->clipboard()->mimeData() && qApp->clipboard()->mimeData()->hasText());
    m_actionDelete->setEnabled(enable);
    m_actionLower->setEnabled(enable);
    m_actionRaise->setEnabled(enable);
    m_actionSelectAll->setEnabled(true);

    m_actionAdjustSize->setEnabled(false);
    m_actionSplitHorizontal->setEnabled(false);
    m_actionSplitVertical->setEnabled(false);

    if (selectedWidgets == 0 && m_activeFormWindow->mainContainer() != 0) {
        widgets.append(m_activeFormWindow->mainContainer());
        selectedWidgets = 1;
    }

    enable = false;
    if (selectedWidgets > 1) {
        int unlaidout = 0;
        int laidout = 0;

        foreach (QWidget *w, widgets) {
            if (!w->parentWidget() || LayoutInfo::layoutType(m_core, w->parentWidget()) == LayoutInfo::NoLayout)
                unlaidout++;
            else
                laidout++;
        }

        m_actionHorizontalLayout->setEnabled(unlaidout > 1);
        m_actionVerticalLayout->setEnabled(unlaidout > 1);
        m_actionSplitHorizontal->setEnabled(unlaidout > 1);
        m_actionSplitVertical->setEnabled(unlaidout > 1);
        m_actionGridLayout->setEnabled(unlaidout > 1);
        m_actionBreakLayout->setEnabled(laidout > 0);
        m_actionAdjustSize->setEnabled(laidout > 0);
        m_layoutSelected = unlaidout > 1;
        m_breakLayout = laidout > 0;
    } else if (selectedWidgets == 1) {
        QWidget *w = widgets.first();
        bool isContainer = core()->widgetDataBase()->isContainer(w)
            || w == m_activeFormWindow->mainContainer();

        m_actionAdjustSize->setEnabled(!w->parentWidget()
            || LayoutInfo::layoutType(m_core, w->parentWidget()) == LayoutInfo::NoLayout);

        if (isContainer == false) {
            m_actionHorizontalLayout->setEnabled(false);
            m_actionVerticalLayout->setEnabled(false);
            m_actionGridLayout->setEnabled(false);
            if (w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout) {
                m_actionBreakLayout->setEnabled(m_activeFormWindow->widgets(w->parentWidget()).isEmpty() == false);
                m_breakLayout = true;
            } else {
                m_actionBreakLayout->setEnabled(false);
            }
        } else {
            if (LayoutInfo::layoutType(m_core, w) == LayoutInfo::NoLayout) {
                m_layoutChilds = m_activeFormWindow->hasInsertedChildren(w);

                m_actionHorizontalLayout->setEnabled(m_layoutChilds);
                m_actionVerticalLayout->setEnabled(m_layoutChilds);
                m_actionGridLayout->setEnabled(m_layoutChilds);
                m_actionBreakLayout->setEnabled(m_layoutChilds);

                if (w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout) {
                    m_actionBreakLayout->setEnabled(m_activeFormWindow->widgets(w->parentWidget()).isEmpty() == false);
                    m_breakLayout = true;
                }
            } else {
                m_actionHorizontalLayout->setEnabled(false);
                m_actionVerticalLayout->setEnabled(false);
                m_actionGridLayout->setEnabled(false);
                m_actionBreakLayout->setEnabled(m_activeFormWindow->widgets(w).isEmpty() == false);
                m_breakLayout = true;
            }
        }
    }

    m_actionUndo->setEnabled(m_activeFormWindow->commandHistory()->canUndo());
    m_actionUndo->setText(tr("Undo ") + m_activeFormWindow->commandHistory()->undoDescription());
    m_actionRedo->setEnabled(m_activeFormWindow->commandHistory()->canRedo());
    m_actionRedo->setText(tr("Redo ") + m_activeFormWindow->commandHistory()->redoDescription());
}

void FormWindowManager::layoutContainerHorizontal()
{
    QWidget *w = m_activeFormWindow->mainContainer();
    QList<QWidget*> l(m_activeFormWindow->selectedWidgets());
    if (l.count() == 1)
        w = l.first();

    if (w != 0)
        m_activeFormWindow->layoutHorizontalContainer(w);
}

void FormWindowManager::layoutContainerVertical()
{
    QWidget *w = m_activeFormWindow->mainContainer();
    QList<QWidget*> l(m_activeFormWindow->selectedWidgets());
    if (l.count() == 1)
        w = l.first();

    if (w)
        m_activeFormWindow->layoutVerticalContainer(w);
}

void FormWindowManager::layoutContainerGrid()
{
    QWidget *w = m_activeFormWindow->mainContainer();
    QList<QWidget*> l(m_activeFormWindow->selectedWidgets());
    if (l.count() == 1)
        w = l.first();

    if (w)
        m_activeFormWindow->layoutGridContainer(w);
}

AbstractFormWindow *FormWindowManager::createFormWindow(QWidget *parentWidget, Qt::WFlags flags)
{
    FormWindow *formWindow = new FormWindow(qobject_cast<FormEditor*>(core()), parentWidget, flags);
    addFormWindow(formWindow);
    return formWindow;
}

QAction *FormWindowManager::actionUndo() const
{
    return m_actionUndo;
}

QAction *FormWindowManager::actionRedo() const
{
    return m_actionRedo;
}

// DnD stuff

void FormWindowManager::dragItems(const QList<AbstractDnDItem*> &item_list, AbstractFormWindow *source_form)
{
    if (!m_drag_item_list.isEmpty()) {
        qWarning("FormWindowManager::dragItem(): called while already dragging");
        return;
    }

    m_source_form = qobject_cast<FormWindow*>(source_form);

    beginDrag(item_list, QCursor::pos());
}

void FormWindowManager::beginDrag(const QList<AbstractDnDItem*> &item_list, const QPoint &globalPos)
{
    Q_ASSERT(m_drag_item_list.isEmpty());

    m_drag_item_list = item_list;

    setItemsPos(globalPos);

    foreach(AbstractDnDItem *item, m_drag_item_list) {
        QWidget *deco = item->decoration();
        QBitmap bitmap(deco->size());
        QPainter p(&bitmap);
        p.fillRect(bitmap.rect(), Qt::color1);
        p.setPen(Qt::color0);
        p.drawPoint(deco->mapFromGlobal(globalPos));
        p.end();
        deco->setMask(bitmap);
        deco->show();
        deco->setWindowOpacity(0.8);
    }

    m_core->topLevel()->installEventFilter(this);
    m_core->topLevel()->grabMouse();
}

void FormWindowManager::setItemsPos(const QPoint &globalPos)
{
    foreach(AbstractDnDItem *item, m_drag_item_list)
        item->decoration()->move(globalPos - item->hotSpot());

    QWidget *widget_under_mouse = qApp->widgetAt(globalPos);
    int max_try = 3;
    while (max_try && widget_under_mouse && isDecoration(widget_under_mouse)) {
        --max_try;
        widget_under_mouse = qApp->widgetAt(widget_under_mouse->pos() - QPoint(1,1));
        Q_ASSERT(!qobject_cast<ConnectionEdit*>(widget_under_mouse));
    }

    FormWindow *form_under_mouse
            = qobject_cast<FormWindow*>(AbstractFormWindow::findFormWindow(widget_under_mouse));
    if (form_under_mouse != 0 && !form_under_mouse->hasFeature(AbstractFormWindow::EditFeature))
        form_under_mouse = 0;
    if (form_under_mouse != 0) {
        // widget_under_mouse might be some temporary thing like the dropLine. We need
        // the actual widget that's part of the edited GUI.
        widget_under_mouse
            = form_under_mouse->widgetAt(form_under_mouse->mapFromGlobal(globalPos));

        Q_ASSERT(!qobject_cast<ConnectionEdit*>(widget_under_mouse));
    }

    if (m_last_form_under_mouse != 0 && widget_under_mouse != m_last_widget_under_mouse) {
        m_last_form_under_mouse->highlightWidget(m_last_widget_under_mouse,
                                    m_last_widget_under_mouse->mapFromGlobal(globalPos),
                                    FormWindow::Restore);
    }

    if (form_under_mouse != 0
        && (m_source_form == 0 || widget_under_mouse != m_source_form->mainContainer())) {

        form_under_mouse->highlightWidget(widget_under_mouse,
                                    widget_under_mouse->mapFromGlobal(globalPos),
                                    FormWindow::Highlight);
    }

    m_last_widget_under_mouse = widget_under_mouse;
    m_last_form_under_mouse = form_under_mouse;
}

void FormWindowManager::endDrag(const QPoint &pos)
{
    m_core->topLevel()->removeEventFilter(this);
    m_core->topLevel()->releaseMouse();

    Q_ASSERT(!m_drag_item_list.isEmpty());

    foreach (AbstractDnDItem *item, m_drag_item_list)
        item->decoration()->hide();

    if (m_last_form_under_mouse != 0 &&
            m_last_form_under_mouse->hasFeature(AbstractFormWindow::EditFeature)) {
        FormWindow *form = qobject_cast<FormWindow*>(m_last_form_under_mouse);

        form->beginCommand(tr("Drop widget"));

        QWidget *parent = m_last_widget_under_mouse;
        if (parent == 0)
            parent = form->mainContainer();

        form->mainContainer()->activateWindow();
        form->clearSelection(false);

        form->highlightWidget(m_last_widget_under_mouse,
                            m_last_widget_under_mouse->mapFromGlobal(pos),
                            FormWindow::Restore);

        if (m_drag_item_list.first()->domUi() != 0) {
            foreach (AbstractDnDItem *item, m_drag_item_list) {
                DomUI *dom_ui = item->domUi();
                Q_ASSERT(dom_ui != 0);

                QRect geometry = item->decoration()->geometry();
                QWidget *widget = form->createWidget(dom_ui, geometry, parent);
                form->selectWidget(widget, true);
            }
        } else if (qobject_cast<FormWindowDnDItem*>(m_drag_item_list.first()) != 0) {
            foreach (AbstractDnDItem *item, m_drag_item_list) {
                FormWindowDnDItem *form_item = qobject_cast<FormWindowDnDItem*>(item);
                Q_ASSERT(form_item != 0);

                QWidget *widget = form_item->widget();
                Q_ASSERT(widget != 0);

                QRect geometry = item->decoration()->geometry();

                if (parent == widget->parent()) {
                    geometry.moveTopLeft(parent->mapFromGlobal(geometry.topLeft()));
                    form->resizeWidget(widget, geometry);
                    form->selectWidget(widget, true);
                    widget->show();
                } else {

                    QDesignerResource builder(m_source_form);
                    DomUI *dom_ui = builder.copy(QList<QWidget*>() << widget);

                    m_source_form->deleteWidgets(QList<QWidget*>() << widget);
                    QWidget *widget = form->createWidget(dom_ui, geometry, parent);
                    form->selectWidget(widget, true);
                }
            }

        }

        form->endCommand();
    } else {
        foreach (AbstractDnDItem *item, m_drag_item_list) {
            FormWindowDnDItem *form_item = qobject_cast<FormWindowDnDItem*>(item);
            if (form_item != 0 && form_item->widget() != 0)
                form_item->widget()->show();
        }
    }

    foreach (AbstractDnDItem *item, m_drag_item_list)
        delete item;
    m_drag_item_list.clear();
    m_last_widget_under_mouse = 0;
    m_last_form_under_mouse = 0;
}

bool FormWindowManager::isDecoration(QWidget *widget) const
{
    foreach (AbstractDnDItem *item, m_drag_item_list) {
        if (item->decoration() == widget)
            return true;
    }

    return false;
}

bool FormWindowManager::isPassiveInteractor(QWidget *o) const
{
    if (lastPassiveInteractor && lastPassiveInteractor == o)
        return lastWasAPassiveInteractor;

    lastWasAPassiveInteractor = false;
    lastPassiveInteractor = o;

    if (qobject_cast<QTabBar*>(o))
        return (lastWasAPassiveInteractor = true);
    else if (qobject_cast<QSizeGrip*>(o))
        return (lastWasAPassiveInteractor = true);
    else if (qobject_cast<QAbstractButton*>(o)
            && (qobject_cast<QTabBar*>(o->parent()) || qobject_cast<QToolBox*>(o->parent())))
        return (lastWasAPassiveInteractor = true);
    else if (qobject_cast<QMenuBar*>(o) && qobject_cast<QMainWindow*>(o->parent()))
        return (lastWasAPassiveInteractor = true);
    else if (qstrcmp(o->metaObject()->className(), "QDockSeparator") == 0)
        return (lastWasAPassiveInteractor = true);
    else if (qstrcmp(o->metaObject()->className(), "QDockWindowSeparator") == 0)
        return (lastWasAPassiveInteractor = true);
    else if (o->objectName() == QLatin1String("designer_wizardstack_button"))
        return (lastWasAPassiveInteractor = true);
    else if (o->objectName().startsWith("__qt__passive_"))
        return (lastWasAPassiveInteractor = true);

    return lastWasAPassiveInteractor;
}

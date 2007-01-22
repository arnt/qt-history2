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

/*
TRANSLATOR qdesigner_internal::FormWindowManager
*/

#include "formwindowmanager.h"
#include "formwindow_dnditem.h"
#include "widgetdatabase_p.h"
#include "iconloader_p.h"
#include "widgetselection.h"
#include "qdesigner_resource.h"
#include "connectionedit_p.h"

#include <QtDesigner/QtDesigner>
#include <qdesigner_command_p.h>
#include <layoutinfo_p.h>
#include <qlayout_widget_p.h>

#include <QtGui/QUndoGroup>
#include <QtGui/QUndoStack>
#include <QtGui/QAction>
#include <QtGui/QLayout>
#include <QtGui/QSplitter>
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
#include <QtGui/QWorkspace>
#include <QtGui/QDesktopWidget>

#include <QtCore/qdebug.h>

namespace {
    const bool debugFWM = false;
    
    inline QString whatsThisFrom(const QString &str) { /// ### implement me!
        return str;
    }
    
    // find the first child of w in a sequence
    template <class Iterator>
        inline Iterator findFirstChildOf(Iterator it,Iterator end, const QWidget *w)    {
            for  (;it != end; ++it) {
                if (w->isAncestorOf(*it))
                    return  it;
            }
            return it;
        }
}

namespace qdesigner_internal {

FormWindowManager::FormWindowManager(QDesignerFormEditorInterface *core, QObject *parent)
    : QDesignerFormWindowManagerInterface(parent),
      m_core(core),
      m_activeFormWindow(0)
{
    m_layoutChilds = false;
    m_savedContextMenuPolicy = Qt::NoContextMenu;

    setupActions();
    qApp->installEventFilter(this);

    // DnD stuff
    m_last_widget_under_mouse = 0;
    m_last_form_under_mouse = 0;
    m_widget_box_under_mouse = 0;
}

FormWindowManager::~FormWindowManager()
{
    qDeleteAll(m_formWindows);
}

QDesignerFormEditorInterface *FormWindowManager::core() const
{
    return m_core;
}

QDesignerFormWindowInterface *FormWindowManager::activeFormWindow() const
{
    return m_activeFormWindow;
}

int FormWindowManager::formWindowCount() const
{
    return m_formWindows.size();
}

QDesignerFormWindowInterface *FormWindowManager::formWindow(int index) const
{
    return m_formWindows.at(index);
}

static bool isMouseMoveOrRelease(QEvent *e)
{
    return e->type() == QEvent::MouseButtonRelease || e->type() == QEvent::MouseMove;
}

bool FormWindowManager::eventFilter(QObject *o, QEvent *e)
{
    bool inDragMode =
#ifdef Q_WS_X11
        o == m_core->topLevel() &&
#endif
        ! m_drag_item_list.isEmpty();


    if (inDragMode && e->type() == QEvent::ShortcutOverride) {
        e->accept();
        endDrag(QPoint());
        return true;
    }

    else if (inDragMode && isMouseMoveOrRelease(e)) {
        // We're dragging
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        me->accept();

        if (me->type() == QEvent::MouseButtonRelease)
            endDrag(me->globalPos());
        else
            setItemsPos(me->globalPos());
        return true;
    }

    if (!o->isWidgetType())
        return false;

    QWidget *widget = static_cast<QWidget*>(o);

    if (qobject_cast<WidgetHandle*>(widget)) { // ### remove me
        return false;
    }

    FormWindow *fw = FormWindow::findFormWindow(widget);
    if (fw == 0) {
        return false;
    }

    if (QWidget *managedWidget = findManagedWidget(fw, widget)) {
       switch (e->type()) {
        case QEvent::Hide: {
            if (widget == managedWidget && fw->isWidgetSelected(managedWidget))
                fw->hideSelection(widget);
        } break;

        case QEvent::WindowActivate: {
            if (fw->parentWidget()->isWindow() && fw->isMainContainer(managedWidget) && activeFormWindow() != fw) {
                setActiveFormWindow(fw);
            }
        } break;

        case QEvent::WindowDeactivate: {
            if (o == fw && o == activeFormWindow())
                fw->repaintSelection();
        } break;

        case QEvent::KeyPress: {
            QKeyEvent *ke = static_cast<QKeyEvent*>(e);
            if (ke->key() == Qt::Key_Escape) {
                ke->accept();
                return true;
            }
        }
        // don't break...

        default: {
            if (fw->handleEvent(widget, managedWidget, e)) {
                return true;
            }
        } break;

        } // end switch
    }

    return false;
}

void FormWindowManager::addFormWindow(QDesignerFormWindowInterface *w)
{
    FormWindow *formWindow = qobject_cast<FormWindow*>(w);
    if (!formWindow || m_formWindows.contains(formWindow))
        return;

    connect(formWindow, SIGNAL(selectionChanged()), this, SLOT(slotUpdateActions()));
    connect(formWindow->commandHistory(), SIGNAL(indexChanged(int)), this, SLOT(slotUpdateActions()));
    connect(formWindow, SIGNAL(toolChanged(int)), this, SLOT(slotUpdateActions()));

    m_formWindows.append(formWindow);
    emit formWindowAdded(formWindow);
}

void FormWindowManager::removeFormWindow(QDesignerFormWindowInterface *w)
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

void FormWindowManager::setActiveFormWindow(QDesignerFormWindowInterface *w)
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
        m_activeFormWindow->commandHistory()->setActive();

        QWidget *parent = m_activeFormWindow->parentWidget();
        QWorkspace *workspace = 0;
        while (parent != 0) {
            if ((workspace = qobject_cast<QWorkspace*>(parent)))
                break;
            parent = parent->parentWidget();
        }
        if (workspace != 0)
            workspace->setActiveWindow(m_activeFormWindow->parentWidget());
    }
}

QWidget *FormWindowManager::findManagedWidget(FormWindow *fw, QWidget *w)
{
    while (w && w != fw) {
        if (fw->isManaged(w))
            break;
        w = w->parentWidget();
    }
    return w;
}

void FormWindowManager::setupActions()
{
    m_actionCut = new QAction(createIconSet(QLatin1String("editcut.png")), tr("Cu&t"), this);
    m_actionCut->setShortcut(Qt::CTRL + Qt::Key_X);
    m_actionCut->setStatusTip(tr("Cuts the selected widgets and puts them on the clipboard"));
    m_actionCut->setWhatsThis(whatsThisFrom(QLatin1String("Edit|Cut")));
    connect(m_actionCut, SIGNAL(triggered()), this, SLOT(slotActionCutActivated()));
    m_actionCut->setEnabled(false);

    m_actionCopy = new QAction(createIconSet(QLatin1String("editcopy.png")), tr("&Copy"), this);
    m_actionCopy->setShortcut(Qt::CTRL + Qt::Key_C);
    m_actionCopy->setStatusTip(tr("Copies the selected widgets to the clipboard"));
    m_actionCopy->setWhatsThis(whatsThisFrom(QLatin1String("Edit|Copy")));
    connect(m_actionCopy, SIGNAL(triggered()), this, SLOT(slotActionCopyActivated()));
    m_actionCopy->setEnabled(false);

    m_actionPaste = new QAction(createIconSet(QLatin1String("editpaste.png")), tr("&Paste"), this);
    m_actionPaste->setShortcut(Qt::CTRL + Qt::Key_V);
    m_actionPaste->setStatusTip(tr("Pastes the clipboard's contents"));
    m_actionPaste->setWhatsThis(whatsThisFrom(QLatin1String("Edit|Paste")));
    connect(m_actionPaste, SIGNAL(triggered()), this, SLOT(slotActionPasteActivated()));
    m_actionPaste->setEnabled(false);

    m_actionDelete = new QAction(tr("&Delete"), this);
    m_actionDelete->setStatusTip(tr("Deletes the selected widgets"));
    m_actionDelete->setWhatsThis(whatsThisFrom(QLatin1String("Edit|Delete")));
    connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(slotActionDeleteActivated()));
    m_actionDelete->setEnabled(false);

    m_actionSelectAll = new QAction(tr("Select &All"), this);
    m_actionSelectAll->setShortcut(Qt::CTRL + Qt::Key_A);
    m_actionSelectAll->setStatusTip(tr("Selects all widgets"));
    m_actionSelectAll->setWhatsThis(whatsThisFrom(QLatin1String("Edit|Select All")));
    connect(m_actionSelectAll, SIGNAL(triggered()), this, SLOT(slotActionSelectAllActivated()));
    m_actionSelectAll->setEnabled(false);

    m_actionRaise = new QAction(createIconSet(QLatin1String("editraise.png")), tr("Bring to &Front"), this);
    m_actionRaise->setShortcut(Qt::CTRL + Qt::Key_L);
    m_actionRaise->setStatusTip(tr("Raises the selected widgets"));
    m_actionRaise->setWhatsThis(tr("Raises the selected widgets"));
    connect(m_actionRaise, SIGNAL(triggered()), this, SLOT(slotActionRaiseActivated()));
    m_actionRaise->setEnabled(false);

    m_actionLower = new QAction(createIconSet(QLatin1String("editlower.png")), tr("Send to &Back"), this);
    m_actionLower->setShortcut(Qt::CTRL + Qt::Key_K);
    m_actionLower->setStatusTip(tr("Lowers the selected widgets"));
    m_actionLower->setWhatsThis(tr("Lowers the selected widgets"));
    connect(m_actionLower, SIGNAL(triggered()), this, SLOT(slotActionLowerActivated()));
    m_actionLower->setEnabled(false);


    m_actionAdjustSize = new QAction(createIconSet(QLatin1String("adjustsize.png")), tr("Adjust &Size"), this);
    m_actionAdjustSize->setShortcut(Qt::CTRL + Qt::Key_J);
    m_actionAdjustSize->setStatusTip(tr("Adjusts the size of the selected widget"));
    m_actionAdjustSize->setWhatsThis(whatsThisFrom(QLatin1String("Layout|Adjust Size")));
    connect(m_actionAdjustSize, SIGNAL(triggered()), this, SLOT(slotActionAdjustSizeActivated()));
    m_actionAdjustSize->setEnabled(false);

    m_actionHorizontalLayout = new QAction(createIconSet(QLatin1String("edithlayout.png")), tr("Lay Out &Horizontally"), this);
    m_actionHorizontalLayout->setShortcut(Qt::CTRL + Qt::Key_1);
    m_actionHorizontalLayout->setStatusTip(tr("Lays out the selected widgets horizontally"));
    m_actionHorizontalLayout->setWhatsThis(whatsThisFrom(QLatin1String("Layout|Lay Out Horizontally")));
    connect(m_actionHorizontalLayout, SIGNAL(triggered()), this, SLOT(slotActionHorizontalLayoutActivated()));
    m_actionHorizontalLayout->setEnabled(false);

    m_actionVerticalLayout = new QAction(createIconSet(QLatin1String("editvlayout.png")), tr("Lay Out &Vertically"), this);
    m_actionVerticalLayout->setShortcut(Qt::CTRL + Qt::Key_2);
    m_actionVerticalLayout->setStatusTip(tr("Lays out the selected widgets vertically"));
    m_actionVerticalLayout->setWhatsThis(whatsThisFrom(QLatin1String("Layout|Lay Out Vertically")));
    connect(m_actionVerticalLayout, SIGNAL(triggered()), this, SLOT(slotActionVerticalLayoutActivated()));
    m_actionVerticalLayout->setEnabled(false);

    m_actionGridLayout = new QAction(createIconSet(QLatin1String("editgrid.png")), tr("Lay Out in a &Grid"), this);
    m_actionGridLayout->setShortcut(Qt::CTRL + Qt::Key_5);
    m_actionGridLayout->setStatusTip(tr("Lays out the selected widgets in a grid"));
    m_actionGridLayout->setWhatsThis(whatsThisFrom(QLatin1String("Layout|Lay Out in a Grid")));
    connect(m_actionGridLayout, SIGNAL(triggered()), this, SLOT(slotActionGridLayoutActivated()));
    m_actionGridLayout->setEnabled(false);

    m_actionSplitHorizontal = new QAction(createIconSet(QLatin1String("editvlayoutsplit.png")),
                                          tr("Lay Out Horizontally in S&plitter"), this);
    m_actionSplitHorizontal->setShortcut(Qt::CTRL + Qt::Key_3);
    m_actionSplitHorizontal->setStatusTip(tr("Lays out the selected widgets horizontally in a splitter"));
    m_actionSplitHorizontal->setWhatsThis(whatsThisFrom(QLatin1String("Layout|Lay Out Horizontally in Splitter")));
    connect(m_actionSplitHorizontal, SIGNAL(triggered()), this, SLOT(slotActionSplitHorizontalActivated()));
    m_actionSplitHorizontal->setEnabled(false);

    m_actionSplitVertical = new QAction(createIconSet(QLatin1String("edithlayoutsplit.png")),
                                        tr("Lay Out Vertically in Sp&litter"), this);
    m_actionSplitVertical->setShortcut(Qt::CTRL + Qt::Key_4);
    m_actionSplitVertical->setStatusTip(tr("Lays out the selected widgets vertically in a splitter"));
    m_actionSplitVertical->setWhatsThis(whatsThisFrom(QLatin1String("Layout|Lay Out Vertically in Splitter")));
    connect(m_actionSplitVertical, SIGNAL(triggered()), this, SLOT(slotActionSplitVerticalActivated()));
    m_actionSplitVertical->setEnabled(false);

    m_actionBreakLayout = new QAction(createIconSet(QLatin1String("editbreaklayout.png")), tr("&Break Layout"), this);
    m_actionBreakLayout->setShortcut(Qt::CTRL + Qt::Key_0);
    m_actionBreakLayout->setStatusTip(tr("Breaks the selected layout"));
    m_actionBreakLayout->setWhatsThis(whatsThisFrom(QLatin1String("Layout|Break Layout")));
    connect(m_actionBreakLayout, SIGNAL(triggered()), this, SLOT(slotActionBreakLayoutActivated()));
    m_actionBreakLayout->setEnabled(false);

    m_undoGroup = new QUndoGroup(this);

    m_actionUndo = m_undoGroup->createUndoAction(this);
    m_actionUndo->setEnabled(false);
    m_actionRedo = m_undoGroup->createRedoAction(this);
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
    m_activeFormWindow->raiseWidgets();
}

void FormWindowManager::slotActionHorizontalLayoutActivated()
{
    if (m_layoutChilds)
        layoutContainerHorizontal();
    else
        m_activeFormWindow->layoutHorizontal();
}

void FormWindowManager::slotActionVerticalLayoutActivated()
{
    if (m_layoutChilds)
        layoutContainerVertical();
    else
        m_activeFormWindow->layoutVertical();
}

void FormWindowManager::slotActionGridLayoutActivated()
{
    if (m_layoutChilds)
        layoutContainerGrid();
    else
        m_activeFormWindow->layoutGrid();
}

void FormWindowManager::slotActionSplitHorizontalActivated()
{
    if (m_layoutChilds)
        ; // no way to do that
    else
        m_activeFormWindow->layoutHorizontalSplit();
}

void FormWindowManager::slotActionSplitVerticalActivated()
{
    if (m_layoutChilds)
        ; // no way to do that
    else
        m_activeFormWindow->layoutVerticalSplit();
}

void FormWindowManager::slotActionBreakLayoutActivated()
{
    const QList<QWidget *> layouts = layoutsToBeBroken();
    if (layouts.isEmpty())
        return;
    
    if (debugFWM) {
        qDebug() << "slotActionBreakLayoutActivated: " << layouts.size();
        foreach (QWidget *w, layouts) {
            qDebug() << w;
        }
    }

    m_activeFormWindow->beginCommand(tr("Break Layout"));
    foreach (QWidget *layout, layouts) {
        m_activeFormWindow->breakLayout(layout);
    }
    m_activeFormWindow->endCommand();
}

void FormWindowManager::slotActionAdjustSizeActivated()
{
    Q_ASSERT(m_activeFormWindow != 0);

    m_activeFormWindow->beginCommand(tr("Adjust Size"));

    QList<QWidget*> selectedWidgets = m_activeFormWindow->selectedWidgets();
    m_activeFormWindow->simplifySelection(&selectedWidgets);

    if (selectedWidgets.isEmpty()) {
        Q_ASSERT(m_activeFormWindow->mainContainer() != 0);
        selectedWidgets.append(m_activeFormWindow->mainContainer());
    }

    foreach (QWidget *widget, selectedWidgets) {
        bool unlaidout = LayoutInfo::layoutType(core(), widget->parentWidget()) == LayoutInfo::NoLayout;
        bool isMainContainer = m_activeFormWindow->isMainContainer(widget);

        if (unlaidout || isMainContainer) {
            AdjustWidgetSizeCommand *cmd = new AdjustWidgetSizeCommand(m_activeFormWindow);
            cmd->init(widget);
            m_activeFormWindow->commandHistory()->push(cmd);
        }
    }

    m_activeFormWindow->endCommand();
}

void FormWindowManager::slotActionSelectAllActivated()
{
    m_activeFormWindow->selectAll();
}


QList<QWidget *> FormWindowManager::layoutsToBeBroken(QWidget *w) const
{
    if (!w)
        return QList<QWidget *>();
    
    if (debugFWM)
        qDebug() << "layoutsToBeBroken: " << w;

    QWidget *parent = w->parentWidget();
    if (m_activeFormWindow->isMainContainer(w))
        parent = 0;

    QWidget *widget = core()->widgetFactory()->containerOfWidget(w);

    // maybe we want to remove following block
    const QDesignerWidgetDataBaseInterface *db = m_core->widgetDataBase();
    const QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(widget));
    if (!item) {
        if (debugFWM)
            qDebug() << "layoutsToBeBroken: Don't have an item, recursing for parent";
        return layoutsToBeBroken(parent);
    }

    const bool layoutContainer = (item->isContainer() || m_activeFormWindow->isMainContainer(widget));

    if (!layoutContainer) {
        if (debugFWM)
            qDebug() << "layoutsToBeBroken: Not a container, recursing for parent";
        return layoutsToBeBroken(parent);
    }

    QLayout *widgetLayout = widget->layout();
    QLayout *managedLayout = LayoutInfo::managedLayout(m_core, widgetLayout);
    if (widgetLayout && !managedLayout) {
        if (qobject_cast<const QSplitter *>(widget)) {
            if (debugFWM)
                qDebug() << "layoutsToBeBroken: Splitter special";
            QList<QWidget *> list = layoutsToBeBroken(parent);
            list.append(widget);
            return list;
        }
        if (debugFWM)
            qDebug() << "layoutsToBeBroken: Is a container but doesn't have a managed layout (has an internal layout), returning 0";
        return QList<QWidget *>();
    }

    if (managedLayout) {
        QList<QWidget *> list;
        if (debugFWM)
            qDebug() << "layoutsToBeBroken: Is a container and has a layout";
        if (qobject_cast<const QLayoutWidget *>(widget)) {
            if (debugFWM) 
                qDebug() << "layoutsToBeBroken: red layout special case";
            list = layoutsToBeBroken(parent);
        }
        list.append(widget);
        return list;
    }
    if (debugFWM)
        qDebug() << "layoutsToBeBroken: Is a container but doesn't have a layout at all, returning 0";
    return QList<QWidget *>();

}
    
QMap<QWidget *, bool> FormWindowManager::getUnsortedLayoutsToBeBroken(bool firstOnly) const
{
    // Return a set of layouts to be broken.
    QMap<QWidget *, bool> layouts;

    QList<QWidget *> selection = m_activeFormWindow->selectedWidgets();
    if (selection.isEmpty() && m_activeFormWindow->mainContainer())
        selection.append(m_activeFormWindow->mainContainer());
        
    const QList<QWidget *>::const_iterator scend = selection.constEnd();
    for (QList<QWidget *>::const_iterator sit = selection.constBegin(); sit != scend; ++sit) {
        // find all layouts
        const QList<QWidget *> list = layoutsToBeBroken(*sit);
        if (!list.empty()) {
            const QList<QWidget *>::const_iterator lbcend = list.constEnd();
            for (QList<QWidget *>::const_iterator lbit = list.constBegin(); lbit != lbcend; ++lbit) {
                layouts.insert(*lbit, true);
            }
            if (firstOnly)
                return layouts;
        }
    }
    return layouts;
}

bool FormWindowManager::hasLayoutsToBeBroken() const
{
    // Quick check for layouts to be broken
    return !getUnsortedLayoutsToBeBroken(true).isEmpty();
}

QList<QWidget *> FormWindowManager::layoutsToBeBroken() const
{
    // Get all layouts
    
    QMap<QWidget *, bool> unsortedLayouts = getUnsortedLayoutsToBeBroken(false);
    // Sort in order of hierarchy
    QList<QWidget *> orderedLayoutList;
    const QMap<QWidget *, bool>::const_iterator lscend  = unsortedLayouts.constEnd();
    for (QMap<QWidget *, bool>::const_iterator itLay = unsortedLayouts.constBegin(); itLay != lscend; ++itLay) {
        QWidget *wToBeInserted = itLay.key();
        if (!orderedLayoutList.contains(wToBeInserted)) {
            // try to find first child, use as insertion position, else append
            const QList<QWidget *>::iterator firstChildPos = findFirstChildOf(orderedLayoutList.begin(), orderedLayoutList.end(), wToBeInserted);
            if (firstChildPos == orderedLayoutList.end()) {
                orderedLayoutList.push_back(wToBeInserted);
            } else {
                orderedLayoutList.insert(firstChildPos, wToBeInserted);
            }
        }
    }
    return orderedLayoutList;
}

void FormWindowManager::slotUpdateActions()
{
    m_layoutChilds = false;

    int selectedWidgetCount = 0;
    int laidoutWidgetCount = 0;
    int unlaidoutWidgetCount = 0;
    bool pasteAvailable = false;
    bool layoutAvailable = false;
    bool breakAvailable = false;
    bool layoutContainer = false;

    do {
        if (m_activeFormWindow == 0 || m_activeFormWindow->currentTool() != 0)
            break;

        breakAvailable = hasLayoutsToBeBroken();

        QList<QWidget*> simplifiedSelection = m_activeFormWindow->selectedWidgets();

        selectedWidgetCount = simplifiedSelection.count();
        pasteAvailable = qApp->clipboard()->mimeData() && qApp->clipboard()->mimeData()->hasText();

        m_activeFormWindow->simplifySelection(&simplifiedSelection);
        if (simplifiedSelection.isEmpty() && m_activeFormWindow->mainContainer())
            simplifiedSelection.append(m_activeFormWindow->mainContainer());

        foreach (QWidget *widget, simplifiedSelection) {
            if (LayoutInfo::isWidgetLaidout(m_core, widget))
                ++laidoutWidgetCount;
            else
                ++unlaidoutWidgetCount;
        }

        // Figure out layouts: Looking at a group of dangling widgets
        if (simplifiedSelection.count() != 1) {
            layoutAvailable = unlaidoutWidgetCount > 1;
            //breakAvailable = false;
            break;
        }
        // Manipulate layout of a single widget
        m_layoutChilds = false;
        QWidget *widget = core()->widgetFactory()->containerOfWidget(simplifiedSelection.first());

        const QDesignerWidgetDataBaseInterface *db = m_core->widgetDataBase();
        const QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(widget));
        if (!item)
            break;

        QLayout *widgetLayout = LayoutInfo::internalLayout(widget);
        QLayout *managedLayout = LayoutInfo::managedLayout(m_core, widgetLayout);
        // We don't touch a layout createds by a custom widget
        if (widgetLayout && !managedLayout)
            break;

        layoutContainer = (item->isContainer() || m_activeFormWindow->isMainContainer(widget));

        layoutAvailable = layoutContainer && m_activeFormWindow->hasInsertedChildren(widget) && managedLayout == 0;
        m_layoutChilds = layoutAvailable;

    } while(false);

    m_actionCut->setEnabled(selectedWidgetCount > 0);
    m_actionCopy->setEnabled(selectedWidgetCount > 0);
    m_actionDelete->setEnabled(selectedWidgetCount > 0);
    m_actionLower->setEnabled(selectedWidgetCount > 0);
    m_actionRaise->setEnabled(selectedWidgetCount > 0);

    m_actionPaste->setEnabled(pasteAvailable);

    m_actionSelectAll->setEnabled(m_activeFormWindow != 0);

    m_actionAdjustSize->setEnabled(unlaidoutWidgetCount > 0);

    m_actionHorizontalLayout->setEnabled(layoutAvailable);
    m_actionVerticalLayout->setEnabled(layoutAvailable);
    m_actionSplitHorizontal->setEnabled(layoutAvailable && !layoutContainer);
    m_actionSplitVertical->setEnabled(layoutAvailable && !layoutContainer);
    m_actionGridLayout->setEnabled(layoutAvailable);

    m_actionBreakLayout->setEnabled(breakAvailable);
}

void FormWindowManager::layoutContainerHorizontal()
{
    QWidget *w = m_activeFormWindow->mainContainer();
    QList<QWidget*> l(m_activeFormWindow->selectedWidgets());
    m_activeFormWindow->simplifySelection(&l);
    if (l.count() > 0)
        w = l.first();

    if (w != 0)
        m_activeFormWindow->layoutHorizontalContainer(w);
}

void FormWindowManager::layoutContainerVertical()
{
    QWidget *w = m_activeFormWindow->mainContainer();
    QList<QWidget*> l(m_activeFormWindow->selectedWidgets());
    m_activeFormWindow->simplifySelection(&l);
    if (l.count() > 0)
        w = l.first();

    if (w)
        m_activeFormWindow->layoutVerticalContainer(w);
}

void FormWindowManager::layoutContainerGrid()
{
    QWidget *w = m_activeFormWindow->mainContainer();
    QList<QWidget*> l(m_activeFormWindow->selectedWidgets());
    m_activeFormWindow->simplifySelection(&l);
    if (l.count() > 0)
        w = l.first();

    if (w)
        m_activeFormWindow->layoutGridContainer(w);
}

QDesignerFormWindowInterface *FormWindowManager::createFormWindow(QWidget *parentWidget, Qt::WindowFlags flags)
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

void FormWindowManager::dragItems(const QList<QDesignerDnDItemInterface*> &item_list)
{
    if (!m_drag_item_list.isEmpty()) {
        qWarning("FormWindowManager::dragItem(): called while already dragging");
        return;
    }

    beginDrag(item_list, QCursor::pos());
}

void FormWindowManager::beginDrag(const QList<QDesignerDnDItemInterface*> &item_list, const QPoint &globalPos)
{
    Q_ASSERT(m_drag_item_list.isEmpty());

    m_drag_item_list = item_list;

    setItemsPos(globalPos);

    foreach(QDesignerDnDItemInterface *item, m_drag_item_list) {
        QWidget *deco = item->decoration();
        deco->setAttribute(Qt::WA_TransparentForMouseEvents);
        QPoint pos = deco->pos();
        QRect ag = qApp->desktop()->availableGeometry(deco);
        deco->move(qMin(qMax(pos.x(), ag.left()), ag.right()), qMin(qMax(pos.y(), ag.top()), ag.bottom()));
        deco->move(pos);
        deco->show();
        deco->setWindowOpacity(0.8);
    }

#ifdef Q_WS_X11
    m_core->topLevel()->grabMouse();
    m_savedContextMenuPolicy = m_core->topLevel()->contextMenuPolicy();
    m_core->topLevel()->setContextMenuPolicy(Qt::NoContextMenu);
#endif
}

static QDesignerWidgetBoxInterface *widgetBoxAt(const QPoint &global_pos)
{
    QWidget *w = qApp->widgetAt(global_pos);
    while (w != 0) {
        QDesignerWidgetBoxInterface *wb = qobject_cast<QDesignerWidgetBoxInterface*>(w);
        if (wb != 0)
            return wb;
        w = w->parentWidget();
    }
    return 0;
}

void FormWindowManager::setItemsPos(const QPoint &globalPos)
{
    foreach(QDesignerDnDItemInterface *item, m_drag_item_list)
        item->decoration()->move(globalPos - item->hotSpot());

    QWidget *widget_under_mouse = qApp->widgetAt(globalPos);
    int max_try = 3;
    while (max_try && widget_under_mouse && isDecoration(widget_under_mouse)) {
        --max_try;
        widget_under_mouse = qApp->widgetAt(widget_under_mouse->pos() - QPoint(1,1));
        Q_ASSERT(!qobject_cast<ConnectionEdit*>(widget_under_mouse));
    }

    FormWindow *form_under_mouse
            = qobject_cast<FormWindow*>(QDesignerFormWindowInterface::findFormWindow(widget_under_mouse));
    if (form_under_mouse != 0 && !form_under_mouse->hasFeature(QDesignerFormWindowInterface::EditFeature))
        form_under_mouse = 0;
    if (form_under_mouse != 0) {
        // widget_under_mouse might be some temporary thing like the dropLine. We need
        // the actual widget that's part of the edited GUI.
        widget_under_mouse
            = form_under_mouse->widgetAt(form_under_mouse->mapFromGlobal(globalPos));

        if (QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), form_under_mouse->findContainer(widget_under_mouse, false))) {
            widget_under_mouse = c->widget(c->currentIndex());
        }
        Q_ASSERT(!qobject_cast<ConnectionEdit*>(widget_under_mouse));
    }

    if (m_last_form_under_mouse != 0 && widget_under_mouse != m_last_widget_under_mouse) {
        m_last_form_under_mouse->highlightWidget(m_last_widget_under_mouse,
                                    m_last_widget_under_mouse->mapFromGlobal(globalPos),
                                    FormWindow::Restore);
    }

    FormWindow *source_form = qobject_cast<FormWindow*>(m_drag_item_list.first()->source());
    if (form_under_mouse != 0
        && (source_form == 0 || widget_under_mouse != source_form->mainContainer())) {

        form_under_mouse->highlightWidget(widget_under_mouse,
                                    widget_under_mouse->mapFromGlobal(globalPos),
                                    FormWindow::Highlight);
    }

    m_last_widget_under_mouse = widget_under_mouse;
    m_last_form_under_mouse = form_under_mouse;
    if (m_last_form_under_mouse == 0)
        m_widget_box_under_mouse = widgetBoxAt(globalPos);
    else
        m_widget_box_under_mouse = 0;
}

void FormWindowManager::endDrag(const QPoint &pos)
{
#ifdef Q_WS_X11
    m_core->topLevel()->releaseMouse();
    m_core->topLevel()->setContextMenuPolicy(m_savedContextMenuPolicy);
#endif

    Q_ASSERT(!m_drag_item_list.isEmpty());

    foreach (QDesignerDnDItemInterface *item, m_drag_item_list)
        item->decoration()->hide();

    // ugly, but you can't qobject_cast from interfaces
    if (m_last_form_under_mouse != 0 &&
            m_last_form_under_mouse->hasFeature(QDesignerFormWindowInterface::EditFeature)) {
        m_last_form_under_mouse->dropWidgets(m_drag_item_list, m_last_widget_under_mouse, pos);
    } else if (m_widget_box_under_mouse != 0) {
        m_widget_box_under_mouse->dropWidgets(m_drag_item_list, pos);
        foreach (QDesignerDnDItemInterface *item, m_drag_item_list) {
            if (item->type() == QDesignerDnDItemInterface::CopyDrop)
                continue;
            FormWindow *source = qobject_cast<FormWindow*>(item->source());
            if (source == 0)
                continue;
            QWidget *widget = item->widget();
            if (widget == 0)
                continue;
            source->deleteWidgets(QList<QWidget*>() << widget);
        }
    } else {
        foreach (QDesignerDnDItemInterface *item, m_drag_item_list) {
            if (item->widget() != 0)
                item->widget()->show();
        }
    }

    foreach (QDesignerDnDItemInterface *item, m_drag_item_list)
        delete item;

    m_drag_item_list.clear();
    m_last_widget_under_mouse = 0;
    m_last_form_under_mouse = 0;
    m_widget_box_under_mouse = 0;
}

bool FormWindowManager::isDecoration(QWidget *widget) const
{
    foreach (QDesignerDnDItemInterface *item, m_drag_item_list) {
        if (item->decoration() == widget)
            return true;
    }

    return false;
}

QUndoGroup *FormWindowManager::undoGroup() const
{
    return m_undoGroup;
}
}

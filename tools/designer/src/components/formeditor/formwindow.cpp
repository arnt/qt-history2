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

#include "formwindow.h"
#include "formwindow_dnditem.h"
#include "formwindow_widgetstack.h"
#include "formwindowcursor.h"
#include "formwindowmanager.h"
#include "tool_widgeteditor.h"

#include "orderindicator.h"
#include "widgetselection.h"
#include "qdesigner_tabwidget.h"
#include "qdesigner_toolbox.h"
#include "qdesigner_stackedbox.h"
#include "qdesigner_resource.h"

// shared
#include <qdesigner_promotedwidget.h>
#include <qdesigner_command.h>
#include <spacer_widget.h>
#include <layoutinfo.h>
#include <layoutdecoration.h>
#include <qdesigner_widget.h>
#include <invisible_widget.h>
#include <connectionedit.h>

// sdk
#include <abstractformeditor.h>
#include <abstractformwindowtool.h>
#include <abstractwidgetfactory.h>
#include <abstractwidgetdatabase.h>
#include <ui4.h>

#include <container.h>
#include <propertysheet.h>
#include <qextensionmanager.h>

#include <QtGui/QtGui>

class FriendlyWidget: public QWidget
{
public:
    FriendlyWidget() { Q_ASSERT(0); }

    friend class FormWindow;
};

class DropLine : public QWidget
{
    Q_OBJECT
public:
    DropLine(QWidget *parent);
};

DropLine::DropLine(QWidget *parent)
    : QWidget(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Background, Qt::red);
    setPalette(p);
}


FormWindow::FormWindow(FormEditor *core, QWidget *parent, Qt::WFlags flags)
    : AbstractFormWindow(parent, flags),
      m_core(core), m_widgetStack(0)
{
    init();

    m_cursor = new FormWindowCursor(this, this);

    core->formWindowManager()->addFormWindow(this);

    setDirty(false);
}

FormWindow::~FormWindow()
{
    hideOrderIndicators();

    core()->formWindowManager()->removeFormWindow(this);
    core()->metaDataBase()->remove(this);

    QList<QWidget*> l = widgets();
    foreach (QWidget *w, l)
        core()->metaDataBase()->remove(w);

    m_widgetStack = 0;
    m_rubberBand = 0;

    qDeleteAll(selections);
}

AbstractFormEditor *FormWindow::core() const
{
    return m_core;
}

AbstractFormWindowCursor *FormWindow::cursor() const
{
    return m_cursor;
}

bool FormWindow::hasFeature(Feature f) const
{
    if (f == EditFeature)
        return f & m_feature;
    return f & m_feature;
}

int FormWindow::widgetDepth(QWidget *w)
{
    int d = -1;
    while (w && !w->isWindow()) {
        d++;
        w = w->parentWidget();
    }

    return d;
}

bool FormWindow::isChildOf(QWidget *c, const QWidget *p)
{
    while (c /*&& !c->isWindow()*/) {
        if (c == p)
            return true;
        c = c->parentWidget();
    }
    return false;
}

FormWindow::Feature FormWindow::features() const
{
    return m_feature;
}

static void recursiveUpdate(QWidget *w)
{
    w->update();

    const QObjectList &l = w->children();
    QObjectList::const_iterator it = l.begin();
    for (; it != l.end(); ++it) {
        if (QWidget *w = qobject_cast<QWidget*>(*it))
            recursiveUpdate(w);
    }
}

void FormWindow::setFeatures(Feature f)
{
    m_feature = f;
    emit featureChanged(f);
    recursiveUpdate(this);
}

void FormWindow::setCursorToAll(const QCursor &c, QWidget *start)
{
    start->setCursor(c);
    QList<QWidget*> l = qFindChildren<QWidget*>(start);
    QListIterator<QWidget*> it(l);
    while (it.hasNext()) {
        QWidget *w = it.next();
        if (!qobject_cast<WidgetHandle*>(w)) {
            w->setCursor(c);
        }
    }
}

void FormWindow::restoreCursors(QWidget *start, FormWindow *fw)
{
    AbstractMetaDataBaseItem *item = core()->metaDataBase()->item(start);
    if (fw->isManaged(start))
        start->setCursor(item->cursor());
    else
        start->setCursor(Qt::ArrowCursor);

    QList<QWidget*> l = qFindChildren<QWidget*>(start);
    QListIterator<QWidget*> it(l);
    while (it.hasNext()) {
        QWidget *w = it.next();
        if (!qobject_cast<WidgetHandle*>(w))
            restoreCursors(w, fw);
    }
}

void FormWindow::init()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    m_widgetStack = new FormWindowWidgetStack(this);
    connect(m_widgetStack, SIGNAL(currentToolChanged(int)), this, SIGNAL(toolChanged(int)));
    layout->addWidget(m_widgetStack);

    m_feature = DefaultFeature;

    m_selectionChangedTimer = new QTimer(this);
    m_selectionChangedTimer->setSingleShot(true);
    connect(m_selectionChangedTimer, SIGNAL(timeout()), this, SLOT(selectionChangedTimerDone()));

    m_checkSelectionTimer = new QTimer(this);
    m_checkSelectionTimer->setSingleShot(true);
    connect(m_checkSelectionTimer, SIGNAL(timeout()), this, SLOT(checkSelectionNow()));

    m_geometryChangedTimer = new QTimer(this);
    m_geometryChangedTimer->setSingleShot(true);
    connect(m_geometryChangedTimer, SIGNAL(timeout()), this, SIGNAL(geometryChanged()));

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_rubberBand->hide();

    setGrid(QPoint(10,10));

    setFocusPolicy(Qt::StrongFocus);

    m_mainContainer = 0;
    m_currentWidget = 0;
    oldRectValid = false;
    drawRubber = false;
    checkedSelectionsForMove = false;

    targetContainer = 0;
    hadOwnPalette = false;
    startWidget = 0;
    endWidget = 0;

    m_commandHistory = new QtUndoStack(this);
    connect(m_commandHistory, SIGNAL(commandExecuted()), this, SLOT(updateDirty()));
    connect(m_commandHistory, SIGNAL(commandExecuted()), this, SIGNAL(changed()));
    connect(commandHistory(), SIGNAL(commandExecuted()), this, SLOT(checkSelection()));

    core()->metaDataBase()->add(this);

    initializeCoreTools();
}

QWidget *FormWindow::mainContainer() const
{
    return m_mainContainer;
}

void FormWindow::setMainContainer(QWidget *w)
{
    if (w == m_mainContainer) {
        // nothing to do
        return;
    }

    if (m_mainContainer) {
        unmanageWidget(m_mainContainer);
        delete m_mainContainer;
        m_mainContainer = 0;
    }

    m_mainContainer = w;

    m_mainContainer->setParent(m_widgetStack, 0);
    m_mainContainer->raise();
    m_mainContainer->show();

    m_widgetStack->setCurrentTool(m_widgetEditor);

    setCurrentWidget(m_mainContainer);

    manageWidget(m_mainContainer);

    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(core()->extensionManager(), m_mainContainer)) {
        sheet->setVisible(sheet->indexOf("windowTitle"), true);
        // ### generalize
    }

    emit mainContainerChanged(m_mainContainer);
}

void FormWindow::handlePaintEvent(QWidget *w, QPaintEvent *e)
{
    static_cast<FriendlyWidget*>(w)->paintEvent(e);
    QAbstractButton *btn = qobject_cast<QAbstractButton*>(w);
    if (btn && qobject_cast<QGroupBox*>(w->parentWidget()) != 0) {
        QPainter p(btn);
        p.setBrush(Qt::red);
        p.drawEllipse(btn->width() - 12, 2, 10, 5);
    }
}

QWidget *FormWindow::findTargetContainer(QWidget *widget) const
{
    Q_ASSERT(widget);

    while (QWidget *parentWidget = widget->parentWidget()) {
        if (LayoutInfo::layoutType(m_core, parentWidget) == LayoutInfo::NoLayout && isManaged(widget))
            return widget;

        widget = parentWidget;
    }

    return mainContainer();
}

void FormWindow::handleMousePressEvent(QWidget *w, QMouseEvent *e)
{
    checkedSelectionsForMove = false;

        // if the dragged widget is not in a layout, raise it
    if (!w->parentWidget() || LayoutInfo::layoutType(m_core, w->parentWidget()) == LayoutInfo::NoLayout)
        w->raise();

    if (isMainContainer(w)) { // press was on the formwindow
        if (e->button() == Qt::LeftButton) { // left button: start rubber selection and show formwindow properties
            drawRubber = true;
            if (!(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
                clearSelection(false);
                QWidget *opw = m_currentWidget;
                setCurrentWidget(mainContainer());
                if (opw)
                    repaintSelection(opw);
            }
            currRect = QRect(0, 0, -1, -1);
            startRectDraw(e->globalPos(), this, Rubber);
        }
    } else {
        startPos = mapFromGlobal(e->globalPos());
        bool sel = isWidgetSelected(w);

        if (e->button() == Qt::LeftButton) {
            if (e->modifiers() & Qt::ShiftModifier) {
                // shift-click - toggle selection state of widget
                selectWidget(w, !sel);
            } else {
                if (!sel)
                    clearSelection(false);

                raiseChildSelections(w);
                selectWidget(w);
            }
        }
    }

/*
    case TabOrderEditMode:
        if (!isMainContainer(w)) { // press on a child widget

            if (OrderIndicator *i = qobject_cast<OrderIndicator*>(w)) {
                w = i->widget();
            }

            int idx = orderedWidgets.indexOf(w);
            orderedWidgets.removeAt(idx);
            orderedWidgets.append(w);

            QListIterator<QWidget*> it(orderedWidgets);
            it.toBack();
            while (it.hasPrevious()) {
                QWidget *wid = it.previous();
                int j = stackedWidgets.indexOf(wid);
                if (j > 0) {
                    stackedWidgets.removeAt(j);
                    stackedWidgets.insert(0, wid);
                }
            }

            TabOrderCommand *cmd = new TabOrderCommand(this);
            cmd->init(stackedWidgets);
            commandHistory()->push(cmd);
        }
        break;
*/
}

void FormWindow::handleMouseMoveEvent(QWidget *w, QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) != Qt::LeftButton)
        return;

    QPoint pos = mapFromGlobal(e->globalPos());

    if (drawRubber) { // draw rubber if we are in rubber-selection mode
        continueRectDraw(e->globalPos(), this, Rubber);
    } else if ((e->modifiers() == 0 || e->modifiers() & Qt::ControlModifier)
            && (startPos - pos).manhattanLength() > QApplication::startDragDistance()){

        // if widget is laid out, find the first non-laid out super-widget
        QWidget *c = w;
        while ( c->parentWidget() &&
            ( LayoutInfo::layoutType(m_core, c->parentWidget() ) != LayoutInfo::NoLayout || !isManaged(c) ) )
                c = c->parentWidget();
        selectWidget(c);

        QDesignerResource res(this);

        QList<QWidget*> sel = checkSelectionsForMove(w);

        QList<AbstractDnDItem*> item_list;
        foreach (QWidget *widget, sel) {
            QWidget *container = findTargetContainer(widget);
            if (container && LayoutInfo::layoutType(core(), container) != LayoutInfo::NoLayout) {
                widget = container;
                selectWidget(widget, true);
            }
            if (e->modifiers() & Qt::ControlModifier) {
                QDesignerResource builder(this);
                DomUI *dom_ui = builder.copy(QList<QWidget*>() << widget);
                item_list.append(new FormWindowDnDItem(dom_ui, widget, e->globalPos()));
            } else {
                item_list.append(new FormWindowDnDItem(widget, e->globalPos()));
                widget->hide();
            }
        }

        if (sel.count())
            core()->formWindowManager()->dragItems(item_list, this);
    }
}

void FormWindow::handleMouseReleaseEvent(QWidget * /*w*/, QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    if (drawRubber) { // we were drawing a rubber selection
        endRectDraw(); // get rid of the rectangle
        bool block = blockSignals(true);
        selectWidgets(); // select widgets which intersect the rect
        blockSignals(block);
        emitSelectionChanged(); // inform about selection changes
    }

    drawRubber = false;
}

void FormWindow::checkPreviewGeometry(QRect &r)
{
    if (!rect().contains(r)) {
        if (r.left() < rect().left())
            r.moveTopLeft(QPoint(0, r.top()));
        if (r.right() > rect().right())
            r.moveBottomRight(QPoint(rect().right(), r.bottom()));
        if (r.top() < rect().top())
            r.moveTopLeft(QPoint(r.left(), rect().top()));
        if (r.bottom() > rect().bottom())
            r.moveBottomRight(QPoint(r.right(), rect().bottom()));
    }
}

void FormWindow::startRectDraw(const QPoint &pos, QWidget *, RectType t)
{
    oldRectValid = false;

    rectAnchor = (t == Insert) ? gridPoint(pos) : pos;

    currRect = QRect(rectAnchor, QPoint(0, 0));
    m_rubberBand->setGeometry(currRect);
    m_rubberBand->show();
}

void FormWindow::continueRectDraw(const QPoint &pos, QWidget *, RectType t)
{
    QPoint p2 = (t == Insert) ? gridPoint(pos) : pos;

    QRect r(rectAnchor, p2);
    r = r.normalize();

    if (currRect == r)
        return;

    if (r.width() > 1 || r.height() > 1) {
        oldRectValid = true;
        currRect = r;
        m_rubberBand->setGeometry(currRect);
    } else {
        oldRectValid = false;
    }
}

void FormWindow::endRectDraw()
{
    if (!m_rubberBand->isVisible())
        return;

    m_rubberBand->hide();
}

QPoint FormWindow::gridPoint(const QPoint &p) const
{
    return QPoint(((p.x() + grid().x()/2) / grid().x()) * grid().x(),
                   ((p.y() + grid().y()/2) / grid().y()) * grid().y());
}

QWidget *FormWindow::currentWidget() const
{
    return m_currentWidget;
}

void FormWindow::setCurrentWidget(QWidget *currentWidget)
{
    m_currentWidget = currentWidget;
}

void FormWindow::selectWidget(QWidget* w, bool select)
{
    if (!isManaged(w))
        return;

    if (isMainContainer(w)) {
        QWidget *opw = m_currentWidget;
        setCurrentWidget(mainContainer());
        repaintSelection(opw);
        return;
    }

    if (qobject_cast<QMainWindow*>(mainContainer()) && w == static_cast<QMainWindow*>(mainContainer())->centralWidget()) {
        QWidget *opw = m_currentWidget;
        setCurrentWidget(mainContainer());
        repaintSelection(opw);
        return;
    }

    if (select) {
        QWidget *opw = m_currentWidget;
        setCurrentWidget(w);
        repaintSelection(opw);
        WidgetSelection *s = usedSelections.value(w);
        if (s) {
            s->show();
            return;
        }

        QListIterator<WidgetSelection*> it(selections);
        while (it.hasNext()) {
            WidgetSelection *sel = it.next();
            if (!sel->isUsed())
                s = sel;
        }

        if (!s) {
            s = new WidgetSelection(this, &usedSelections);
            selections.append(s);
        }

        s->setWidget(w);
        emitSelectionChanged();
    } else {
        WidgetSelection *s = usedSelections.value(w);
        if (s)
            s->setWidget(0);
        QWidget *opw = m_currentWidget;
        if (!usedSelections.isEmpty())
            setCurrentWidget((*usedSelections.begin())->widget());
        else
            setCurrentWidget(mainContainer());
        repaintSelection(opw);
        emitSelectionChanged();
    }
}

void FormWindow::hideSelection(QWidget *w)
{
    selectWidget(w, false);
    m_selectionChangedTimer->stop();
}

void FormWindow::clearSelection(bool changePropertyDisplay)
{
    for (QHash<QWidget *, WidgetSelection *>::Iterator it = usedSelections.begin(); it != usedSelections.end(); ++it) {
        it.value()->setWidget(0, false);
    }

    usedSelections.clear();
    if (changePropertyDisplay) {
        setCurrentWidget(mainContainer());
        if (m_currentWidget) {
            repaintSelection(m_currentWidget);
        }
    }

    emitSelectionChanged();
}

void FormWindow::emitSelectionChanged()
{
    m_selectionChangedTimer->setSingleShot(true);
    m_selectionChangedTimer->start(0);
}

void FormWindow::selectionChangedTimerDone()
{
    emit selectionChanged();
}

void FormWindow::repaintSelection(QWidget *w)
{
    if (WidgetSelection *s = usedSelections.value(w))
        s->update();
}

bool FormWindow::isWidgetSelected(QWidget *w) const
{
    return usedSelections.contains(w);
}

bool FormWindow::isMainContainer(const QWidget *w) const
{
    return w && (w == this || w == mainContainer());
}

void FormWindow::updateChildSelections(QWidget *w)
{
    QList<QWidget*> l = qFindChildren<QWidget*>(w);

    QListIterator<QWidget*> it(l);
    while (it.hasNext()) {
        QWidget *w = it.next();
        if (isManaged(w)) {
            updateSelection(w);
        }
    }
}

void FormWindow::updateSelection(QWidget *w)
{
    WidgetSelection *s = usedSelections.value(w);
    if (!w->isVisibleTo(this)) {
        selectWidget(w, false);
    } else if (s)
        s->updateGeometry();
}

QWidget *FormWindow::designerWidget(QWidget *w) const
{
    while (w && !isMainContainer(w) && !isManaged(w) || isCentralWidget(w))
        w = w->parentWidget();

    return w;
}

bool FormWindow::isCentralWidget(QWidget *w) const
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(mainContainer()))
        return w == mainWindow->centralWidget();

    return false;
}

bool FormWindow::unify(QObject *w, QString &s, bool changeIt)
{
    bool found = !isMainContainer(static_cast<QWidget*>(w)) && objectName() == s;
    if (!found) {
        QString orig = s;
        int num = 1;

        QListIterator<QWidget*> it(m_widgets);
        while (it.hasNext()) {
            QWidget *child = it.next();

            if (child != w && child->objectName() == s) {
                found = true;
                if (!changeIt)
                    break;
                s = orig + QLatin1String("_") + QString::number(++num);
                it.toFront();
            }
        }

        if (qobject_cast<QMainWindow*>(mainContainer())) {
            if (!found) {
                QList<QDockWidget*> l = qFindChildren<QDockWidget*>(mainContainer());
                QListIterator<QDockWidget*> it(l);
                while (it.hasNext()) {
                    QDockWidget* o = it.next();
                    if (o != w && o->objectName() == s) {
                        found = true;
                        if (!changeIt)
                            break;
                        s = orig + QLatin1Char('_') + QString::number(++num);
                        o = l.first();
                    }
                }
            }
        }
    }

    return !found;
}

void FormWindow::insertWidget(QWidget *w, const QRect &rect, QWidget *target)
{
    QWidget *container = findContainer(target, false);
    if (!container)
        return;

    clearSelection(false);

    beginCommand(tr("Insert widget '%1").arg(w->metaObject()->className())); // ### use the WidgetDatabaseItem

    if (w->parentWidget() != container) {
        ReparentWidgetCommand *cmd = new ReparentWidgetCommand(this);
        cmd->init(w, container);
        m_commandHistory->push(cmd);
    }

    QRect r = rect;
    Q_ASSERT(r.isValid());
    r.moveTopLeft(gridPoint(container->mapFromGlobal(r.topLeft())));

    SetPropertyCommand *geom_cmd = new SetPropertyCommand(this);
    geom_cmd->init(w, "geometry", r); // ### use rc.size()
    m_commandHistory->push(geom_cmd);

    InsertWidgetCommand *cmd = new InsertWidgetCommand(this);
    cmd->init(w);
    m_commandHistory->push(cmd);

    endCommand();

    w->show();

    if (container && container->layout()) {
        recursiveUpdate(container);
        container->layout()->invalidate();
    }
}

QWidget *FormWindow::createWidget(DomUI *ui, const QRect &rc, QWidget *target)
{
    QWidget *container = findContainer(target, false);
    if (!container)
        return 0;

    QDesignerResource resource(this);
    QList<QWidget*> widgets = resource.paste(ui, container);
    Q_ASSERT(widgets.size() == 1); // multiple-paste from DomUI not supported yet

    insertWidget(widgets.first(), rc, container);
    return widgets.first();
}

static bool isDescendant(const QWidget *parent, const QWidget *child)
{
    for (; child != 0; child = child->parentWidget()) {
        if (child == parent)
            return true;
    }
    return false;
}

void FormWindow::resizeWidget(QWidget *widget, const QRect &geometry)
{
    Q_ASSERT(isDescendant(this, widget));

    QRect r = geometry;
    r.moveTopLeft(gridPoint(geometry.topLeft()));
    SetPropertyCommand *cmd = new SetPropertyCommand(this);
    cmd->init(widget, "geometry", r);
    cmd->setDescription(tr("Resize"));
    m_commandHistory->push(cmd);
}

void FormWindow::invalidCheckedSelections()
{
    checkedSelectionsForMove = false;
}

void FormWindow::raiseChildSelections(QWidget *w)
{
    QList<QWidget*> l = qFindChildren<QWidget*>(w);
    if (l.isEmpty())
        return;

    QHashIterator<QWidget *, WidgetSelection *> it(usedSelections);
    while (it.hasNext()) {
        it.next();

        WidgetSelection *w = it.value();
        if (l.contains(w->widget()))
            w->show();
    }
}

bool FormWindow::allowMove(QWidget *w)
{
    if (!hasFeature(EditFeature))
        return false;

    w = w->parentWidget();
    while (w) {
        bool valid = isMainContainer(w) || isManaged(w);

        if (valid && LayoutInfo::layoutType(m_core, w) == LayoutInfo::NoLayout)
            return true;

        w = w->parentWidget();
    }

    return false;
}

QWidget *FormWindow::containerAt(const QPoint &pos, QWidget *notParentOf)
{
    QWidget *container = 0;
    int depth = -1;
    QList<QWidget*> selected = selectedWidgets();
    if (rect().contains(mapFromGlobal(pos))) {
        container = mainContainer();
        depth = widgetDepth(container);
    }

    QListIterator<QWidget*> it(m_widgets);
    while (it.hasNext()) {
        QWidget *wit = it.next();
        if (qobject_cast<QLayoutWidget*>(wit) || qobject_cast<QSplitter*>(wit))
            continue;
        if (!wit->isVisibleTo(this))
            continue;
        if (selected.indexOf(wit) != -1)
            continue;
        if (!core()->widgetDataBase()->isContainer(wit) &&
             wit != mainContainer())
            continue;

        // the rectangles of all ancestors of the container must contain the insert position
        QWidget *w = wit;
        while (w && !w->isWindow()) {
            if (!w->rect().contains((w->mapFromGlobal(pos))))
                break;
            w = w->parentWidget();
        }
        if (!(w == 0 || w->isWindow()))
            continue; // we did not get through the full while loop

        int wd = widgetDepth(wit);
        if (wd == depth && container) {
            if (wit->parentWidget()->children().indexOf(wit) >
                 container->parentWidget()->children().indexOf(container))
                wd++;
        }
        if (wd > depth && !isChildOf(wit, notParentOf)) {
            depth = wd;
            container = wit;
        }
    }
    return container;
}

QList<QWidget*> FormWindow::checkSelectionsForMove(QWidget *w)
{
    // if widget is laid out, find the first non-laid out super-widget
    while ( w->parentWidget() &&
                ( LayoutInfo::layoutType(m_core, w->parentWidget() ) != LayoutInfo::NoLayout || !isManaged(w) ) )
        w = w->parentWidget();

    QMap<QWidget *, QPoint> moving;

    checkedSelectionsForMove = true;

    QList<QWidget*> l = qFindChildren<QWidget*>(w->parentWidget());
    if (!l.isEmpty()) {
        QHashIterator<QWidget *, WidgetSelection *> it(usedSelections);
        while (it.hasNext()) {
            it.next();

            WidgetSelection *sel = it.value();
            if (it.key() == mainContainer())
                continue;

            if (!l.contains(sel->widget())) {
                if (LayoutInfo::layoutType(m_core, w) == LayoutInfo::NoLayout)
                    sel->setWidget(0);
            } else {
                if (LayoutInfo::layoutType(m_core, sel->widget()->parentWidget()) == LayoutInfo::NoLayout) {
                    moving.insert(sel->widget(), sel->widget()->pos());
                    sel->widget()->raise();
                    raiseChildSelections(sel->widget());
                    raiseSelection(sel->widget());
                }
            }
        }
    }

    return moving.keys();
}

QList<QWidget*> FormWindow::selectedWidgets() const
{
    QList<QWidget*> widgets;
    for (QHash<QWidget *, WidgetSelection *>::ConstIterator it = usedSelections.begin(); it != usedSelections.end(); ++it)
        widgets.append(it.value()->widget());
    return widgets;
}

void FormWindow::raiseSelection(QWidget *w)
{
    WidgetSelection *s = usedSelections.value(w);
    if (s)
        s->show();
}

void FormWindow::selectWidgets()
{
    QList<QWidget*> l = qFindChildren<QWidget*>(mainContainer());
    QListIterator <QWidget*> it(l);
    while (it.hasNext()) {
        QWidget *w = it.next();
        if (w->isVisibleTo(this) && isManaged(w)) {
            QPoint p = w->mapToGlobal(QPoint(0,0));
            QRect r(p, w->size());
            if (r.intersects(currRect) && !r.contains(currRect))
                selectWidget(w);
        }
    }

    emitSelectionChanged();
}

void FormWindow::handleKeyPressEvent(QWidget *w, QKeyEvent *e)
{
    Q_UNUSED(w);

    e->accept(); // we always accept!

    switch (e->key()) {
        default: break; // we don't care about the other keys

        case Qt::Key_Left:
            cursor()->movePosition(AbstractFormWindowCursor::Left);
            break;

        case Qt::Key_Right:
            cursor()->movePosition(AbstractFormWindowCursor::Right);
            break;
    }
}

void FormWindow::handleKeyReleaseEvent(QWidget *w, QKeyEvent *e)
{
    Q_UNUSED(w);
    e->accept();
}

void FormWindow::selectAll()
{
    checkedSelectionsForMove = false;
    blockSignals(true);
    QList<QWidget*> l = qFindChildren<QWidget*>(mainContainer());
    QListIterator<QWidget*> it(l);
    while (it.hasNext()) {
        QWidget *w = it.next();
        if (w->isVisibleTo(this) && isManaged(w)) {
            selectWidget(w);
        }
    }

    blockSignals(false);
    emitSelectionChanged();
}

void FormWindow::layoutHorizontal()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::HBox);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutVertical()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::VBox);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutGrid()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::Grid);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::deleteWidgets(const QList<QWidget*> &widget_list)
{
    if (widget_list.isEmpty())
        return;

    beginCommand(tr("Delete"));

    foreach (QWidget *w, widget_list) {
        emit widgetRemoved(w);
        DeleteWidgetCommand *cmd = new DeleteWidgetCommand(this);
        cmd->init(w);
        m_commandHistory->push(cmd);
    }

    endCommand();
}


void FormWindow::deleteWidgets()
{
    deleteWidgets(usedSelections.keys());
}

QString FormWindow::fileName() const
{
    return m_fileName;
}

void FormWindow::setFileName(const QString &fileName)
{
    if (m_fileName == fileName)
        return;

    m_fileName = fileName;
    emit fileNameChanged(fileName);
}

QString FormWindow::contents() const
{
    QBuffer b;
    if (!b.open(QIODevice::WriteOnly))
        return QString::null;

    QDesignerResource resource(const_cast<FormWindow*>(this));
    resource.save(&b, mainContainer());

    return b.buffer();
}

void FormWindow::copy()
{
    QBuffer b;
    if (!b.open(QIODevice::WriteOnly))
        return;

    QDesignerResource resource(this);
    QList<QWidget*> sel = selectedWidgets();
    simplifySelection(&sel);
    resource.copy(&b, sel);

    qApp->clipboard()->setText(b.buffer(), QClipboard::Clipboard);
}

void FormWindow::cut()
{
    copy();
    deleteWidgets();
}

void FormWindow::paste()
{
    QWidget *w = mainContainer();
    QList<QWidget*> l(selectedWidgets());
    if (l.count() == 1) {
        w = l.first();
        if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
             (!core()->widgetDataBase()->isContainer(w) &&
               w != mainContainer()))
            w = mainContainer();
    }

    if (w && LayoutInfo::layoutType(m_core, w) == LayoutInfo::NoLayout) {
        clearSelection(true);

        QByteArray code = qApp->clipboard()->text().toAscii();
        QBuffer b(&code);
        b.open(QIODevice::ReadOnly);

        QDesignerResource resource(this);
        QWidget *widget = core()->widgetFactory()->containerOfWidget(w);
        QList<QWidget*> widgets = resource.paste(&b, widget);

        beginCommand(tr("Paste"));
        foreach (QWidget *w, widgets) {
            InsertWidgetCommand *cmd = new InsertWidgetCommand(this);
            cmd->init(w);
            m_commandHistory->push(cmd);
            selectWidget(w, true);
        }
        endCommand();

        /* This will put the freshly pasted widgets into the clipboard, replacing the original.
           The point here is that the copied widgets are shifted a little with respect to the original.
           If the user presses paste again, the pasted widgets will be shifted again, rather than
           appearing on top of the previously pasted widgets. */
        copy();

    } else {
        QMessageBox::information(this, tr("Paste error"),
                                  tr("Can't paste widgets. Designer couldn't find a container\n"
                                      "to paste into which does not contain a layout. Break the layout\n"
                                      "of the container you want to paste into and select this container\n"
                                      "and then paste again."));
    }

}

void FormWindow::manageWidget(QWidget *w)
{
    if (isManaged(w))
        return;

    if (w->hasFocus())
        setFocus();

    core()->metaDataBase()->add(w);

    m_insertedWidgets.insert(w);
    m_widgets.append(w);

    setCursorToAll(Qt::ArrowCursor, w);

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(w))
        manageWidget(promoted->child());

    emit changed();
    emit widgetManaged(w);
}

void FormWindow::unmanageWidget(QWidget *w)
{
    if (!isManaged(w))
        return;

    if (usedSelections.contains(w))
        usedSelections.value(w)->setWidget(0);

    emit aboutToUnmanageWidget(w);

    core()->metaDataBase()->remove(w);

    m_insertedWidgets.remove(w);
    m_widgets.removeAt(m_widgets.indexOf(w));

    emit changed();
    emit widgetUnmanaged(w);
}

bool FormWindow::isManaged(QWidget *w) const
{
    return m_insertedWidgets.contains(w);
}

void FormWindow::breakLayout(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    beginCommand(tr("Break layout"));

    for (;;) {
        if (!w || w == this)
            break;

        if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout
                && core()->widgetDataBase()->isContainer(w, false)) {

            if (BreakLayoutCommand *cmd = breakLayoutCommand(w)) {
                commandHistory()->push(cmd);
            }

            if (!qobject_cast<QLayoutWidget*>(w) && !qobject_cast<QSplitter*>(w))
                break;
        }

        w = w->parentWidget();
    }

    clearSelection(false);
    endCommand();
}

BreakLayoutCommand *FormWindow::breakLayoutCommand(QWidget *w)
{
    QList<QWidget*> widgets;

    QListIterator<QObject*> it(w->children());
    while (it.hasNext()) {
        QObject *obj = it.next();

        if (!obj->isWidgetType()
                || !core()->metaDataBase()->item(obj))
            continue;

        widgets.append(static_cast<QWidget*>(obj));
    }

    if (!widgets.count()) {
        qDebug() << "the layout is empty!!!! layout:" << w->layout();
    }

    BreakLayoutCommand *cmd = new BreakLayoutCommand(this);
    cmd->init(widgets, core()->widgetFactory()->widgetOfContainer(w));
    return cmd;
}

void FormWindow::breakLayout()
{
    QWidget *w = currentWidget() ? currentWidget() : mainContainer();

    if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
         w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout) {
        breakLayout(w);
        return;
    } else {
        QList<QWidget*> widgets = selectedWidgets();
        QListIterator<QWidget*> it(widgets);
        while (it.hasNext()) {
            QWidget *w = it.next();
            if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
                 w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout)
                break;
        }
        if (w) {
            breakLayout(w);
            return;
        }
    }

    w = mainContainer();

    if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout ||
         w->parentWidget() && LayoutInfo::layoutType(m_core, w->parentWidget()) != LayoutInfo::NoLayout)
        breakLayout(w);
}

void FormWindow::beginCommand(const QString &description)
{
    m_commandHistory->push(new QtCommand(QtCommand::MacroBegin, description));
}

void FormWindow::endCommand()
{
    m_commandHistory->push(new QtCommand(QtCommand::MacroEnd, QString::null));
}

void FormWindow::raiseWidgets()
{
    beginCommand(tr("Raise"));

    QList<QWidget*> widgets = selectedWidgets();
    foreach (QWidget *w, widgets) {
        RaiseWidgetCommand *cmd = new RaiseWidgetCommand(this);
        cmd->init(w);
        m_commandHistory->push(cmd);
    }

    endCommand();
}

void FormWindow::lowerWidgets()
{
    beginCommand(tr("Lower"));

    QList<QWidget*> widgets = selectedWidgets();
    foreach (QWidget *w, widgets) {
        LowerWidgetCommand *cmd = new LowerWidgetCommand(this);
        cmd->init(w);
        m_commandHistory->push(cmd);
    }

    endCommand();
}

void FormWindow::handleMouseButtonDblClickEvent(QWidget *w, QMouseEvent * /*e*/)
{
    emit activated(w);

/*
    case TabOrderEditMode:
        if (!isMainContainer(w)) { // press on a child widget
            orderedWidgets.clear();
            orderedWidgets.append(w);
            QListIterator<QWidget*> it(orderedWidgets);
            it.toBack();
            while (it.hasPrevious()) {
                QWidget *wid = it.previous();
                int j = stackedWidgets.indexOf(wid);
                if (j > 0) {
                    stackedWidgets.removeAt(j);
                    stackedWidgets.insert(0, wid);
                }
            }

            TabOrderCommand *cmd = new TabOrderCommand(this);
            cmd->init(stackedWidgets);
            commandHistory()->push(cmd);
        }
    break;
*/
}

void FormWindow::handleContextMenu(QWidget *w, QContextMenuEvent *e)
{
    if (!isMainContainer(w)) { // press on a child widget
        raiseChildSelections(w); // raise selections and select widget
        selectWidget(w);

        // if widget is laid out, find the first non-laid out super-widget
        QWidget *realWidget = w; // but store the original one

        if (qobject_cast<QMainWindow*>(mainContainer()) && static_cast<QMainWindow*>(mainContainer())->centralWidget() == realWidget) {
            e->accept();
#ifndef VS_CTX_MENU
            QMenu *menu = createPopupMenu(this);
            if (menu) {
                menu->exec(e->globalPos());
                delete menu;
            }
#else
            emit showContextMenu(this, e->globalPos());
#endif
        } else {
            e->accept();
#ifndef VS_CTX_MENU
            QMenu *menu = createPopupMenu(realWidget);
            if (menu) {
                menu->exec(e->globalPos());
                delete menu;
            }
#else
            emit showContextMenu(realWidget, e->globalPos());
#endif
        }
    } else {
        e->accept();
        clearSelection();
#ifndef VS_CTX_MENU
        QMenu *menu = createPopupMenu(this);
        if (menu) {
            menu->exec(e->globalPos());
            delete menu;
        }
#else
        emit showContextMenu(this, e->globalPos());
#endif
    }
}

void FormWindow::setContents(QIODevice *dev)
{
    clearSelection();

    if (mainContainer()) {
        core()->metaDataBase()->remove(mainContainer());
        delete mainContainer();
        m_mainContainer = 0;
    }

    m_insertedWidgets.clear();
    m_widgets.clear();
    emit changed();

    QDesignerResource r(this);
    QWidget *w = r.load(dev, this);
    if (!w) {
        w = core()->widgetFactory()->createWidget("QWidget", this);
    }

    setMainContainer(w);
}

void FormWindow::setContents(const QString &contents)
{
    QByteArray data = contents.toUtf8();
    QBuffer b(&data);
    if (b.open(QIODevice::ReadOnly))
        setContents(&b);
}

void FormWindow::layoutHorizontalContainer(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    QList<QObject*> l = w->children();
    if (l.isEmpty())
        return;

    QList<QWidget*> widgets;
    QListIterator<QObject*> it(l);
    while (it.hasNext()) {
        QObject* o = it.next();
        if (!o->isWidgetType())
            continue;

        QWidget *widget = static_cast<QWidget*>(o);
        if (widget->isVisibleTo(this) && isManaged(widget))
            widgets.append(widget);
    }

    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), widgets, LayoutInfo::HBox, w);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutVerticalContainer(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    QList<QObject*> l = w->children();
    if (l.isEmpty())
        return;

    QListIterator<QObject*> it(l);
    QList<QWidget*> widgets;
    while (it.hasNext()) {
        QObject* o = it.next();
        if (!o->isWidgetType())
            continue;

        QWidget *widget = static_cast<QWidget*>(o);
        if (widget->isVisibleTo(this) && isManaged(widget))
            widgets.append(widget);
    }

    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), widgets, LayoutInfo::VBox, w);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutGridContainer(QWidget *w)
{
    if (w == this)
        w = mainContainer();

    w = core()->widgetFactory()->containerOfWidget(w);

    QList<QObject*> l = w->children();
    if (l.isEmpty())
        return;

    QList<QWidget*> widgets;
    QListIterator<QObject*> it(l);
    while (it.hasNext()) {
        QObject* o = it.next();

        if (!o->isWidgetType())
            continue;

        QWidget *widget = static_cast<QWidget*>(o);
        if (widget->isVisibleTo(this) && isManaged(widget))
            widgets.append(widget);
    }

    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), widgets, LayoutInfo::Grid, w);
    clearSelection(false);
    commandHistory()->push(cmd);
}

bool FormWindow::hasInsertedChildren(QWidget *w) const
{
    if (!w)
        return false;
    w = core()->widgetFactory()->containerOfWidget(w);
    if (!w)
        return false;
    QList<QWidget*> l = qFindChildren<QWidget*>(w);
    if (l.isEmpty())
        return false;

    for (int i = 0; i < l.size(); ++i) {
        QWidget* w = l.at(i);
        if (w->isVisibleTo(const_cast<FormWindow*>(this)) && isManaged(w)) {
            return true;
        }
    }

    return false;
}

void FormWindow::layoutHorizontalSplit()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::HBox, /*layoutBase=*/ 0, /*splitter=*/ true);
    clearSelection(false);
    commandHistory()->push(cmd);
}

void FormWindow::layoutVerticalSplit()
{
    LayoutCommand *cmd = new LayoutCommand(this);
    cmd->init(mainContainer(), selectedWidgets(), LayoutInfo::VBox, /*layoutBase=*/ 0, /*splitter=*/ true);
    clearSelection(false);
    commandHistory()->push(cmd);
}

QMenu *FormWindow::createPopupMenu(QWidget *w)
{
    AbstractFormWindowManager *manager = core()->formWindowManager();
    bool isFormWindow = qobject_cast<FormWindow*>(w);

    QMenu *popup = new QMenu(this);

    if (qobject_cast<QDesignerTabWidget*>(w)) {
        QDesignerTabWidget *tabWidget = static_cast<QDesignerTabWidget*>(w);
        if (tabWidget->count()) {
            popup->addAction(tabWidget->actionDeletePage());
        }
        popup->addAction(tabWidget->actionInsertPage());
        popup->addSeparator();
    } else if (qobject_cast<QDesignerStackedWidget*>(w)) {
        QDesignerStackedWidget *stackedWidget = static_cast<QDesignerStackedWidget*>(w);
        if (stackedWidget->count()) {
            popup->addAction(stackedWidget->actionDeletePage());
        }
        popup->addAction(stackedWidget->actionInsertPage());
        popup->addAction(stackedWidget->actionNextPage());
        popup->addAction(stackedWidget->actionPreviousPage());
        popup->addSeparator();
    } else if (qobject_cast<QDesignerToolBox*>(w)) {
        QDesignerToolBox *toolBox = static_cast<QDesignerToolBox*>(w);
        if (toolBox->count()) {
            popup->addAction(toolBox->actionDeletePage());
        }
        popup->addAction(toolBox->actionInsertPage());
        popup->addSeparator();
    }

    if (!isFormWindow) {
        popup->addAction(manager->actionCut());
        popup->addAction(manager->actionCopy());
    }

    popup->addAction(manager->actionPaste());
    popup->addAction(manager->actionSelectAll());

    if (!isFormWindow) {
        popup->addAction(manager->actionDelete());
    }

    popup->addSeparator();
    popup->addAction(manager->actionAdjustSize());
    popup->addAction(manager->actionVerticalLayout());
    popup->addAction(manager->actionHorizontalLayout());
    popup->addAction(manager->actionGridLayout());

    if (!isFormWindow) {
        popup->addAction(manager->actionSplitHorizontal());
        popup->addAction(manager->actionSplitVertical());
    }

    popup->addAction(manager->actionBreakLayout());

    return popup;
}

void FormWindow::showOrderIndicators()
{
    hideOrderIndicators();

    stackedWidgets.clear();
    if (AbstractMetaDataBaseItem *item = core()->metaDataBase()->item(this)) {
        stackedWidgets = item->tabOrder();
    }
    int order = 1;

    foreach (QWidget *widget, widgets()) {
        if (qobject_cast<QLayoutWidget*>(widget)
                || widget == mainContainer()
                || !widget->isExplicitlyHidden()
                || !canBeBuddy(widget))
            continue;

        OrderIndicator* ind = new OrderIndicator(order++, widget, this);
        orderIndicators.append(ind);

        if (!stackedWidgets.contains(widget))
            stackedWidgets.append(widget);
    }

    updateOrderIndicators();
}

void FormWindow::hideOrderIndicators()
{
    while (!orderIndicators.isEmpty())
        delete orderIndicators.takeFirst();
}

void FormWindow::updateOrderIndicators()
{
    int order = 1;
    foreach (QWidget *w, stackedWidgets) {
        foreach (OrderIndicator *indicator, orderIndicators) {
            indicator->setOrder(order, w);
        }

        ++order;
    }
}

void FormWindow::repositionOrderIndicators()
{
    foreach (OrderIndicator *indicator, orderIndicators)
        indicator->reposition();
}

void FormWindow::resizeEvent(QResizeEvent *e)
{
/*    if (editMode() == TabOrderEditMode)
        repositionOrderIndicators(); */

    m_geometryChangedTimer->start(10);

    QWidget::resizeEvent(e);
}

/*!
  Maps \a pos in \a w's coordinates to the form's coordinate system.

  This is the equivalent to mapFromGlobal(w->mapToGlobal(pos)) but
  avoids the two roundtrips to the X-Server on Unix/X11.
 */
QPoint FormWindow::mapToForm(const QWidget *w, const QPoint &pos) const
{
    QPoint p = pos;
    const QWidget* i = w;
    while (i && !i->isWindow() && !isMainContainer(i)) {
        p = i->mapToParent(p);
        i = i->parentWidget();
    }

    return mapFromGlobal(w->mapToGlobal(pos));
}

bool FormWindow::canBeBuddy(QWidget *w) const // ### rename me.
{
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(core()->extensionManager(), w)) {
        int index = sheet->indexOf("focusPolicy");
        if (index != -1) {
            bool ok = false;
            Qt::FocusPolicy q = (Qt::FocusPolicy) Utils::valueOf(sheet->property(index), &ok);
            return ok && q != Qt::NoFocus;
        }
    }

    return false;
}

QWidget *FormWindow::findContainer(QWidget *w, bool excludeLayout) const
{
    if (!isChildOf(w, this)
        || const_cast<const QWidget *>(w) == this)
        return 0;

    AbstractWidgetFactory *widgetFactory = core()->widgetFactory();
    AbstractWidgetDataBase *widgetDataBase = core()->widgetDataBase();
    AbstractMetaDataBase *metaDataBase = core()->metaDataBase();

    QWidget *container = widgetFactory->containerOfWidget(mainContainer()); // default parent for new widget is the formwindow
    if (!isMainContainer(w)) { // press was not on formwindow, check if we can find another parent
        while (w) {
            if (qobject_cast<InvisibleWidget*>(w) || !metaDataBase->item(w)) {
                w = w->parentWidget();
                continue;
            }

            bool isContainer = widgetDataBase->isContainer(w, true) || w == mainContainer();
            if (!isContainer || (excludeLayout && qobject_cast<QLayoutWidget*>(w))) { // ### skip QSplitter
                w = w->parentWidget();
            } else {
                container = w;
                break;
            }
        }
    }

    return container;
}

void FormWindow::simplifySelection(QList<QWidget*> *sel)
{
    QMutableListIterator<QWidget*> it(*sel);
    while (it.hasNext()) {
        QWidget *child = it.next();
        QWidget *w = child;

        while (w->parentWidget() && sel->contains(w->parentWidget()))
            w = w->parentWidget();

        if (child != w)
            it.remove();
    }
}

// This is very similar to the static AbstractFormWindow::findFormWindow(), please KEEP IN SYNC.
FormWindow *FormWindow::findFormWindow(QWidget *w)
{
    while (w) {
        if (FormWindow *fw = qobject_cast<FormWindow*>(w)) {
            return fw;
        } else if (qobject_cast<QMainWindow*>(w)) {
            /* skip me */
        } else if (w->isWindow())
            break;

        w = w->parentWidget();
    }

    return 0;
}

void FormWindow::repaintSelection()
{
    QList<QWidget*> sel = selectedWidgets();
    foreach (QWidget *ww, sel)
        repaintSelection(ww);
}

bool FormWindow::isDirty() const
{
    return m_dirty;
}

void FormWindow::setDirty(bool dirty)
{
    m_dirty = dirty;

    if (!m_dirty)
        m_lastIndex = m_commandHistory->currentIndex();
}

void FormWindow::updateDirty()
{
    m_dirty = m_commandHistory->currentIndex() != m_lastIndex;
}

QWidget *FormWindow::containerAt(const QPoint &pos)
{
    QWidget *widget = widgetAt(pos);
    return findContainer(widget, true);
}

static QWidget *childAt_SkipDropLine(QWidget *w, QPoint pos)
{
    QObjectList child_list = w->children();
    for (int i = child_list.size() - 1; i >= 0; --i) {
        QObject *child_obj = child_list[i];
        if (!child_obj->isWidgetType())
            continue;
        if (qobject_cast<DropLine*>(child_obj) != 0)
            continue;
        if (qobject_cast<WidgetHandle*>(child_obj) != 0)
            continue;
        QWidget *child = static_cast<QWidget*>(child_obj);
        if (!child->geometry().contains(pos))
            continue;
        pos = child->mapFromParent(pos);
        QWidget *res = childAt_SkipDropLine(child, pos);
        if (res == 0)
            return child;
        else
            return res;
    }

    return 0;
}

QWidget *FormWindow::widgetAt(const QPoint &pos)
{
    QWidget *w = childAt(pos);
    if (qobject_cast<DropLine*>(w) != 0 || qobject_cast<WidgetHandle*>(w) != 0)
        w = childAt_SkipDropLine(this, pos);
    return w == 0 ? this : w;
}

void FormWindow::highlightWidget(QWidget *widget, const QPoint &pos, HighlightMode mode)
{
    Q_ASSERT(widget);

//    if (widget == mainContainer()) // skip the maincontainer
//        return;

    QWidget *container = findContainer(widget, false);

    Q_ASSERT(!qobject_cast<ConnectionEdit*>(widget));

    if (container == 0 || core()->metaDataBase()->item(container) == 0)
        return;

    if (mode == Restore) {
        container->setPalette(palettesBeforeHighlight.take(container));

        if (ILayoutDecoration *g = qt_extension<ILayoutDecoration*>(core()->extensionManager(), container)) {
            g->adjustIndicator(QPoint(), -1);
        }

        return;
    }

    QPalette p = container->palette();
    if (!palettesBeforeHighlight.contains(container)) {
        if (container->testAttribute(Qt::WA_SetPalette))
            palettesBeforeHighlight[container] = p;
        else
            palettesBeforeHighlight[container] = QPalette();
    }
    p.setColor(backgroundRole(), p.midlight().color());
    container->setPalette(p);

    if (ILayoutDecoration *g = qt_extension<ILayoutDecoration*>(core()->extensionManager(), container)) {
        QPoint pt = widget->mapTo(container, pos);
        int index = g->findItemAt(pt);
        g->adjustIndicator(pt, index);
    }
}

DomConnections *FormWindow::saveConnections()
{
#if 0 // ### port me
    if (m_signalSlotEditor)
        return m_signalSlotEditor->toUi();
#endif

    return 0;
}

QList<QWidget *> FormWindow::widgets(QWidget *widget) const
{
    QList<QWidget *> l;

    foreach (QObject *o, widget->children()) {
        QWidget *w = qobject_cast<QWidget*>(o);
        if (w && isManaged(w))
            l.append(w);
    }

    return l;
}


void FormWindow::createConnections(DomConnections *connections, QWidget *parent)
{
#if 0 // ### port me
    m_signalSlotEditor->fromUi(connections, parent);
#endif
}

int FormWindow::toolCount() const
{
    return m_widgetStack->count();
}

AbstractFormWindowTool *FormWindow::tool(int index) const
{
    return m_widgetStack->tool(index);
}

void FormWindow::registerTool(AbstractFormWindowTool *tool)
{
    Q_ASSERT(tool != 0);

    m_widgetStack->addTool(tool);

    if (m_mainContainer)
        m_mainContainer->update();
}

void FormWindow::setCurrentTool(int index)
{
    m_widgetStack->setCurrentTool(index);
}

int FormWindow::currentTool() const
{
    return m_widgetStack->currentIndex();
}

bool FormWindow::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    if (m_widgetStack == 0)
        return false;

    AbstractFormWindowTool *tool = m_widgetStack->currentTool();
    if (tool == 0)
        return false;
    bool handled = tool->handleEvent(widget, managedWidget, event);

    return handled;
}

void FormWindow::initializeCoreTools()
{
    m_widgetEditor = new WidgetEditorTool(this);
    registerTool(m_widgetEditor);
}

void FormWindow::checkSelection()
{
    m_checkSelectionTimer->start(0);
}

void FormWindow::checkSelectionNow()
{
    m_checkSelectionTimer->stop();

    foreach (QWidget *widget, selectedWidgets()) {
        updateSelection(widget);

        if (LayoutInfo::layoutType(core(), widget) != LayoutInfo::NoLayout)
            updateChildSelections(widget);
    }
}

QString FormWindow::author() const
{
    return m_author;
}

QString FormWindow::comment() const
{
     return m_comment;
}

void FormWindow::setAuthor(const QString &author)
{
    m_author = author;
}

void FormWindow::setComment(const QString &comment)
{
    m_comment = comment;
}

void FormWindow::editWidgets()
{
    m_widgetEditor->action()->trigger();
}

#include "formwindow.moc"

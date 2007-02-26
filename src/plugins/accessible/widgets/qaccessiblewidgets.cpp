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

#include "qaccessiblewidgets.h"
#include "qabstracttextdocumentlayout.h"
#include "qtextedit.h"
#include "qtextdocument.h"
#include "qtextobject.h"
#include "qscrollbar.h"
#include "qdebug.h"
#include <QApplication>
#include <QStackedWidget>
#include <QToolBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QWorkspace>
#include <QDialogButtonBox>
#include <limits.h>
#include <QRubberBand>
#include <QTextBrowser>
#include <QCalendarWidget>
#include <QAbstractItemView>
#include <QDockWidget>
#include <private/qdockwidget_p.h>

#ifndef QT_NO_ACCESSIBILITY

#ifndef QT_NO_TEXTEDIT

static inline int distance(QWidget *source, QWidget *target,
                           QAccessible::RelationFlag relation)
{
    if (!source || !target)
        return -1;

    int returnValue = -1;
    switch (relation) {
    case QAccessible::Up:
        if (target->y() <= source->y())
            returnValue = source->y() - target->y();
        break;
    case QAccessible::Down:
        if (target->y() >= source->y() + source->height())
            returnValue = target->y() - (source->y() + source->height());
        break;
    case QAccessible::Right:
        if (target->x() >= source->x() + source->width())
            returnValue = target->x() - (source->x() + source->width());
        break;
    case QAccessible::Left:
        if (target->x() <= source->x())
            returnValue = source->x() - target->x();
        break;
    default:
        break;
    }
    return returnValue;
}

static inline QWidget *mdiAreaNavigate(QWidget *area,
                                       QAccessible::RelationFlag relation, int entry)
{
    const QMdiArea *mdiArea = qobject_cast<QMdiArea *>(area);
#ifndef QT_NO_WORKSPACE
    const QWorkspace *workspace = qobject_cast<QWorkspace *>(area);
#endif
    if (!mdiArea
#ifndef QT_NO_WORKSPACE
    && !workspace
#endif
    )
        return 0;

    QWidgetList windows;
    if (mdiArea) {
        foreach (QMdiSubWindow *window, mdiArea->subWindowList())
            windows.append(window);
    } else {
#ifndef QT_NO_WORKSPACE
        foreach (QWidget *window, workspace->windowList())
            windows.append(window->parentWidget());
#endif
    }

    if (windows.isEmpty() || entry < 1 || entry > windows.count())
        return 0;

    QWidget *source = windows.at(entry - 1);
    QMap<int, QWidget *> candidates;
    foreach (QWidget *window, windows) {
        if (source == window)
            continue;
        int candidateDistance = distance(source, window, relation);
        if (candidateDistance >= 0)
            candidates.insert(candidateDistance, window);
    }

    int minimumDistance = INT_MAX;
    QWidget *target = 0;
    foreach (QWidget *candidate, candidates.values()) {
        switch (relation) {
        case QAccessible::Up:
        case QAccessible::Down:
            if (qAbs(candidate->x() - source->x()) < minimumDistance) {
                target = candidate;
                minimumDistance = qAbs(candidate->x() - source->x());
            }
            break;
        case QAccessible::Left:
        case QAccessible::Right:
            if (qAbs(candidate->y() - source->y()) < minimumDistance) {
                target = candidate;
                minimumDistance = qAbs(candidate->y() - source->y());
            }
            break;
        default:
            break;
        }
        if (minimumDistance == 0)
            break;
    }

#ifndef QT_NO_WORKSPACE
    if (workspace) {
        foreach (QWidget *widget, workspace->windowList()) {
            if (widget->parentWidget() == target)
                target = widget;
        }
    }
#endif
    return target;
}

/*!
  \class QAccessibleTextEdit qaccessiblewidget.h
  \brief The QAccessibleTextEdit class implements the QAccessibleInterface for richtext editors.
  \internal
*/

static QTextBlock qTextBlockAt(const QTextDocument *doc, int pos)
{
    Q_ASSERT(pos >= 0);

    QTextBlock block = doc->begin();
    int i = 0;
    while (block.isValid() && i < pos) {
        block = block.next();
        ++i;
    }
    return block;
}

static int qTextBlockPosition(QTextBlock block)
{
    int child = 0;
    while (block.isValid()) {
        block = block.previous();
        ++child;
    }

    return child;
}

/*!
  \fn QAccessibleTextEdit::QAccessibleTextEdit(QWidget* widget)

  Constructs a QAccessibleTextEdit object for a \a widget.
*/
QAccessibleTextEdit::QAccessibleTextEdit(QWidget *o)
: QAccessibleWidgetEx(o, EditableText)
{
    Q_ASSERT(widget()->inherits("QTextEdit"));
    childOffset = QAccessibleWidgetEx::childCount();
}

/*! Returns the text edit. */
QTextEdit *QAccessibleTextEdit::textEdit() const
{
    return static_cast<QTextEdit *>(widget());
}

QRect QAccessibleTextEdit::rect(int child) const
{
    if (child <= childOffset)
        return QAccessibleWidgetEx::rect(child);

     QTextEdit *edit = textEdit();
     QTextBlock block = qTextBlockAt(edit->document(), child - childOffset - 1);
     if (!block.isValid())
         return QRect();

     QRect rect = edit->document()->documentLayout()->blockBoundingRect(block).toRect();
     rect.translate(-edit->horizontalScrollBar()->value(), -edit->verticalScrollBar()->value());

     rect = edit->viewport()->rect().intersect(rect);
     if (rect.isEmpty())
         return QRect();

     return rect.translated(edit->viewport()->mapToGlobal(QPoint(0, 0)));
}

int QAccessibleTextEdit::childAt(int x, int y) const
{
    QTextEdit *edit = textEdit();
    if (!edit->isVisible())
        return -1;

    QPoint point = edit->viewport()->mapFromGlobal(QPoint(x, y));
    QTextBlock block = edit->cursorForPosition(point).block();
    if (block.isValid())
        return qTextBlockPosition(block) + childOffset;

    return QAccessibleWidgetEx::childAt(x, y);
}

/*! \reimp */
QString QAccessibleTextEdit::text(Text t, int child) const
{
    if (!textEdit()->isVisible())
        return QString();
    if (t == Value) {
        if (child > childOffset)
            return qTextBlockAt(textEdit()->document(), child - childOffset - 1).text();
        if (!child)
            return textEdit()->toPlainText();
    }

    return QAccessibleWidgetEx::text(t, child);
}

/*! \reimp */
void QAccessibleTextEdit::setText(Text t, int child, const QString &text)
{
    if (!textEdit()->isVisible())
        return;
    if (t != Value || (child > 0 && child <= childOffset)) {
        QAccessibleWidgetEx::setText(t, child, text);
        return;
    }
    if (textEdit()->isReadOnly())
        return;

    if (!child) {
        textEdit()->setText(text);
        return;
    }
    QTextBlock block = qTextBlockAt(textEdit()->document(), child - childOffset - 1);
    if (!block.isValid())
        return;

    QTextCursor cursor(block);
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.insertText(text);
}

/*! \reimp */
QAccessible::Role QAccessibleTextEdit::role(int child) const
{
    if (child > childOffset)
        return EditableText;
    return QAccessibleWidgetEx::role(child);
}

QVariant QAccessibleTextEdit::invokeMethodEx(QAccessible::Method method, int child,
                                                     const QVariantList &params)
{
    if (child)
        return QVariant();

    switch (method) {
    case ListSupportedMethods: {
        QVariantList list;
        list << ListSupportedMethods << SetCursorPosition << GetCursorPosition;
        return list;
    }
    case SetCursorPosition: {
        QTextCursor cursor = textEdit()->textCursor();
        cursor.setPosition(params.value(0).toInt());
        textEdit()->setTextCursor(cursor);
        return true; }
    case GetCursorPosition:
        return textEdit()->textCursor().position();
    }

    return QVariant();
}

int QAccessibleTextEdit::childCount() const
{
    if (!textEdit()->isVisible())
        return 0;
    return childOffset + textEdit()->document()->blockCount();
}
#endif // QT_NO_TEXTEDIT

#ifndef QT_NO_STACKEDWIDGET
// ======================= QAccessibleStackedWidget ======================
QAccessibleStackedWidget::QAccessibleStackedWidget(QWidget *widget)
    : QAccessibleWidgetEx(widget, LayeredPane)
{
    Q_ASSERT(qobject_cast<QStackedWidget *>(widget));
}

QVariant QAccessibleStackedWidget::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}


int QAccessibleStackedWidget::childAt(int x, int y) const
{
    if (!stackedWidget()->isVisible())
        return -1;
    QWidget *currentWidget = stackedWidget()->currentWidget();
    if (!currentWidget)
        return -1;
    QPoint position = currentWidget->mapFromGlobal(QPoint(x, y));
    if (currentWidget->rect().contains(position))
        return 1;
    return -1;
}

int QAccessibleStackedWidget::childCount() const
{
    if (!stackedWidget()->isVisible())
        return 0;
    return stackedWidget()->count();
}

int QAccessibleStackedWidget::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child || (stackedWidget()->currentWidget() != child->object()))
        return -1;
    return 1;
}

int QAccessibleStackedWidget::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (!stackedWidget()->isVisible())
        return -1;

    QObject *targetObject = 0;
    switch (relation) {
    case Child:
        if (entry != 1)
            return -1;
        targetObject = stackedWidget()->currentWidget();
        break;
    default:
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    }
    *target = QAccessible::queryAccessibleInterface(targetObject);
    return *target ? 0 : -1;
}

QStackedWidget *QAccessibleStackedWidget::stackedWidget() const
{
    return static_cast<QStackedWidget *>(object());
}
#endif // QT_NO_STACKEDWIDGET

#ifndef QT_NO_TOOLBOX
// ======================= QAccessibleToolBox ======================
QAccessibleToolBox::QAccessibleToolBox(QWidget *widget)
    : QAccessibleWidgetEx(widget, LayeredPane)
{
    Q_ASSERT(qobject_cast<QToolBox *>(widget));
}

QString QAccessibleToolBox::text(Text textType, int child) const
{
    if (!toolBox()->isVisible())
        return QString();
    if (textType != Value || child <= 0 || child > toolBox()->count())
        return QAccessibleWidgetEx::text(textType, child);
    return toolBox()->itemText(child - 1);
}

void QAccessibleToolBox::setText(Text textType, int child, const QString &text)
{
    if (!toolBox()->isVisible())
        return;
    if (textType != Value || child <= 0 || child > toolBox()->count()) {
        QAccessibleWidgetEx::setText(textType, child, text);
        return;
    }
    toolBox()->setItemText(child - 1, text);
}

QAccessible::State QAccessibleToolBox::state(int child) const
{
    QWidget *childWidget = toolBox()->widget(child - 1);
    if (!childWidget)
        return QAccessibleWidgetEx::state(child);
    QAccessible::State childState = QAccessible::Normal;
    if (toolBox()->currentWidget() == childWidget)
        childState |= QAccessible::Expanded;
    else
        childState |= QAccessible::Collapsed;
    return childState;
}

QVariant QAccessibleToolBox::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

int QAccessibleToolBox::childCount() const
{
    if (!toolBox()->isVisible())
        return 0;
    return toolBox()->count();
}

int QAccessibleToolBox::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child)
        return -1;
    QWidget *childWidget = qobject_cast<QWidget *>(child->object());
    if (!childWidget)
        return -1;
    int index = toolBox()->indexOf(childWidget);
    if (index != -1)
        ++index;
    return index;
}

int QAccessibleToolBox::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (!toolBox()->isVisible())
        return -1;
    if (entry <= 0 || entry > toolBox()->count())
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    int index = -1;
    if (relation == QAccessible::Up)
        index = entry - 2;
    else if (relation == QAccessible::Down)
        index = entry;
    *target = QAccessible::queryAccessibleInterface(toolBox()->widget(index));
    return *target ? index + 1: -1;
}

QToolBox * QAccessibleToolBox::toolBox() const
{
    return static_cast<QToolBox *>(object());
}
#endif // QT_NO_TOOLBOX

// ======================= QAccessibleMdiArea ======================
QAccessibleMdiArea::QAccessibleMdiArea(QWidget *widget)
    : QAccessibleWidgetEx(widget, LayeredPane)
{
    Q_ASSERT(qobject_cast<QMdiArea *>(widget));
}

QAccessible::State QAccessibleMdiArea::state(int child) const
{
    if (child < 0)
        return QAccessibleWidgetEx::state(child);
    if (child == 0)
        return QAccessible::Normal;
    QList<QMdiSubWindow *> subWindows = mdiArea()->subWindowList();
    if (subWindows.isEmpty() || child > subWindows.count())
        return QAccessibleWidgetEx::state(child);
    if (subWindows.at(child - 1) == mdiArea()->activeSubWindow())
        return QAccessible::Focused;
    return QAccessible::Normal;
}

QVariant QAccessibleMdiArea::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

int QAccessibleMdiArea::childCount() const
{
    if (!mdiArea()->isVisible())
        return 0;
    return mdiArea()->subWindowList().count();
}

int QAccessibleMdiArea::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child || !child->object() || mdiArea()->subWindowList().isEmpty())
        return -1;
    if (QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(child->object())) {
        int index = mdiArea()->subWindowList().indexOf(window);
        if (index != -1)
            return ++index;
    }
    return -1;
}

int QAccessibleMdiArea::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (!mdiArea()->isVisible())
        return -1;
    QWidget *targetObject = 0;
    QList<QMdiSubWindow *> subWindows = mdiArea()->subWindowList();
    switch (relation) {
    case Child:
        if (entry < 1 || subWindows.isEmpty() || entry > subWindows.count())
            return -1;
        targetObject = subWindows.at(entry - 1);
        break;
    case Up:
    case Down:
    case Left:
    case Right:
        targetObject = mdiAreaNavigate(mdiArea(), relation, entry);
        break;
    default:
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    }
    *target = QAccessible::queryAccessibleInterface(targetObject);
    return indexOfChild(*target);
}

QMdiArea *QAccessibleMdiArea::mdiArea() const
{
    return static_cast<QMdiArea *>(object());
}

// ======================= QAccessibleMdiSubWindow ======================
QAccessibleMdiSubWindow::QAccessibleMdiSubWindow(QWidget *widget)
    : QAccessibleWidgetEx(widget, QAccessible::Window)
{
    Q_ASSERT(qobject_cast<QMdiSubWindow *>(widget));
}

QString QAccessibleMdiSubWindow::text(Text textType, int child) const
{
    if (!mdiSubWindow()->isVisible())
        return QString();
    if (textType == QAccessible::Name && (child == 0 || child == 1)) {
        QString title = mdiSubWindow()->windowTitle();
        title.replace(QLatin1String("[*]"), QLatin1String(""));
        return title;
    }
    return QAccessibleWidgetEx::text(textType, child);
}

void QAccessibleMdiSubWindow::setText(Text textType, int child, const QString &text)
{
        if (!mdiSubWindow()->isVisible())
        return;
    if (textType == QAccessible::Name && (child == 0 || child == 1))
        mdiSubWindow()->setWindowTitle(text);
    else
        QAccessibleWidgetEx::setText(textType, child, text);
}

QAccessible::State QAccessibleMdiSubWindow::state(int child) const
{
    if (child != 0 || !mdiSubWindow()->parent())
        return QAccessibleWidgetEx::state(child);
    QAccessible::State state = QAccessible::Normal | QAccessible::Focusable;
    if (!mdiSubWindow()->isMaximized())
        state |= (QAccessible::Movable | QAccessible::Sizeable);
    if (mdiSubWindow()->isAncestorOf(QApplication::focusWidget())
            || QApplication::focusWidget() == mdiSubWindow())
        state |= QAccessible::Focused;
    if (!mdiSubWindow()->isVisible())
        state |= QAccessible::Invisible;
    if (!mdiSubWindow()->parentWidget()->contentsRect().contains(mdiSubWindow()->geometry()))
        state |= QAccessible::Offscreen;
    if (!mdiSubWindow()->isEnabled())
        state |= QAccessible::Unavailable;
    return state;
}

QVariant QAccessibleMdiSubWindow::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

int QAccessibleMdiSubWindow::childCount() const
{
    if (!mdiSubWindow()->isVisible())
        return 0;
    if (mdiSubWindow()->widget())
        return 1;
    return 0;
}

int QAccessibleMdiSubWindow::indexOfChild(const QAccessibleInterface *child) const
{
    if (child && child->object() && child->object() == mdiSubWindow()->widget())
        return 1;
    return -1;
}

int QAccessibleMdiSubWindow::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (!mdiSubWindow()->isVisible())
        return -1;

    if (!mdiSubWindow()->parent())
        return QAccessibleWidgetEx::navigate(relation, entry, target);

    QWidget *targetObject = 0;
    QMdiSubWindow *source = mdiSubWindow();
    switch (relation) {
    case Child:
        if (entry != 1 || !source->widget())
            return -1;
        targetObject = source->widget();
        break;
    case Up:
    case Down:
    case Left:
    case Right: {
        if (entry != 0)
            break;
        QWidget *parent = source->parentWidget();
        while (parent && !parent->inherits("QMdiArea"))
            parent = parent->parentWidget();
        QMdiArea *mdiArea = qobject_cast<QMdiArea *>(parent);
        if (!mdiArea)
            break;
        int index = mdiArea->subWindowList().indexOf(source);
        if (index == -1)
            break;
        if (QWidget *dest = mdiAreaNavigate(mdiArea, relation, index + 1)) {
            *target = QAccessible::queryAccessibleInterface(dest);
            return *target ? 0 : -1;
        }
        break;
    }
    default:
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    }
    *target = QAccessible::queryAccessibleInterface(targetObject);
    return indexOfChild(*target);
}

QRect QAccessibleMdiSubWindow::rect(int child) const
{
    if (mdiSubWindow()->isHidden())
        return QRect();
    if (!mdiSubWindow()->parent())
        return QAccessibleWidgetEx::rect(child);
    const QPoint pos = mdiSubWindow()->mapToGlobal(QPoint(0, 0));
    if (child == 0)
        return QRect(pos, mdiSubWindow()->size());
    if (child == 1 && mdiSubWindow()->widget()) {
        if (mdiSubWindow()->widget()->isHidden())
            return QRect();
        const QRect contentsRect = mdiSubWindow()->contentsRect();
        return QRect(pos.x() + contentsRect.x(), pos.y() + contentsRect.y(),
                     contentsRect.width(), contentsRect.height());
    }
    return QRect();
}

int QAccessibleMdiSubWindow::childAt(int x, int y) const
{
    if (!mdiSubWindow()->isVisible())
        return -1;
    if (!mdiSubWindow()->parent())
        return QAccessibleWidgetEx::childAt(x, y);
    const QRect globalGeometry = rect(0);
    if (!globalGeometry.isValid())
        return -1;
    const QRect globalChildGeometry = rect(1);
    if (globalChildGeometry.isValid() && globalChildGeometry.contains(QPoint(x, y)))
        return 1;
    if (globalGeometry.contains(QPoint(x, y)))
        return 0;
    return -1;
}

QMdiSubWindow *QAccessibleMdiSubWindow::mdiSubWindow() const
{
    return static_cast<QMdiSubWindow *>(object());
}

// ======================= QAccessibleWorkspace ======================
#ifndef QT_NO_WORKSPACE
QAccessibleWorkspace::QAccessibleWorkspace(QWidget *widget)
    : QAccessibleWidgetEx(widget, LayeredPane)
{
    Q_ASSERT(qobject_cast<QWorkspace *>(widget));
}

QAccessible::State QAccessibleWorkspace::state(int child) const
{
    if (child < 0)
        return QAccessibleWidgetEx::state(child);
    if (child == 0)
        return QAccessible::Normal;
    QWidgetList subWindows = workspace()->windowList();
    if (subWindows.isEmpty() || child > subWindows.count())
        return QAccessibleWidgetEx::state(child);
    if (subWindows.at(child - 1) == workspace()->activeWindow())
        return QAccessible::Focused;
    return QAccessible::Normal;
}

QVariant QAccessibleWorkspace::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

int QAccessibleWorkspace::childCount() const
{
    if (!workspace()->isVisible())
        return 0;
    return workspace()->windowList().count();
}

int QAccessibleWorkspace::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child || !child->object() || workspace()->windowList().isEmpty())
        return -1;
    if (QWidget *window = qobject_cast<QWidget *>(child->object())) {
        int index = workspace()->windowList().indexOf(window);
        if (index != -1)
            return ++index;
    }
    return -1;
}

int QAccessibleWorkspace::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (!workspace()->isVisible())
        return -1;
    QWidget *targetObject = 0;
    QWidgetList subWindows = workspace()->windowList();
    switch (relation) {
    case Child:
        if (entry < 1 || subWindows.isEmpty() || entry > subWindows.count())
            return -1;
        targetObject = subWindows.at(entry - 1);
        break;
    case Up:
    case Down:
    case Left:
    case Right:
        targetObject = mdiAreaNavigate(workspace(), relation, entry);
        break;
    default:
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    }
    *target = QAccessible::queryAccessibleInterface(targetObject);
    return indexOfChild(*target);
}

QWorkspace *QAccessibleWorkspace::workspace() const
{
    return static_cast<QWorkspace *>(object());
}
#endif


#ifndef QT_NO_DIALOGBUTTONBOX
// ======================= QAccessibleDialogButtonBox ======================
QAccessibleDialogButtonBox::QAccessibleDialogButtonBox(QWidget *widget)
    : QAccessibleWidgetEx(widget, Grouping)
{
    Q_ASSERT(qobject_cast<QDialogButtonBox*>(widget));
}

QVariant QAccessibleDialogButtonBox::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}
#endif // QT_NO_DIALOGBUTTONBOX

#ifndef QT_NO_RUBBERBAND
// ======================= QAccessibleRubberBand =========================
QAccessibleRubberBand::QAccessibleRubberBand(QWidget *widget)
    : QAccessibleWidgetEx(widget, Border)
{
    Q_ASSERT(qobject_cast<QRubberBand *>(widget));
}

QVariant QAccessibleRubberBand::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}
#endif // QT_NO_RUBBERBAND

#ifndef QT_NO_TEXTBROWSER
QAccessibleTextBrowser::QAccessibleTextBrowser(QWidget *widget)
    : QAccessibleTextEdit(widget)
{
    Q_ASSERT(qobject_cast<QTextBrowser *>(widget));
}

QAccessible::Role QAccessibleTextBrowser::role(int child) const
{
    if (child != 0)
        return QAccessibleTextEdit::role(child);
    return QAccessible::StaticText;
}
#endif // QT_NO_TEXTBROWSER

#ifndef QT_NO_CALENDARWIDGET
// ===================== QAccessibleCalendarWidget ========================
QAccessibleCalendarWidget::QAccessibleCalendarWidget(QWidget *widget)
    : QAccessibleWidgetEx(widget, Table)
{
    Q_ASSERT(qobject_cast<QCalendarWidget *>(widget));
}

QVariant QAccessibleCalendarWidget::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

int QAccessibleCalendarWidget::childCount() const
{
   if (!calendarWidget()->isVisible())
       return 0;
   return calendarWidget()->isNavigationBarVisible() ? 2 : 1;
}

int QAccessibleCalendarWidget::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child || !child->object() || childCount() <= 0)
        return -1;
    if (qobject_cast<QAbstractItemView *>(child->object()))
        return childCount();
    return 1;
}

int QAccessibleCalendarWidget::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (!calendarWidget()->isVisible())
        return -1;
    if (entry <= 0 || entry > childCount())
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    QWidget *targetWidget = 0;
    switch (relation) {
    case Child:
        if (childCount() == 1) {
            targetWidget = calendarView();
        } else {
            if (entry == 1)
                targetWidget = navigationBar();
            else
                targetWidget = calendarView();
        }
        break;
    case Up:
        if (entry == 2)
            targetWidget = navigationBar();
        break;
    case Down:
        if (entry == 1 && childCount() == 2)
            targetWidget = calendarView();
        break;
    default:
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    }
    if (targetWidget && !targetWidget->isVisible())
        return -1;
    *target = queryAccessibleInterface(targetWidget);
    return indexOfChild(*target);
}

QRect QAccessibleCalendarWidget::rect(int child) const
{
    if (!calendarWidget()->isVisible() || child > childCount())
        return QRect();
    if (child == 0)
        return QAccessibleWidgetEx::rect(child);
    QWidget *childWidget = 0;
    if (childCount() == 2)
        childWidget = child == 1 ? navigationBar() : calendarView();
    else
        childWidget = calendarView();
    return QRect(childWidget->mapToGlobal(QPoint(0, 0)), childWidget->size());
}

int QAccessibleCalendarWidget::childAt(int x, int y) const
{
    const QPoint globalTargetPos = QPoint(x, y);
    if (!rect(0).contains(globalTargetPos))
        return -1;
    if (rect(1).contains(globalTargetPos))
        return 1;
    if (rect(2).contains(globalTargetPos))
        return 2;
    return 0;
}

QCalendarWidget *QAccessibleCalendarWidget::calendarWidget() const
{
    return static_cast<QCalendarWidget *>(object());
}

QAbstractItemView *QAccessibleCalendarWidget::calendarView() const
{
    foreach (QObject *child, calendarWidget()->children()) {
        if (child->objectName() == QLatin1String("qt_calendar_calendarview"))
            return static_cast<QAbstractItemView *>(child);
    }
    return 0;
}

QWidget *QAccessibleCalendarWidget::navigationBar() const
{
    foreach (QObject *child, calendarWidget()->children()) {
        if (child->objectName() == QLatin1String("qt_calendar_navigationbar"))
            return static_cast<QWidget *>(child);
    }
    return 0;
}
#endif // QT_NO_CALENDARWIDGET

#ifndef QT_NO_DOCKWIDGET
QAccessibleDockWidget::QAccessibleDockWidget(QWidget *widget)
    : QAccessibleWidgetEx(widget, Window)
{

}

int QAccessibleDockWidget::navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const
{
    if (relation == Child) {
        if (entry == 1) {
            *iface = new QAccessibleTitleBar(dockWidget());
            return 0;
        } else if (entry == 2) {
            if (dockWidget()->widget())
                *iface = QAccessible::queryAccessibleInterface(dockWidget()->widget());
            return 0;
        }
        *iface = 0;
        return -1;
    }
    return QAccessibleWidgetEx::navigate(relation, entry, iface);
}

int QAccessibleDockWidget::childAt(int x, int y) const
{
    for (int i = childCount(); i >= 1; --i) {
        if (rect(i).contains(x,y))
            return i;
    }
    return -1;
}

int QAccessibleDockWidget::childCount() const
{
    return dockWidget()->widget() ? 2 : 1;
}

int QAccessibleDockWidget::indexOfChild(const QAccessibleInterface *child) const
{
    if (child) {
        if (qobject_cast<QDockWidget *>(child->object()) == dockWidget() && child->role(0) == TitleBar) {
            return 1;
        } else {
            return 2;   //###
        }
    }
    return -1;
}

QAccessible::Role QAccessibleDockWidget::role(int child) const
{
    switch (child) {
        case 0:
            return Window;
        case 1:
            return TitleBar;
        case 2:
            //###
            break;
        default:
            break;
    }
    return NoRole;
}

QRect QAccessibleDockWidget::rect (int child ) const
{
    QRect rect;
    if (child == 0) {
        rect = dockWidget()->rect();
    }else if (child == 1) {
        QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout*>(dockWidget()->layout());
        rect = layout->titleArea();
    }else if (child == 2) {
        if (dockWidget()->widget())
            rect = dockWidget()->widget()->geometry();
    }
    if (rect.isNull())
        return rect;

    rect.moveTopLeft(dockWidget()->mapToGlobal(rect.topLeft()));

    return rect;
}

QVariant QAccessibleDockWidget::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

QDockWidget *QAccessibleDockWidget::dockWidget() const
{
    return static_cast<QDockWidget *>(object());
}

////
//      QAccessibleTitleBar
////
QAccessibleTitleBar::QAccessibleTitleBar(QDockWidget *widget)
    : m_dockWidget(widget)
{

}

int QAccessibleTitleBar::navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const
{
    if (entry == 0 || relation == Self) {
        *iface = new QAccessibleTitleBar(dockWidget());
        return 0;
    }
    switch (relation) {
    case Child:
    case FocusChild:
        if (entry >= 1) {
            QDockWidgetLayout *layout = dockWidgetLayout();
            int index = 1;
            int role;
            for (role = QDockWidgetLayout::CloseButton; role <= QDockWidgetLayout::FloatButton; ++role) {
                QWidget *w = layout->widget((QDockWidgetLayout::Role)role);
                if (!w->isVisible())
                    continue;
                if (index == entry)
                    break;
                ++index;
            }
            
            *iface = 0;
            return role > QDockWidgetLayout::FloatButton ? -1 : index;
        }
        break;
    case Ancestor:
        {
        QAccessibleDockWidget *target = new QAccessibleDockWidget(dockWidget());
        int index;
        if (entry == 1) {
            *iface = target;
            return 0;
        }
        index = target->navigate(Ancestor, entry - 1, iface);
        delete target;
        return index;

        break;}
    case Sibling:
        return navigate(Child, entry, iface);
        break;
    }
    *iface = 0;
    return -1;
}

QAccessible::Relation QAccessibleTitleBar::relationTo(int child,  const QAccessibleInterface * other, int otherChild) const
{
    return Unrelated;   //###
}

int QAccessibleTitleBar::indexOfChild(const QAccessibleInterface * /*child*/) const
{
    return -1;
}

int QAccessibleTitleBar::childCount() const
{
    QDockWidgetLayout *layout = dockWidgetLayout();
    int count = 0;
    for (int role = QDockWidgetLayout::CloseButton; role <= QDockWidgetLayout::FloatButton; ++role) {
        QWidget *w = layout->widget((QDockWidgetLayout::Role)role);
        if (w && w->isVisible())
            ++count;
    }
    return count;
}

QString QAccessibleTitleBar::text(Text t, int child) const
{
    if (!child) {
        if (t == Value) {
            return dockWidget()->windowTitle();
        }
    }
    return QString::fromAscii("QAccessibleTitleBar::implement me. Child :%1").arg(child);
}

QAccessible::State QAccessibleTitleBar::state(int child) const
{
    return Normal;
}

QRect QAccessibleTitleBar::rect (int child ) const
{
    QRect rect;
    if (child == 0) {
        QDockWidgetLayout *layout = dockWidgetLayout();
        rect = layout->titleArea();
    }else if (child >= 1 && child <= childCount()) {
        QDockWidgetLayout *layout = dockWidgetLayout();
        int index = 1;
        for (int role = QDockWidgetLayout::CloseButton; role <= QDockWidgetLayout::FloatButton; ++role) {
            QWidget *w = layout->widget((QDockWidgetLayout::Role)role);
            if (!w || !w->isVisible())
                continue;
            if (index == child) {
                rect = w->geometry();
                break;
            }
            ++index;
        }
    }
    if (rect.isNull())
        return rect;

    rect.moveTopLeft(dockWidget()->mapToGlobal(rect.topLeft()));
    return rect;
}

int QAccessibleTitleBar::childAt(int x, int y) const
{
    for (int i = childCount(); i >= 1; --i) {
        if (rect(i).contains(x,y))
            return i;
    }
    return -1;
}

QObject *QAccessibleTitleBar::object() const
{
    return m_dockWidget;
}

QDockWidgetLayout *QAccessibleTitleBar::dockWidgetLayout() const
{
    return qobject_cast<QDockWidgetLayout*>(dockWidget()->layout());
}

QDockWidget *QAccessibleTitleBar::dockWidget() const
{
    return m_dockWidget;
}

QString QAccessibleTitleBar::actionText(int action, Text t, int child) const
{
    return QString::fromAscii("QAccessibleTitleBar::implement me. Child: %1").arg(child);
}

bool QAccessibleTitleBar::doAction(int action, int child, const QVariantList& params)
{
    return false;
}

int QAccessibleTitleBar::userActionCount ( int child) const
{
    return 0;
}

QAccessible::Role QAccessibleTitleBar::role(int child) const
{
    switch (child) {
        case 0:
            return TitleBar;
            break;
        default:
            if (child >= 1 && child <= childCount())
                return PushButton;
            break;
    }

    return NoRole;
}

void QAccessibleTitleBar::setText(Text t, int child, const QString &text)
{

}

bool QAccessibleTitleBar::isValid() const
{
    return dockWidget();
}

#endif // QT_NO_DOCKWIDGET

#endif // QT_NO_ACCESSIBILITY

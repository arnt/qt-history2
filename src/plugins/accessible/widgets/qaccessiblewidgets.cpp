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
    const QWorkspace *workspace = qobject_cast<QWorkspace *>(area);
    if (!mdiArea && !workspace)
        return 0;

    QWidgetList windows;
    if (mdiArea) {
        foreach (QMdiSubWindow *window, mdiArea->subWindowList())
            windows.append(window);
    } else {
        foreach (QWidget *window, workspace->windowList())
            windows.append(window->parentWidget());
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

    if (workspace) {
        foreach (QWidget *widget, workspace->windowList()) {
            if (widget->parentWidget() == target)
                target = widget;
        }
    }
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

    QPoint point = edit->viewport()->mapFromGlobal(QPoint(x, y));
    QTextBlock block = edit->cursorForPosition(point).block();
    if (block.isValid())
        return qTextBlockPosition(block) + childOffset;

    return QAccessibleWidgetEx::childAt(x, y);
}

/*! \reimp */
QString QAccessibleTextEdit::text(Text t, int child) const
{
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
    if (textType != Value || child <= 0 || child > toolBox()->count())
        return QAccessibleWidgetEx::text(textType, child);
    return toolBox()->itemText(child - 1);
}

void QAccessibleToolBox::setText(Text textType, int child, const QString &text)
{
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
    if (textType == QAccessible::Name && (child == 0 || child == 1)) {
        QString title = mdiSubWindow()->windowTitle();
        title.replace(QLatin1String("[*]"), QLatin1String(""));
        return title;
    }
    return QAccessibleWidgetEx::text(textType, child);
}

void QAccessibleMdiSubWindow::setText(Text textType, int child, const QString &text)
{
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
    if (!mdiSubWindow()->parent())
        return QAccessibleWidgetEx::navigate(relation, entry, target);

    *target = 0;
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

#endif // QT_NO_ACCESSIBILITY

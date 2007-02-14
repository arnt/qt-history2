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
#include <QAbstractScrollArea>

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

static inline void removeInvisibleWidgetsFromList(QWidgetList *list)
{
    if (!list || list->isEmpty())
        return;

    for (int i = 0; i < list->count(); ++i) {
        QWidget *widget = list->at(i);
        if (!widget->isVisible())
            list->removeAt(i);
    }
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

#ifndef QT_NO_SCROLLAREA
QAccessibleAbstractScrollArea::QAccessibleAbstractScrollArea(QWidget *widget)
    : QAccessibleWidgetEx(widget, ScrollBar)
{
    Q_ASSERT(qobject_cast<QAbstractScrollArea *>(widget));
}

QString QAccessibleAbstractScrollArea::text(Text textType, int child) const
{
    if (!abstractScrollArea()->isVisible())
        return QString();
    if (child == Self)
        return QAccessibleWidgetEx::text(textType, 0);
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return QString();
    QAccessibleInterface *childInterface = queryAccessibleInterface(children.at(child - 1));
    if (!childInterface)
        return QString();
    QString string = childInterface->text(textType, 0);
    delete childInterface;
    return string;
}

void QAccessibleAbstractScrollArea::setText(Text textType, int child, const QString &text)
{
    if (!abstractScrollArea()->isVisible() || text.isEmpty())
        return;
    if (child == 0) {
        QAccessibleWidgetEx::setText(textType, 0, text);
        return;
    }
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return;
    QAccessibleInterface *childInterface = queryAccessibleInterface(children.at(child - 1));
    if (!childInterface)
        return;
    childInterface->setText(textType, 0, text);
    delete childInterface;
}

QAccessible::State QAccessibleAbstractScrollArea::state(int child) const
{
    if (child == Self)
        return QAccessibleWidgetEx::state(child);
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return QAccessibleWidgetEx::state(Self);
    QAccessibleInterface *childInterface = queryAccessibleInterface(children.at(child - 1));
    if (!childInterface)
        return QAccessibleWidgetEx::state(Self);
    QAccessible::State returnState = childInterface->state(0);
    delete childInterface;
    return returnState;
}

QVariant QAccessibleAbstractScrollArea::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

int QAccessibleAbstractScrollArea::childCount() const
{
    if (!abstractScrollArea()->isVisible())
        return 0;
    return accessibleChildren().count();
}

int QAccessibleAbstractScrollArea::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child || !child->object() || !abstractScrollArea()->isVisible())
        return -1;
    int index = accessibleChildren().indexOf(qobject_cast<QWidget *>(child->object()));
    if (index >= 0)
        return ++index;
    return -1;
}

int QAccessibleAbstractScrollArea::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    if (!abstractScrollArea()->isVisible() || !target)
        return -1;

    *target = 0;
    QWidgetList children = accessibleChildren();
    if (entry < 0 || entry > children.count())
        return -1;

    QWidget *targetWidget = 0;
    QWidget *entryWidget = 0;
    if (entry == 0)
        entryWidget = abstractScrollArea();
    else if (entry > 0)
        entryWidget = children.at(entry - 1);
    AbstractScrollAreaElement entryElement = elementType(entryWidget);

    // Widgets to the left for the horizontal scrollBar.
    QWidgetList leftScrollBarWidgets = abstractScrollArea()
        ->scrollBarWidgets(isLeftToRight() ? Qt::AlignLeft : Qt::AlignRight);
    removeInvisibleWidgetsFromList(&leftScrollBarWidgets);

    // Widgets to the right for the horizontal scrollBar.
    QWidgetList rightScrollBarWidgets = abstractScrollArea()
        ->scrollBarWidgets(isLeftToRight() ? Qt::AlignRight : Qt::AlignLeft);
    removeInvisibleWidgetsFromList(&rightScrollBarWidgets);

    // Widgets above the vertical scrollBar.
    QWidgetList topScrollBarWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignTop);
    removeInvisibleWidgetsFromList(&topScrollBarWidgets);

    // Widgets below the vertical scrollBar.
    QWidgetList bottomScrollBarWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignBottom);
    removeInvisibleWidgetsFromList(&bottomScrollBarWidgets);

    // Not one of the most beautiful switches I've ever seen, but I believe it has
    // to be like this since each case need special handling.
    // It might be possible to make it more general, but I'll leave that as an exercise
    // to the reader. :-)
    switch (relation) {
    case Child:
        if (entry > 0)
            targetWidget = children.at(entry - 1);
        break;
    case Left:
        if (entry < 1)
            break;
        switch (entryElement) {
        case Viewport:
            if (!isLeftToRight())
                targetWidget = abstractScrollArea()->verticalScrollBar();
            break;
        case HorizontalScrollBar:
            if (leftScrollBarWidgets.count() >= 1)
                targetWidget = leftScrollBarWidgets.back();
            else if (!isLeftToRight())
                targetWidget = abstractScrollArea()->cornerWidget();
            break;
        case VerticalScrollBar:
        case VerticalScrollBarWidget:
            if (isLeftToRight())
                targetWidget = abstractScrollArea()->viewport();
            break;
        case CornerWidget:
            if (!isLeftToRight())
                break;
            if (rightScrollBarWidgets.count() >= 1)
                targetWidget = rightScrollBarWidgets.back();
            else
                targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        case HorizontalScrollBarWidget: {
            int index = leftScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index > 0)
                    targetWidget = leftScrollBarWidgets.at(index - 1);
                else if (!isLeftToRight())
                    targetWidget = abstractScrollArea()->cornerWidget();
                break;
            }
            index = rightScrollBarWidgets.indexOf(entryWidget);
            if (index > 0)
                targetWidget = rightScrollBarWidgets.at(index - 1);
            else
                targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        }
        default:
            break;
        }
        break;
    case Right:
        if (entry < 1)
            break;
        switch (entryElement) {
        case Viewport:
            if (isLeftToRight())
                targetWidget = abstractScrollArea()->verticalScrollBar();
            break;
        case HorizontalScrollBar:
            if (rightScrollBarWidgets.count() >= 1)
                targetWidget = rightScrollBarWidgets.at(0);
            else
                targetWidget = abstractScrollArea()->cornerWidget();
            break;
        case VerticalScrollBar:
        case VerticalScrollBarWidget:
            if (!isLeftToRight())
                targetWidget = abstractScrollArea()->viewport();
            break;
        case CornerWidget:
            if (isLeftToRight())
                break;
            if (leftScrollBarWidgets.count() >= 1)
                targetWidget = rightScrollBarWidgets.at(0);
            else
                targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        case HorizontalScrollBarWidget: {
            int index = leftScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index < leftScrollBarWidgets.count() - 1)
                    targetWidget = leftScrollBarWidgets.at(index + 1);
                break;
            }
            index = rightScrollBarWidgets.indexOf(entryWidget);
            if (index != -1 && index < rightScrollBarWidgets.count() - 1) {
                targetWidget = rightScrollBarWidgets.at(index + 1);
                break;
            }
            if (isLeftToRight())
                targetWidget = abstractScrollArea()->cornerWidget();
            break;
        }
        default:
            break;
        }
        break;
    case Up:
        if (entry < 1)
            break;
        switch (entryElement) {
        case HorizontalScrollBar:
        case HorizontalScrollBarWidget:
            targetWidget = abstractScrollArea()->viewport();
            break;
        case VerticalScrollBar: {
            if (topScrollBarWidgets.count() >= 1)
                targetWidget = topScrollBarWidgets.back();
            break;
        }
        case VerticalScrollBarWidget: {
            int index = topScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index > 0)
                    targetWidget = topScrollBarWidgets.at(index - 1);
                break;
            }
            index = bottomScrollBarWidgets.indexOf(entryWidget);
            if (index > 0) {
                targetWidget = bottomScrollBarWidgets.at(index - 1);
                break;
            }
            targetWidget = abstractScrollArea()->verticalScrollBar();
            break;
        }
        case CornerWidget:
            if (bottomScrollBarWidgets.count() >= 1)
                targetWidget = bottomScrollBarWidgets.back();
            break;
        default:
            break;
        }
        break;
    case Down:
        if (entry < 1)
            break;
        switch (entryElement) {
        case Viewport:
            targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        case VerticalScrollBar:
            if (bottomScrollBarWidgets.count() >= 1)
                targetWidget = bottomScrollBarWidgets.at(0);
            break;
        case VerticalScrollBarWidget: {
            int index = topScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index < topScrollBarWidgets.count() - 1)
                    targetWidget = topScrollBarWidgets.at(index + 1);
                else
                    targetWidget = abstractScrollArea()->verticalScrollBar();
                break;
            }
            index = bottomScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index < bottomScrollBarWidgets.count() - 1)
                    targetWidget = bottomScrollBarWidgets.at(index + 1);
                break;
            }
            targetWidget = abstractScrollArea()->cornerWidget();
            break;
        }
        default:
            break;
        }
        break;
    default:
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    }

    if (targetWidget && !targetWidget->isVisible())
        return -1;
    *target = QAccessible::queryAccessibleInterface(targetWidget);
    return indexOfChild(*target);
}

QRect QAccessibleAbstractScrollArea::rect(int child) const
{
    if (!abstractScrollArea()->isVisible())
        return QRect();
    if (child == Self)
        return QAccessibleWidgetEx::rect(child);
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return QRect();
    const QWidget *childWidget = children.at(child - 1);
    if (!childWidget->isVisible())
        return QRect();
    return QRect(childWidget->mapToGlobal(QPoint(0, 0)), childWidget->size());
}

int QAccessibleAbstractScrollArea::childAt(int x, int y) const
{
    if (!abstractScrollArea()->isVisible())
        return -1;
    const QRect globalSelfGeometry = rect(Self);
    if (!globalSelfGeometry.isValid() || !globalSelfGeometry.contains(QPoint(x, y)))
        return -1;
    const QWidgetList children = accessibleChildren();
    for (int i = 0; i < children.count(); ++i) {
        const QWidget *child = children.at(i);
        const QRect globalChildGeometry = QRect(child->mapToGlobal(QPoint(0, 0)), child->size());
        if (globalChildGeometry.contains(QPoint(x, y)))
            return ++i;
    }
    return 0;
}

QAbstractScrollArea *QAccessibleAbstractScrollArea::abstractScrollArea() const
{
    return static_cast<QAbstractScrollArea *>(object());
}

QWidgetList QAccessibleAbstractScrollArea::accessibleChildren() const
{
    QWidgetList children;

    // Viewport.
    QWidget * viewport = abstractScrollArea()->viewport();
    if (viewport && viewport->isVisible())
        children.append(viewport);

    // Horizontal scrollBar and all its scrollWidgets.
    QScrollBar *horizontalScrollBar = abstractScrollArea()->horizontalScrollBar();
    if (horizontalScrollBar && horizontalScrollBar->isVisible()) {
        children.append(horizontalScrollBar);
        QWidgetList scrollWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignLeft | Qt::AlignRight);
        foreach (QWidget *widget, scrollWidgets) {
            if (widget->isVisible())
                children.append(widget);
        }
    }

    // Vertical scrollBar and all its scrollWidgets.
    QScrollBar *verticalScrollBar = abstractScrollArea()->verticalScrollBar();
    if (verticalScrollBar && verticalScrollBar->isVisible()) {
        children.append(verticalScrollBar);
        QWidgetList scrollWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignTop | Qt::AlignBottom);
        foreach (QWidget *widget, scrollWidgets) {
            if (widget->isVisible())
                children.append(widget);
        }
    }

    // CornerWidget.
    QWidget *cornerWidget = abstractScrollArea()->cornerWidget();
    if (cornerWidget && cornerWidget->isVisible())
        children.append(cornerWidget);

    return children;
}

QAccessibleAbstractScrollArea::AbstractScrollAreaElement
QAccessibleAbstractScrollArea::elementType(QWidget *widget) const
{
    if (!widget)
        return Undefined;

    if (widget == abstractScrollArea())
        return Self;
    if (widget == abstractScrollArea()->viewport())
        return Viewport;
    if (widget == abstractScrollArea()->horizontalScrollBar())
        return HorizontalScrollBar;
    if (widget == abstractScrollArea()->verticalScrollBar())
        return VerticalScrollBar;
    if (widget == abstractScrollArea()->cornerWidget())
        return CornerWidget;

    // Check horizontal scrollBar widgets.
    QWidgetList list = abstractScrollArea()->scrollBarWidgets(Qt::AlignLeft | Qt::AlignRight);
    foreach (QWidget *candidate, list) {
        if (widget == candidate)
            return HorizontalScrollBarWidget;
    }

    // Check vertical scrollBar widgets.
    list = abstractScrollArea()->scrollBarWidgets(Qt::AlignTop | Qt::AlignBottom);
    foreach (QWidget *candidate, list) {
        if (widget == candidate)
            return VerticalScrollBarWidget;
    }

    return Undefined;
}

bool QAccessibleAbstractScrollArea::isLeftToRight() const
{
    return abstractScrollArea()->isLeftToRight();
}
#endif // QT_NO_SCROLLAREA


#endif // QT_NO_ACCESSIBILITY

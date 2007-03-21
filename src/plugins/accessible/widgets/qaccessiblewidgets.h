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

#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <QtGui/qaccessible2.h>
#include <QtGui/qaccessiblewidget.h>

#if !defined(QT_NO_ACCESSIBILITY) && !defined(QT_NO_TEXTEDIT)

#include <QtCore/QPointer>

class QTextEdit;
class QStackedWidget;
class QToolBox;
class QMdiArea;
class QMdiSubWindow;
class QWorkspace;
class QRubberBand;
class QTextBrowser;
class QCalendarWidget;
class QAbstractItemView;
class QDockWidget;
class QDockWidgetLayout;
class QMainWindow;

class QAccessibleTextEdit : public QAccessibleWidgetEx, public QAccessibleTextInterface,
                            public QAccessibleEditableTextInterface
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessibleTextEdit(QWidget *o);

    QString text(Text t, int child) const;
    void setText(Text t, int control, const QString &text);
    Role role(int child) const;

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);

    QRect rect(int child) const;
    int childAt(int x, int y) const;

    int childCount() const;

    // QAccessibleTextInterface
    void addSelection(int startOffset, int endOffset);
    QString attributes(int offset, int *startOffset, int *endOffset);
    int cursorPosition();
    QRect characterRect(int offset, QAccessible2::CoordinateType coordType);
    int selectionCount();
    int offsetAtPoint(const QPoint &point, QAccessible2::CoordinateType coordType);
    void selection(int selectionIndex, int *startOffset, int *endOffset);
    QString text(int startOffset, int endOffset);
    QString textBeforeOffset (int offset, QAccessible2::BoundaryType boundaryType,
            int *startOffset, int *endOffset);
    QString textAfterOffset(int offset, QAccessible2::BoundaryType boundaryType,
            int *startOffset, int *endOffset);
    QString textAtOffset(int offset, QAccessible2::BoundaryType boundaryType,
            int *startOffset, int *endOffset);
    void removeSelection(int selectionIndex);
    void setCursorPosition(int position);
    void setSelection(int selectionIndex, int startOffset, int endOffset);
    int characterCount();
    void scrollToSubstring(int startIndex, int endIndex);

    // QAccessibleEditableTextInterface
    void copyText(int startOffset, int endOffset);
    void deleteText(int startOffset, int endOffset);
    void insertText(int offset, const QString &text);
    void cutText(int startOffset, int endOffset);
    void pasteText(int offset);
    void replaceText(int startOffset, int endOffset, const QString &text);
    void setAttributes(int startOffset, int endOffset, const QString &attributes);

protected:
    QTextEdit *textEdit() const;

private:
    int childOffset;
};

class QAccessibleStackedWidget : public QAccessibleWidgetEx
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessibleStackedWidget(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childAt(int x, int y) const;
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QStackedWidget *stackedWidget() const;
};

class QAccessibleToolBox : public QAccessibleWidgetEx
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessibleToolBox(QWidget *widget);

    QString text(Text textType, int child) const;
    void setText(Text textType, int child, const QString &text);
    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QToolBox *toolBox() const;
};

class QAccessibleMdiArea : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMdiArea(QWidget *widget);

    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QMdiArea *mdiArea() const;
};

class QAccessibleMdiSubWindow : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMdiSubWindow(QWidget *widget);

    QString text(Text textType, int child) const;
    void setText(Text textType, int child, const QString &text);
    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;
    QRect rect(int child) const;
    int childAt(int x, int y) const;

protected:
    QMdiSubWindow *mdiSubWindow() const;
};

#ifndef QT_NO_WORKSPACE
class QAccessibleWorkspace : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleWorkspace(QWidget *widget);

    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QWorkspace *workspace() const;
};
#endif

class QAccessibleDialogButtonBox : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleDialogButtonBox(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
};

#ifndef QT_NO_TEXTBROWSER
class QAccessibleTextBrowser : public QAccessibleTextEdit
{
public:
    explicit QAccessibleTextBrowser(QWidget *widget);

    Role role(int child) const;
};
#endif // QT_NO_TEXTBROWSER

#ifndef QT_NO_CALENDARWIDGET
class QAccessibleCalendarWidget : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleCalendarWidget(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;
    QRect rect(int child) const;
    int childAt(int x, int y) const;

protected:
    QCalendarWidget *calendarWidget() const;

private:
    QAbstractItemView *calendarView() const;
    QWidget *navigationBar() const;
};
#endif // QT_NO_CALENDARWIDGET

#ifndef QT_NO_DOCKWIDGET
class QAccessibleDockWidget: public QAccessibleWidgetEx
{
public:
    explicit QAccessibleDockWidget(QWidget *widget);
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int childCount() const;
    QRect rect (int child ) const;
    Role role(int child) const;
    State state(int child) const;
    int childAt(int x, int y) const;

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    QDockWidget *dockWidget() const;
};

class QAccessibleTitleBar : public QAccessibleInterface
{
public:
    explicit QAccessibleTitleBar(QDockWidget *widget);
    QString actionText(int action, Text t, int child) const;
    bool doAction(int action, int child, const QVariantList& params = QVariantList());
    int userActionCount ( int child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int childCount() const;
    Relation relationTo(int child,  const QAccessibleInterface *other, int otherChild) const;
    void setText(Text t, int child, const QString &text);
    QString text(Text t, int child) const;
    Role role(int child) const;
    QRect rect (int child) const;
    State state(int child) const;
    int childAt(int x, int y) const;
    QObject *object() const;
    bool isValid() const;


    QPointer<QDockWidget> m_dockWidget;

    QDockWidget *dockWidget() const;
    QDockWidgetLayout *dockWidgetLayout() const;
};

#endif // QT_NO_DOCKWIDGET


#ifndef QT_NO_MAINWINDOW
class QAccessibleMainWindow : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMainWindow(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const;
    int indexOfChild(const QAccessibleInterface *iface) const;
    QMainWindow *mainWindow() const;

};
#endif //QT_NO_MAINWINDOW

#endif // QT_NO_ACCESSIBILITY

#endif // QACESSIBLEWIDGETS_H

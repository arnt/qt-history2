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

#ifndef QACCESSIBLE2_H
#define QACCESSIBLE2_H

#include <QtGui/qaccessible.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_ACCESSIBILITY

namespace QAccessible2
{
    enum CoordinateType
    {
        RelativeToScreen = 0,
        RelativeToParent = 1
    };

    enum BoundaryType {
        CharBoundary,
        WordBoundary,
        SentenceBoundary,
        ParagraphBoundary,
        LineBoundary,
        NoBoundary
    };
}

class Q_GUI_EXPORT QAccessible2Interface
{
public:
    virtual ~QAccessible2Interface() {}
};

// catch-all functions. If an accessible class doesn't implement interface T, return 0
inline QAccessible2Interface *qAccessibleValueCastHelper() { return 0; }
inline QAccessible2Interface *qAccessibleTextCastHelper() { return 0; }
inline QAccessible2Interface *qAccessibleEditableTextCastHelper() { return 0; }
inline QAccessible2Interface *qAccessibleTableCastHelper() { return 0; }

#define Q_ACCESSIBLE_OBJECT \
    public: \
    QAccessible2Interface *interface_cast(QAccessible2::InterfaceType t) \
    { \
        switch (t) { \
        case QAccessible2::TextInterface: \
            return qAccessibleTextCastHelper(); \
        case QAccessible2::EditableTextInterface: \
            return qAccessibleEditableTextCastHelper(); \
        case QAccessible2::ValueInterface: \
            return qAccessibleValueCastHelper(); \
        case QAccessible2::TableInterface: \
            return qAccessibleTableCastHelper(); \
        } \
        return 0; \
    } \
    private:

class Q_GUI_EXPORT QAccessibleTextInterface: public QAccessible2Interface
{
public:
    inline QAccessible2Interface *qAccessibleTextCastHelper() { return this; }

    virtual ~QAccessibleTextInterface() {}

    virtual void addSelection(int startOffset, int endOffset) = 0;
    virtual QString attributes(int offset, int *startOffset, int *endOffset) = 0;
    virtual int cursorPosition() = 0;
    virtual QRect characterRect(int offset, QAccessible2::CoordinateType coordType) = 0;
    virtual int selectionCount() = 0;
    virtual int offsetAtPoint(const QPoint &point, QAccessible2::CoordinateType coordType) = 0;
    virtual void selection(int selectionIndex, int *startOffset, int *endOffset) = 0;
    virtual QString text(int startOffset, int endOffset) = 0;
    virtual QString textBeforeOffset (int offset, QAccessible2::BoundaryType boundaryType,
                              int *startOffset, int *endOffset) = 0;
    virtual QString textAfterOffset(int offset, QAccessible2::BoundaryType boundaryType,
                            int *startOffset, int *endOffset) = 0;
    virtual QString textAtOffset(int offset, QAccessible2::BoundaryType boundaryType,
                         int *startOffset, int *endOffset) = 0;
    virtual void removeSelection(int selectionIndex) = 0;
    virtual void setCursorPosition(int position) = 0;
    virtual void setSelection(int selectionIndex, int startOffset, int endOffset) = 0;
    virtual int characterCount() = 0;
    virtual void scrollToSubstring(int startIndex, int endIndex) = 0;
};

class Q_GUI_EXPORT QAccessibleEditableTextInterface: public QAccessible2Interface
{
public:
    inline QAccessible2Interface *qAccessibleEditableTextCastHelper() { return this; }

    virtual ~QAccessibleEditableTextInterface() {}

    virtual void copyText(int startOffset, int endOffset) = 0;
    virtual void deleteText(int startOffset, int endOffset) = 0;
    virtual void insertText(int offset, const QString &text) = 0;
    virtual void cutText(int startOffset, int endOffset) = 0;
    virtual void pasteText(int offset) = 0;
    virtual void replaceText(int startOffset, int endOffset, const QString &text) = 0;
    virtual void setAttributes(int startOffset, int endOffset, const QString &attributes) = 0;
};

class Q_GUI_EXPORT QAccessibleSimpleEditableTextInterface: public QAccessibleEditableTextInterface
{
public:
    QAccessibleSimpleEditableTextInterface(QAccessibleInterface *accessibleInterface);

    void copyText(int startOffset, int endOffset);
    void deleteText(int startOffset, int endOffset);
    void insertText(int offset, const QString &text);
    void cutText(int startOffset, int endOffset);
    void pasteText(int offset);
    void replaceText(int startOffset, int endOffset, const QString &text);
    inline void setAttributes(int, int, const QString &) {}

private:
    QAccessibleInterface *iface;
};

class Q_GUI_EXPORT QAccessibleValueInterface: public QAccessible2Interface
{
public:
    inline QAccessible2Interface *qAccessibleValueCastHelper() { return this; }

    virtual ~QAccessibleValueInterface() {}

    virtual QVariant currentValue() = 0;
    virtual void setCurrentValue(const QVariant &value) = 0;
    virtual QVariant maximumValue() = 0;
    virtual QVariant minimumValue() = 0;
};

class Q_GUI_EXPORT QAccessibleTableInterface: public QAccessible2Interface
{
public:
    inline QAccessible2Interface *qAccessibleTableCastHelper() { return this; }

    virtual QAccessibleInterface *accessibleAt(int row, int column) = 0;
    virtual QAccessibleInterface *caption() = 0;
    virtual int childIndex(int rowIndex, int columnIndex) = 0;
    virtual QString columnDescription(int column) = 0;
    virtual int columnSpan(int row, int column) = 0;
    virtual QAccessibleInterface *columnHeader() = 0;
    virtual int columnIndex(int childIndex) = 0;
    virtual int columnCount() = 0;
    virtual int rowCount() = 0;
    virtual int selectedColumnCount() = 0;
    virtual int selectedRowCount() = 0;
    virtual QString rowDescription(int row) = 0;
    virtual int rowSpan(int row, int column) = 0;
    virtual QAccessibleInterface *rowHeader() = 0;
    virtual int rowIndex(int childIndex) = 0;
    virtual int selectedRows(int maxRows, QList<int> *rows) = 0;
    virtual int selectedColumns(int maxColumns, QList<int> *columns) = 0;
    virtual QAccessibleInterface *summary() = 0;
    virtual bool isColumnSelected(int column) = 0;
    virtual bool isRowSelected(int row) = 0;
    virtual bool isSelected(int row, int column) = 0;
    virtual void selectRow(int row) = 0;
    virtual void selectColumn(int column) = 0;
    virtual void unselectRow(int row) = 0;
    virtual void unselectColumn(int column) = 0;
    virtual void cellAtIndex(int index, int *row, int *column, int *rowSpan,
                             int *columnSpan, bool *isSelected) = 0;
};

#endif // QT_NO_ACCESSIBILITY

QT_END_HEADER

#endif

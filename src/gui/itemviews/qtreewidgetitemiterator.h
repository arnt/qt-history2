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
#ifndef QTREEWIDGETITEMITERATOR_H
#define QTREEWIDGETITEMITERATOR_H

#include <qglobal.h>

class QTreeWidget;
class QTreeWidgetItem;
class QTreeModel;

class Q_GUI_EXPORT QTreeWidgetItemIterator
{
public:
    enum IteratorFlag {
        All           = 0x00000000,
        Hidden        = 0x00000001,
        NotHidden     = 0x00000002,
        Selected      = 0x00000004,
        Unselected    = 0x00000008,
        Selectable    = 0x00000010,
        NotSelectable = 0x00000020,
        DragEnabled   = 0x00000040,
        DragDisabled  = 0x00000080,
        DropEnabled   = 0x00000100,
        DropDisabled  = 0x00000200,
        HasChildren   = 0x00000400,
        NoChildren    = 0x00000800,
        Checked       = 0x00001000,
        NotChecked    = 0x00002000,
        Enabled       = 0x00004000,
        Disabled      = 0x00008000,
        Editable      = 0x00010000,
        NotEditable   = 0x00020000
    };
    Q_DECLARE_FLAGS(IteratorFlags, IteratorFlag)

    QTreeWidgetItemIterator(const QTreeWidgetItemIterator &it);
    explicit QTreeWidgetItemIterator(QTreeWidget *widget, IteratorFlags flags = All);
    explicit QTreeWidgetItemIterator(QTreeWidgetItem *item, IteratorFlags flags = All);
    ~QTreeWidgetItemIterator();

    QTreeWidgetItemIterator &operator=(const QTreeWidgetItemIterator &it);

    QTreeWidgetItemIterator &operator++();
    inline const QTreeWidgetItemIterator operator++(int);
    inline QTreeWidgetItemIterator &operator+=(int n);

    QTreeWidgetItemIterator &operator--();
    inline const QTreeWidgetItemIterator operator--(int);
    inline QTreeWidgetItemIterator &operator-=(int n);

    inline QTreeWidgetItem *operator*() const;

protected:
    bool matchesFlags(const QTreeWidgetItem *item) const;
    QTreeModel *model;
    QTreeWidgetItem *current;
    IteratorFlags flags;
};

inline const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator++(int)
{
    QTreeWidgetItemIterator it = *this;
    ++(*this);
    return it;
}

inline const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator--(int)
{
    QTreeWidgetItemIterator it = *this;
    --(*this);
    return it;
}

inline QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator+=(int n)
{
    if (n < 0)
        return (*this) -= (-n);
    while (current && n++)
        ++(*this);
    return *this;
}

inline QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator-=(int n)
{
    if (n < 0)
        return (*this) += (-n);
    while (current && n--)
        --(*this);
    return *this;
}

inline QTreeWidgetItem *QTreeWidgetItemIterator::operator*() const
{
    return current;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QTreeWidgetItemIterator::IteratorFlags)

#endif // QTREEWIDGETITEMITERATOR_H

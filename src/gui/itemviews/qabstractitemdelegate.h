/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#ifndef QT_H
#include <qobject.h>
#include <qstyleoption.h>
#endif

class QPainter;
class QModelIndex;
class QAbstractItemModel;
class QAbstractItemDelegatePrivate;

class Q_GUI_EXPORT QAbstractItemDelegate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractItemDelegate)
    Q_ENUMS(EditorType EndEditFlags)
    Q_FLAGS(StartEditFlags)

public:
    QAbstractItemDelegate(QAbstractItemModel *model, QObject *parent = 0);
    virtual ~QAbstractItemDelegate();

    QAbstractItemModel *model() const;

    enum EditorType {
        Widget,
        Events
    };

    enum BeginEditAction {
        NeverEdit = 0,
        CurrentChanged = 1,
        DoubleClicked = 2,
        SelectedClicked = 4,
        EditKeyPressed = 8,
        AnyKeyPressed = 16,
        AlwaysEdit = 32
    };

    Q_DECLARE_FLAGS(BeginEditActions, BeginEditAction);

    enum EndEditAction {
        Accepted = 1,
        Cancelled = 2
    };

    // painting
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const = 0;
    virtual QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const = 0;

    // editing
    virtual EditorType editorType(const QModelIndex &index) const;
    virtual QWidget *editor(BeginEditAction action, QWidget *parent,
                            const QStyleOptionViewItem &option, const QModelIndex &index);
    virtual void setModelData(QWidget *editor, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;
    virtual void releaseEditor(EndEditAction action, QWidget *editor, const QModelIndex &index);

    // events for non-widget editors
    virtual bool event(QEvent *e, const QModelIndex &index);

protected:
    QAbstractItemDelegate(QAbstractItemDelegatePrivate &, QAbstractItemModel* model,
                          QObject *parent = 0);
    QString ellipsisText(const QFontMetrics &fontMetrics, int width, int align,
                         const QString &org) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemDelegate::BeginEditActions);

#endif

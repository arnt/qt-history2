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

#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#ifndef QT_H
#include <qobject.h>
#include <qstyleoption.h>
#endif

class QPainter;
class QModelIndex;
class QAbstractItemModel;

class Q_GUI_EXPORT QAbstractItemDelegate : public QObject
{
    Q_OBJECT
    Q_ENUMS(EditorType EndEditFlags)
    Q_FLAGS(StartEditFlags)

public:
    QAbstractItemDelegate(QObject *parent = 0);
    virtual ~QAbstractItemDelegate();

    enum EditorType {
        Events,
        Widget
    };

    enum BeginEditAction {
        NeverEdit = 0,
        CurrentChanged = 1,
        DoubleClicked = 2,
        SelectedClicked = 4,
        EditKeyPressed = 8,
        AnyKeyPressed = 16,
        AlwaysEdit = 31
    };

    Q_DECLARE_FLAGS(BeginEditActions, BeginEditAction);

    enum EndEditAction {
        Accepted = 1,
        Cancelled = 2
    };

    // painting
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QAbstractItemModel *model, const QModelIndex &index) const = 0;
    virtual QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                           const QAbstractItemModel *model, const QModelIndex &index) const = 0;

    // editing
    virtual EditorType editorType(const QAbstractItemModel *model, const QModelIndex &index) const;
    virtual QWidget *editor(BeginEditAction action, QWidget *parent,
                            const QStyleOptionViewItem &option,
                            const QAbstractItemModel *model,
                            const QModelIndex &index);
    virtual void releaseEditor(EndEditAction action, QWidget *editor,
                               QAbstractItemModel *model, const QModelIndex &index);

    virtual void setEditorData(QWidget *editor,
                               const QAbstractItemModel *model,
                               const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const;

    virtual void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QAbstractItemModel* model,
                                      const QModelIndex &index) const;

    // events for non-widget editors
    virtual bool event(QEvent *e, QAbstractItemModel* model, const QModelIndex &index);

signals:
    void doneEditing(QWidget *editor, QAbstractItemDelegate::EndEditAction action);

protected:
    QString ellipsisText(const QFontMetrics &fontMetrics, int width, int align,
                         const QString &org) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemDelegate::BeginEditActions);

#endif

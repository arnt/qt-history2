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

#ifndef DELEGATE_H
#define DELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>

class SpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SpinBoxDelegate(QObject *parent = 0);

    QItemDelegate::EditorType editorType(const QAbstractItemModel *model,
        const QModelIndex &index) const;

    QWidget *editor(QWidget *parent, const QStyleOptionViewItem &option,
                    const QAbstractItemModel *model, const QModelIndex &index);

    void releaseEditor(QWidget *editor);

    void setEditorData(QWidget *editor, const QAbstractItemModel *model,
                       const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QAbstractItemModel* model,
        const QModelIndex &index) const;

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    void acceptEditing(QWidget *editor);
    void cancelEditing(QWidget *editor);

    QSpinBox *spinBox;
};

#endif

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

#ifndef QITEMDELEGATE_H
#define QITEMDELEGATE_H

#include <QtGui/qabstractitemdelegate.h>
#include <QtCore/qstring.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qvariant.h>

class QItemDelegatePrivate;
class QItemEditorFactory;

class Q_GUI_EXPORT QItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit QItemDelegate(QObject *parent = 0);
    ~QItemDelegate();

    // painting
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

    // editing
    QWidget *editor(QWidget *parent,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;

    void releaseEditor(QWidget *editor, const QModelIndex &index);
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

    // editor factory
    QItemEditorFactory *itemEditorFactory() const;
    void setItemEditorFactory(QItemEditorFactory *factory);

protected:
    virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                             const QRect &rect, const QString &text) const;
    virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QPixmap &pixmap) const;
    virtual void drawFocus(QPainter *painter, const QStyleOptionViewItem &option,
                           const QRect &rect) const;
    virtual void drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
                           const QRect &rect, Qt::CheckState state) const;

    void doLayout(const QStyleOptionViewItem &option,
                  QRect *checkRect, QRect *iconRect, QRect *textRect, bool hint) const;
    void doAlignment(Qt::LayoutDirection direction, const QRect &boundingRect, int alignment,
                     QRect *rect) const;
    QPixmap decoration(const QStyleOptionViewItem &option, const QVariant &variant) const;
    QPixmap *selected(const QPixmap &pixmap, const QPalette &palette, bool enabled) const;
    QRect check(const QStyleOptionViewItem &option, const QVariant &variant) const;

    bool eventFilter(QObject *object, QEvent *event);
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    Q_DECLARE_PRIVATE(QItemDelegate)
    Q_DISABLE_COPY(QItemDelegate)
};

#endif // QITEMDELEGATE_H

#ifndef QITEMDELEGATE_H
#define QITEMDELEGATE_H

#include <qabstractitemdelegate.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qvariant.h>

class Q_GUI_EXPORT QItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    QItemDelegate(QAbstractItemModel *model, QObject *parent = 0);
    ~QItemDelegate();

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

    // editing
    QAbstractItemDelegate::EditorType editorType(const QModelIndex &index) const;
    QWidget *editor(QAbstractItemDelegate::BeginEditAction action, QWidget *parent,
                    const QStyleOptionViewItem &option, const QModelIndex &index);
    void releaseEditor(EndEditAction action, QWidget *editor, const QModelIndex &index);

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

protected:
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect,
                     const QString &text) const;
    void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect,
                        const QPixmap &pixmap) const;
    void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;
    void doLayout(const QStyleOptionViewItem &option, QRect *iconRect, QRect *textRect,
                  bool hint) const;
    void doAlignment(const QRect &boundingRect, int alignment, QRect *rect) const;
    QPixmap decoration(const QStyleOptionViewItem &option, const QVariant &variant) const;
};

#endif

#include "qitemdelegate.h"
#include <qabstractitemview.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qpoint.h>
#include <qrect.h>

QItemDelegate::QItemDelegate(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemDelegate(model, parent), spacing(4)
{
}

QItemDelegate::~QItemDelegate()
{
}

void QItemDelegate::paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const
{
    static QPoint pt(0, 0);
    QString text = model()->data(item, QAbstractItemModel::Display).toString();
    QIconSet icons = model()->data(item, QAbstractItemModel::Decoration).toIconSet();

    QRect iconRect(pt, iconSize(options, icons));
    QRect textRect(pt, textSize(painter->fontMetrics(), options, text));
    doLayout(options, &iconRect, &textRect, false);

    drawIcon(painter, options, iconRect, icons);
    drawText(painter, options, textRect, text);
    drawFocus(painter, options, textRect);
}

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
                              const QModelIndex &item) const
{
    static QPoint pt(0, 0);
    QRect iconRect(pt, iconSize(options,
                                model()->data(item, QAbstractItemModel::Decoration).toIconSet()));
    QRect textRect(pt, textSize(fontMetrics, options,
                                model()->data(item, QAbstractItemModel::Display).toString()));
    doLayout(options, &iconRect, &textRect, true); // makes the rects valid
    return iconRect.unite(textRect).size();
}

QItemDelegate::EditType QItemDelegate::editType(const QModelIndex &) const
{
    return WidgetOnTyping;
}

QWidget *QItemDelegate::createEditor(StartEditAction action, QWidget *parent,
                                     const QItemOptions &options, const QModelIndex &item) const
{
    if (item.type() != QModelIndex::View)
        return 0;
    if (action & (EditKeyPressed | AnyKeyPressed | DoubleClicked)) {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setFrame(false);
        lineEdit->setText(model()->data(item, QAbstractItemModel::Edit).toString());
        lineEdit->selectAll();
        updateEditorGeometry(lineEdit, options, item);
        return lineEdit;
    }
    return 0;
}

void QItemDelegate::setContentFromEditor(QWidget *editor, const QModelIndex &item) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit)
        model()->setData(item, QAbstractItemModel::Edit, lineEdit->text());
}

void QItemDelegate::updateEditorContents(QWidget *editor, const QModelIndex &item) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit)
        lineEdit->setText(model()->data(item, QAbstractItemModel::Edit).toString());
}

void QItemDelegate::updateEditorGeometry(QWidget *editor, const QItemOptions &options, const QModelIndex &item) const
{
    static QPoint pt(0, 0);
    if (editor) {
        QRect iconRect(pt, iconSize(options,
                                    model()->data(item, QAbstractItemModel::Decoration).toIconSet()));
        QRect textRect(pt, textSize(editor->fontMetrics(), options,
                                    model()->data(item, QAbstractItemModel::Display).toString()));
        doLayout(options, &iconRect, &textRect, false); // makes the rects valid
        editor->setGeometry(textRect);
    }
}

void QItemDelegate::drawText(QPainter *painter, const QItemOptions &options, const QRect &rect,
                              const QString &text) const
{
    QPen old = painter->pen();
    if (options.selected) {
        painter->fillRect(rect, options.palette.highlight());
        painter->setPen(options.palette.highlightedText());
    } else {
        painter->setPen(options.palette.text());
    }
    if (options.iconAlignment & Qt::AlignLeft) {
        painter->drawText(QRect(rect.x() + spacing, rect.y(), rect.width() - spacing, rect.height()),
                          options.textAlignment, text);
    } else {
        painter->drawText(rect, options.textAlignment, text);
    }
    painter->setPen(old);
}

void QItemDelegate::drawIcon(QPainter *painter, const QItemOptions &options, const QRect &rect,
                              const QIconSet &icons) const
{
    if (options.selected)
        if (options.smallItem)
            painter->fillRect(rect, options.palette.highlight());
        else
            painter->fillRect(rect, QBrush(options.palette.highlight(), QBrush::Dense4Pattern));
    QIconSet::Mode mode = options.disabled ? QIconSet::Disabled : QIconSet::Normal; // FIXME: open
    QIconSet::Size size = options.smallItem ? QIconSet::Small : QIconSet::Large;
    QIconSet::State state = options.selected ? QIconSet::On : QIconSet::Off;
    painter->drawPixmap(rect.topLeft(), icons.pixmap(size, mode, state));
}

void QItemDelegate::drawFocus(QPainter *painter, const QItemOptions &options, const QRect &rect) const
{
    if (options.focus)
        QApplication::style().drawPrimitive(QStyle::PE_FocusRect, painter, rect, options.palette);
}

void QItemDelegate::doLayout(const QItemOptions &options, QRect *iconRect, QRect *textRect, bool hint) const
{
    if (iconRect && textRect) {
        int x = options.itemRect.left();
        int y = options.itemRect.top();
        if (options.iconAlignment & Qt::AlignLeft) {
            int height = hint ? qMax(textRect->height(), iconRect->height()) : options.itemRect.height();
            QRect leftRect(x, y, iconRect->width(), height);
            QRect rightRect(x + iconRect->width(), y, textRect->width(), height);
            iconRect->moveCenter(leftRect.center());
            textRect->setRect(rightRect.x(), rightRect.y(),
                              hint ? rightRect.width() + spacing : options.itemRect.width() - leftRect.width(),
                              rightRect.height());
            return;
        }
        if (options.iconAlignment & Qt::AlignTop) {
            int width = hint ? qMax(textRect->width(), iconRect->width()) : options.itemRect.width();
            QRect topRect(x, y, width, iconRect->height());
            QRect bottomRect(x, y + iconRect->height(), width, textRect->height());
            iconRect->moveCenter(topRect.center());
            textRect->setRect(bottomRect.x(), bottomRect.y() + spacing, bottomRect.width(), bottomRect.height());
            return;
        }
    }
}

QSize QItemDelegate::textSize(const QFontMetrics &fontMetrics, const QItemOptions &, const QString &text) const
{
    return fontMetrics.size(0, text); // FIXME: use flags
}

QSize QItemDelegate::iconSize(const QItemOptions &options, const QIconSet &icons) const
{
    QIconSet::Mode mode = options.disabled ? QIconSet::Disabled : QIconSet::Normal; // FIXME: open
    QIconSet::Size size = options.smallItem ? QIconSet::Small : QIconSet::Large;
    QIconSet::State state = options.selected ? QIconSet::On : QIconSet::Off;
    return icons.pixmap(size, mode, state).size();
}

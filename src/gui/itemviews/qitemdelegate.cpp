#include "qitemdelegate.h"
#include <qabstractitemview.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qpoint.h>
#include <qrect.h>

static const int border = 0;

QItemDelegate::QItemDelegate(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemDelegate(model, parent)
{
}

QItemDelegate::~QItemDelegate()
{
}

void QItemDelegate::paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const
{
    static QPoint pt(0, 0);

    QIconSet icons = model()->data(item, QAbstractItemModel::Decoration).toIconSet();
    QString text = model()->data(item, QAbstractItemModel::Display).toString();
#if 0
    static unsigned char r = 0;
    static unsigned char g = 0;
    static unsigned char b = 0;
    painter->fillRect(options.itemRect, QColor(r, g, b));
    r -= 10;
    g += 30;
    b += 10;
    painter->drawRect(options.itemRect);
#endif
#if 1
    QRect iconRect(pt, iconSize(options, icons));
    QRect textRect(pt, textSize(painter->fontMetrics(), options, text));
    doLayout(options, &iconRect, &textRect, false);
    drawIcon(painter, options, iconRect, icons);
    drawText(painter, options, textRect, text);
    drawFocus(painter, options, textRect);
#endif
}

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
                              const QModelIndex &item) const
{
    static QPoint pt(0, 0);
    static QSize sz(border << 1, border << 1);
    
    QString text = model()->data(item, QAbstractItemModel::Display).toString();
    QIconSet icons = model()->data(item, QAbstractItemModel::Decoration).toIconSet();
    
    QRect iconRect(pt, iconSize(options, icons));
    QRect textRect(pt, textSize(fontMetrics, options, text) + sz);
    doLayout(options, &iconRect, &textRect, true);
    
    return iconRect.unite(textRect).size();
}

QItemDelegate::EditType QItemDelegate::editType(const QModelIndex &) const
{
    return WidgetOnTyping;
}

QWidget *QItemDelegate::createEditor(StartEditAction action, QWidget *parent,
                                     const QItemOptions &options, const QModelIndex &item)
{
    if (item.type() != QModelIndex::View)
        return 0;
    if (action & (NoAction | EditKeyPressed | AnyKeyPressed | DoubleClicked)
        || (options.focus && editType(item) == WidgetWhenCurrent)) {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setFrame(false);
        lineEdit->setText(model()->data(item, QAbstractItemModel::Edit).toString());
        lineEdit->selectAll();
        updateEditorGeometry(lineEdit, options, item);
        return lineEdit;
    }
    return 0;
}

void QItemDelegate::removeEditor(EndEditAction, QWidget *editor, const QModelIndex &)
{
    delete editor;
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
        QIconSet icons = model()->data(item, QAbstractItemModel::Decoration).toIconSet();
        QString text = model()->data(item, QAbstractItemModel::Display).toString();
        QRect iconRect(pt, iconSize(options, icons));
        QRect textRect(pt, textSize(editor->fontMetrics(), options, text));
        doLayout(options, &iconRect, &textRect, false);
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
    // reduce the rect to create a border
//     painter->drawText(QRect(rect.x() + border, rect.y(), rect.width() - (border << 1), rect.height()),
//                       options.textAlignment, text);
    QString display;
    if (painter->fontMetrics().width(text) > rect.width())
        painter->drawText(rect, options.textAlignment,
                          ellipsisText(painter->fontMetrics(), rect.width(), options.textAlignment, text));
    else
        painter->drawText(rect, options.textAlignment, text);
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
    QIconSet::Mode mode = options.disabled ? QIconSet::Disabled : QIconSet::Normal; // FIXME: active == focus ?
    QIconSet::Size size = options.smallItem ? QIconSet::Small : QIconSet::Large;
    QIconSet::State state = options.open ? QIconSet::On : QIconSet::Off;
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
        if (options.iconAlignment & Qt::AlignTop) {
            int width = hint ? qMax(textRect->width(), iconRect->width()) : options.itemRect.width();
            QRect topRect(x, y, width, iconRect->height());
            QRect bottomRect(x, y + iconRect->height(), width, textRect->height());
            iconRect->moveCenter(topRect.center());
            textRect->setRect(bottomRect.x(), bottomRect.y(), bottomRect.width(), bottomRect.height());
            return;
        }
        int height = hint ? qMax(textRect->height(), iconRect->height()) : options.itemRect.height();
        bool alignAuto = (options.iconAlignment & Qt::AlignHorizontal_Mask) == Qt::AlignAuto;
        bool reverse = QApplication::reverseLayout() && alignAuto;
        if (reverse || (options.iconAlignment & Qt::AlignRight)) {
            int w = hint ? textRect->width() : options.itemRect.width() - iconRect->width();
            textRect->setRect(x, y, w, height);
            iconRect->moveCenter(QRect(x + w, y, iconRect->width(), height).center());
            return;
        } else {
            QRect leftRect(x, y, iconRect->width(), height);
            QRect rightRect(x + leftRect.width(), y, textRect->width(), height);
            iconRect->moveCenter(leftRect.center());
            int w = hint ? rightRect.width() : options.itemRect.width() - leftRect.width();
            textRect->setRect(rightRect.x(), y, w, height);
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

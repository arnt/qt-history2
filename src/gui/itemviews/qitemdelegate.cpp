#include "qitemdelegate.h"
#include <qabstractitemview.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstyle.h>
#include <qstyleoption.h>

static const int border = 1;

static const char * const unchecked_xpm[] = {
"16 16 2 1",
"  c None",
"# c #000000000000",
"                ",
"                ",
"  ###########   ",
"  #         #   ",
"  #         #   ",
"  #         #   ",
"  #         #   ",
"  #         #   ",
"  #         #   ",
"  #         #   ",
"  #         #   ",
"  #         #   ",
"  ###########   ",
"                ",
"                ",
"                "};

static const char * const checked_xpm[] = {
"16 16 2 1",
"  c None",
"# c #000000000000",
"                ",
"                ",
"  ###########   ",
"  #         #   ",
"  #       # #   ",
"  #      ## #   ",
"  # #   ##  #   ",
"  # ## ##   #   ",
"  #  ###    #   ",
"  #   #     #   ",
"  #         #   ",
"  #         #   ",
"  ###########   ",
"                ",
"                ",
"                ",};

QItemDelegate::QItemDelegate(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemDelegate(model, parent)
{

}

QItemDelegate::~QItemDelegate()
{
}

void QItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
#if 0
    static unsigned char r = 0;
    static unsigned char g = 0;
    static unsigned char b = 0;
    painter->fillRect(option.rect, QColor(r, g, b));
    r -= 10;
    g += 20;
    b += 10;
    painter->drawRect(options.rect);
#endif
    static QPoint pt(0, 0);
    static QSize sz(border * 2, border * 2);
    QVariant variant = model()->data(index, QAbstractItemModel::Decoration);
    QPixmap pixmap = decoration(option, variant);
    QString text = model()->data(index, QAbstractItemModel::Display).toString();
#if 1
    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, painter->fontMetrics().size(0, text) + sz);
    doLayout(option, &pixmapRect, &textRect, false);

    drawDecoration(painter, option, pixmapRect, pixmap);
    drawDisplay(painter, option, textRect, text);
    drawFocus(painter, option, textRect);
#endif
}

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    static QSize sz(border * 2, border * 2);

    QVariant variant = model()->data(index, QAbstractItemModel::Decoration);
    QPixmap pixmap = decoration(option, variant);
    QString text = model()->data(index, QAbstractItemModel::Display).toString();

    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, fontMetrics.size(0, text) + sz);
    doLayout(option, &pixmapRect, &textRect, true);

    return pixmapRect.unite(textRect).size();
}

QItemDelegate::EditorType QItemDelegate::editorType(const QModelIndex &) const
{
    return Widget;
}

QWidget *QItemDelegate::editor(BeginEditAction action, QWidget *parent,
                               const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.type() != QModelIndex::View)
        return 0;
    if (action & (EditKeyPressed | AnyKeyPressed | DoubleClicked | AlwaysEdit)
        || (option.state & QStyle::Style_HasFocus && editorType(index) == Widget)) {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setFrame(false);
        lineEdit->setText(model()->data(index, QAbstractItemModel::Edit).toString());
        lineEdit->selectAll();
        updateEditorGeometry(lineEdit, option, index);
        return lineEdit;
    }
    return 0;
}

void QItemDelegate::releaseEditor(EndEditAction, QWidget *editor, const QModelIndex &)
{
    delete editor;
}

void QItemDelegate::setModelData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit)
        model()->setData(index, QAbstractItemModel::Edit, lineEdit->text());
}

void QItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit)
        lineEdit->setText(model()->data(index, QAbstractItemModel::Edit).toString());
}

void QItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    if (editor) {
        QPixmap pixmap = decoration(option, model()->data(index, QAbstractItemModel::Decoration));
        QString text = model()->data(index, QAbstractItemModel::Display).toString();
        QRect pixmapRect = pixmap.rect();
        QRect textRect(pt, editor->fontMetrics().size(0, text));
        doLayout(option, &pixmapRect, &textRect, false);
        editor->setGeometry(textRect);
    }
}

void QItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QString &text) const
{
    QPen old = painter->pen();
    if (option.state & QStyle::Style_Selected) {
        QRect fill(rect.x() - border, rect.y() - border,
                   rect.width() + border * 2, rect.height() + border * 2);
        painter->fillRect(fill, option.palette.highlight());
        painter->setPen(option.palette.highlightedText());
    } else {
        painter->setPen(option.palette.text());
    }
    QString display;
    if (painter->fontMetrics().width(text) > rect.width())
        painter->drawText(rect, option.displayAlignment,
                          ellipsisText(painter->fontMetrics(), rect.width(),
                                       option.displayAlignment, text));
    else
        painter->drawText(rect, option.displayAlignment, text);
    painter->setPen(old);
}

void QItemDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QRect &rect, const QPixmap &pixmap) const
{
    if (option.state & QStyle::Style_Selected && option.decorationSize == QStyleOptionViewItem::Large)
        painter->fillRect(rect, QBrush(option.palette.highlight(), Qt::Dense4Pattern));
    painter->drawPixmap(rect.topLeft(), pixmap);
}

void QItemDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option,
                              const QRect &rect) const
{
    if (option.state & QStyle::Style_HasFocus) {
        QStyleOptionFocusRect o(0);
        o.rect = rect;
        o.palette = option.palette;
        o.state = QStyle::Style_Default;
        QApplication::style().drawPrimitive(QStyle::PE_FocusRect, &o, painter);
    }
}

void QItemDelegate::doLayout(const QStyleOptionViewItem &option, QRect *pixmapRect,
                             QRect *textRect, bool hint) const
{
    if (pixmapRect && textRect) {
        int x = option.rect.left();
        int y = option.rect.top();
        int b = border * 2;
        int w, h;
        if (hint) {
            w = qMax(textRect->width(), pixmapRect->width()) + b;
            h = qMax(textRect->height(), pixmapRect->height()) + b;
        } else {
            x += border;
            y += border;
            w = option.rect.width() - b;
            h = option.rect.height() - b;
        }
        QRect decoration;
        switch (option.decorationPosition) {
        case QStyleOptionViewItem::Top: {
            decoration.setRect(x, y, w, pixmapRect->height());
            h = hint ? textRect->height() : h - pixmapRect->height();
            textRect->setRect(x, y + pixmapRect->height(), w, h);
            break;}
        case QStyleOptionViewItem::Bottom: {
            textRect->setRect(x, y, w, textRect->height());
            h = hint ? pixmapRect->height() : h - textRect->height();
            decoration.setRect(x, y + textRect->height(), w, h);
            break;}
        case QStyleOptionViewItem::Left: {
            decoration.setRect(x, y, pixmapRect->width(), h);
            w = hint ? textRect->width() : w - pixmapRect->width();
            textRect->setRect(x + pixmapRect->width(), y, w, h);
            break;}
        case QStyleOptionViewItem::Right: {
            textRect->setRect(x, y, textRect->width(), h);
            w = hint ? pixmapRect->width() : w - textRect->width();
            decoration.setRect(x + textRect->width(), y, w, h);
            break;}
        default:
            qWarning("doLayout: decoration positon is invalid");
            decoration = *pixmapRect;
            break;
        }
        if (hint)
            *pixmapRect = decoration;
        else
            doAlignment(decoration, option.decorationAlignment, pixmapRect);
    }
}

void QItemDelegate::doAlignment(const QRect &boundingRect, int alignment, QRect *rect) const
{
    if (alignment == Qt::AlignCenter) {
        rect->moveCenter(boundingRect.center());
        return;
    }
    // Qt::Horizontal
    switch (alignment & Qt::AlignHorizontal_Mask) {
    case Qt::AlignLeft:
        rect->moveLeft(boundingRect.left());
        break;
    case Qt::AlignRight:
        rect->moveRight(boundingRect.right());
        break;
    case Qt::AlignAuto: {
        if (QApplication::reverseLayout())
            rect->moveRight(boundingRect.right());
        else
            rect->moveLeft(boundingRect.left());
        break;}
    case Qt::AlignJustify:
    case Qt::AlignHCenter: {
        rect->moveBy(boundingRect.center().x() - rect->center().x(), 0);
        break;}
    default:
        break;
    }
    // Qt::Vertical
    switch (alignment & Qt::AlignVertical_Mask) {
    case Qt::AlignTop:
        rect->moveTop(boundingRect.top());
        return;
    case Qt::AlignBottom:
        rect->moveBottom(boundingRect.bottom());
        return;
    case Qt::AlignVCenter:
        rect->moveBy(0, boundingRect.center().y() - rect->center().y());
        return;
    default:
        return;
    }
}

QPixmap QItemDelegate::decoration(const QStyleOptionViewItem &option, const QVariant &variant) const
{
    switch (variant.type()) {
    case QVariant::IconSet:
        return variant.toIconSet().pixmap(option.decorationSize == QStyleOptionViewItem::Small
                                          ? QIconSet::Small : QIconSet::Large,
                                          option.state & QStyle::Style_Enabled
                                          ? QIconSet::Normal : QIconSet::Disabled,
                                          option.state & QStyle::Style_Open
                                          ? QIconSet::On : QIconSet::Off);
    case QVariant::Bool: {
        static QPixmap checked(checked_xpm);
        static QPixmap unchecked(unchecked_xpm);
        return variant.toBool() ? checked : unchecked; }
    case QVariant::Color: {
        static QPixmap pixmap(20, 20);
        pixmap.fill(variant.toColor());
        return pixmap; }
    default:
        break;
    }
    return variant.toPixmap();
}

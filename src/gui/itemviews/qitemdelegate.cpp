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

void QItemDelegate::paint(QPainter *painter, const QItemOptions &options,
                          const QModelIndex &item) const
{
#if 0
    static unsigned char r = 0;
    static unsigned char g = 0;
    static unsigned char b = 0;
    painter->fillRect(options.itemRect, QColor(r, g, b));
    r -= 10;
    g += 20;
    b += 10;
    painter->drawRect(options.itemRect);
#endif
    
    static QPoint pt(0, 0);
    static QSize sz(border << 1, border << 1);
    QVariant variant = model()->data(item, QAbstractItemModel::Decoration);
    QPixmap pixmap = decoration(options, variant);
    QString text = model()->data(item, QAbstractItemModel::Display).toString();
#if 1
    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, painter->fontMetrics().size(0, text) + sz);
    doLayout(options, &pixmapRect, &textRect, false);
    
    drawDecoration(painter, options, pixmapRect, pixmap);
    drawDisplay(painter, options, textRect, text);
    drawFocus(painter, options, textRect);
#endif
}

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
                              const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    static QSize sz(border << 1, border << 1);

    QVariant variant = model()->data(index, QAbstractItemModel::Decoration);
    QPixmap pixmap = decoration(options, variant);
    QString text = model()->data(index, QAbstractItemModel::Display).toString();
    
    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, fontMetrics.size(0, text) + sz);
    doLayout(options, &pixmapRect, &textRect, true);
    
    return pixmapRect.unite(textRect).size();
}

QItemDelegate::EditType QItemDelegate::editType(const QModelIndex &) const
{
    return WidgetOnTyping;
}

QWidget *QItemDelegate::createEditor(StartEditAction action, QWidget *parent,
                                     const QItemOptions &options, const QModelIndex &index)
{
    if (index.type() != QModelIndex::View)
        return 0;
    if (action & (EditKeyPressed | AnyKeyPressed | DoubleClicked | AlwaysEdit)
        || (options.focus && editType(index) == WidgetWhenCurrent)) {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setFrame(false);
        lineEdit->setText(model()->data(index, QAbstractItemModel::Edit).toString());
        lineEdit->selectAll();
        updateEditorGeometry(lineEdit, options, index);
        return lineEdit;
    }
    return 0;
}

void QItemDelegate::removeEditor(EndEditAction, QWidget *editor, const QModelIndex &)
{
    delete editor;
}

void QItemDelegate::setContentFromEditor(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit)
        model()->setData(index, QAbstractItemModel::Edit, lineEdit->text());
}

void QItemDelegate::updateEditorContents(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit)
        lineEdit->setText(model()->data(index, QAbstractItemModel::Edit).toString());
}

void QItemDelegate::updateEditorGeometry(QWidget *editor, const QItemOptions &options,
                                         const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    if (editor) {
        QPixmap pixmap = decoration(options, model()->data(index, QAbstractItemModel::Decoration));
        QString text = model()->data(index, QAbstractItemModel::Display).toString();
        QRect pixmapRect = pixmap.rect();
        QRect textRect(pt, editor->fontMetrics().size(0, text));
        doLayout(options, &pixmapRect, &textRect, false);
        editor->setGeometry(textRect);
    }
}

void QItemDelegate::drawDisplay(QPainter *painter, const QItemOptions &options, const QRect &rect,
                                const QString &text) const
{
    QPen old = painter->pen();
    if (options.selected) {
        painter->fillRect(rect, options.palette.highlight());
        painter->setPen(options.palette.highlightedText());
    } else {
        painter->setPen(options.palette.text());
    }
    QString display;
    if (painter->fontMetrics().width(text) > rect.width())
        painter->drawText(rect, options.displayAlignment,
                          ellipsisText(painter->fontMetrics(), rect.width(),
                                       options.displayAlignment, text));
    else
        painter->drawText(rect, options.displayAlignment, text);
    painter->setPen(old);
}

void QItemDelegate::drawDecoration(QPainter *painter, const QItemOptions &options,
                                   const QRect &rect, const QPixmap &pixmap) const
{
    if (options.selected && !options.smallItem)
        painter->fillRect(rect, QBrush(options.palette.highlight(), QBrush::Dense4Pattern));
    painter->drawPixmap(rect.topLeft(), pixmap);
}

void QItemDelegate::drawFocus(QPainter *painter, const QItemOptions &options, const QRect &rect) const
{
    if (options.focus)
        QApplication::style().drawPrimitive(QStyle::PE_FocusRect, painter, rect, options.palette);
}

void QItemDelegate::doLayout(const QItemOptions &options, QRect *pixmapRect,
                             QRect *textRect, bool hint) const
{
    if (pixmapRect && textRect) {
        int x = options.itemRect.left();
        int y = options.itemRect.top();
        int w, h;
        if (hint) {
            w = qMax(textRect->width(), pixmapRect->width());
            h = qMax(textRect->height(), pixmapRect->height());
        } else {
            w = options.itemRect.width();
            h = options.itemRect.height();
        }
        QRect decoration;
        switch (options.decorationPosition) {
        case QItemOptions::Top: {
            decoration.setRect(x, y, w, pixmapRect->height());
            h = hint ? textRect->height() : options.itemRect.height() - pixmapRect->height();
            textRect->setRect(x, y + pixmapRect->height(),w, h);
            break;}
        case QItemOptions::Bottom: {
            textRect->setRect(x, y, w, textRect->height());
            h = hint ? pixmapRect->height() : options.itemRect.height() - textRect->height();
            decoration.setRect(x, y + textRect->height(), w, h);
            break;}
        case QItemOptions::Left: {
            decoration.setRect(x, y, pixmapRect->width(), h);
            w = hint ? textRect->width() : options.itemRect.width() - pixmapRect->width();
            textRect->setRect(x + pixmapRect->width(), y, w, h);
            break;}
        case QItemOptions::Right: {
            textRect->setRect(x, y, textRect->width(), h);
            w = hint ? pixmapRect->width() : options.itemRect.width() - textRect->width();
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
            doAlignment(decoration, options.decorationAlignment, pixmapRect);
    }
}

void QItemDelegate::doAlignment(const QRect &boundingRect, int alignment, QRect *rect) const
{
    if (alignment == Qt::AlignCenter) {
        rect->moveCenter(boundingRect.center());
        return;
    }
    // Horizontal
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
    // Vertical
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

QPixmap QItemDelegate::decoration(const QItemOptions &options, const QVariant &variant) const
{
    switch (variant.type()) {
    case QVariant::IconSet:
        return variant.toIconSet().pixmap(options.smallItem ? QIconSet::Small : QIconSet::Large,
                                          options.disabled ? QIconSet::Disabled : QIconSet::Normal,
                                          options.open ? QIconSet::On : QIconSet::Off);
    case QVariant::Bool: {
        static QPixmap checked(checked_xpm);
        static QPixmap unchecked(unchecked_xpm);
        return variant.toBool() ? checked : unchecked; }
    default:
        break;
    }
    return variant.toPixmap();
}

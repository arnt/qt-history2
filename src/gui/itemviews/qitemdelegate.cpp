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
"16 16 4 1",
"        c None",
".        c #000000000000",
"X        c #FFFFFFFFFFFF",
"o        c #C71BC30BC71B",
"                ",
"                ",
" ..........     ",
" .XXXXXXXX.     ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" ..........oo   ",
"   oooooooooo   ",
"   oooooooooo   ",
"                ",
"                "};

static const char * const checked_xpm[] = {
"16 16 4 1",
"        c None",
".        c #000000000000",
"X        c #FFFFFFFFFFFF",
"o        c #C71BC30BC71B",
"                ",
"                ",
" ..........     ",
" .XXXXXXXX.     ",
" .XXXXXXoX.oo   ",
" .XoXXXo.X.oo   ",
" .X.oXo..X.oo   ",
" .X..o..oX.oo   ",
" .Xo...oXX.oo   ",
" .XXo.oXXX.oo   ",
" .XXXoXXXX.oo   ",
" ..........oo   ",
"   oooooooooo   ",
"   oooooooooo   ",
"                ",
"                "};

QItemDelegate::QItemDelegate(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemDelegate(model, parent)
{

}

QItemDelegate::~QItemDelegate()
{
}

void QItemDelegate::paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const
{
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
    
    static QPoint pt(0, 0);
    static QSize sz(border << 1, border << 1);
    QVariant variant = model()->data(item, QAbstractItemModel::Decoration);
    QPixmap pixmap = decoration(options, variant);
    QString text = model()->data(item, QAbstractItemModel::Display).toString();
#if 1
    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, painter->fontMetrics().size(0, text) + sz);
    doLayout(options, &pixmapRect, &textRect, false);
    
    drawPixmap(painter, options, pixmapRect, pixmap);
    drawText(painter, options, textRect, text);
    drawFocus(painter, options, textRect);
#endif
}

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
                              const QModelIndex &item) const
{
    static QPoint pt(0, 0);
    static QSize sz(border << 1, border << 1);

    QVariant variant = model()->data(item, QAbstractItemModel::Decoration);
    QPixmap pixmap = decoration(options, variant);
    QString text = model()->data(item, QAbstractItemModel::Display).toString();
    
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
                                     const QItemOptions &options, const QModelIndex &item)
{
    if (item.type() != QModelIndex::View)
        return 0;
    if (action & (EditKeyPressed | AnyKeyPressed | DoubleClicked | AlwaysEdit)
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
        QPixmap pixmap = decoration(options, model()->data(item, QAbstractItemModel::Decoration));
        QString text = model()->data(item, QAbstractItemModel::Display).toString();
        QRect pixmapRect = pixmap.rect();
        QRect textRect(pt, editor->fontMetrics().size(0, text));
        doLayout(options, &pixmapRect, &textRect, false);
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
    QString display;
    if (painter->fontMetrics().width(text) > rect.width())
        painter->drawText(rect, options.textAlignment,
                          ellipsisText(painter->fontMetrics(), rect.width(),
                                       options.textAlignment, text));
    else
        painter->drawText(rect, options.textAlignment, text);
    painter->setPen(old);
}

void QItemDelegate::drawPixmap(QPainter *painter, const QItemOptions &options,
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

void QItemDelegate::doLayout(const QItemOptions &options, QRect *pixmapRect, QRect *textRect, bool hint) const
{
    if (pixmapRect && textRect) {
        int x = options.itemRect.left();
        int y = options.itemRect.top();
        if (options.iconAlignment & Qt::AlignTop) {
            int width = hint ? qMax(textRect->width(), pixmapRect->width()) : options.itemRect.width();
            QRect topRect(x, y, width, pixmapRect->height());
            QRect bottomRect(x, y + pixmapRect->height(), width, textRect->height());
            pixmapRect->moveCenter(topRect.center());
            textRect->setRect(bottomRect.x(), bottomRect.y(), bottomRect.width(), bottomRect.height());
            return;
        }
        int height = hint ? qMax(textRect->height(), pixmapRect->height()) : options.itemRect.height();
        bool alignAuto = (options.iconAlignment & Qt::AlignHorizontal_Mask) == Qt::AlignAuto;
        bool reverse = QApplication::reverseLayout() && alignAuto;
        if (reverse || (options.iconAlignment & Qt::AlignRight)) {
            int w = hint ? textRect->width() : options.itemRect.width() - pixmapRect->width();
            textRect->setRect(x, y, w, height);
            pixmapRect->moveCenter(QRect(x + w, y, pixmapRect->width(), height).center());
            return;
        } else {
            QRect leftRect(x, y, pixmapRect->width(), height);
            QRect rightRect(x + leftRect.width(), y, textRect->width(), height);
            pixmapRect->moveCenter(leftRect.center());
            int w = hint ? rightRect.width() : options.itemRect.width() - leftRect.width();
            textRect->setRect(rightRect.x(), y, w, height);
            return;
        }
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

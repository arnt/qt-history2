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

#include "qitemdelegate.h"
#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpixmapcache.h>
#include <qitemeditorfactory.h>
#include <private/qobject_p.h>

static const int border = 1;

class QItemDelegatePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QItemDelegate)

public:
    QItemDelegatePrivate() : f(0) {}

    inline const QItemEditorFactory *editorFactory() const
        { return f ? f : QItemEditorFactory::defaultFactory(); }

    QItemEditorFactory *f;
};

#define d d_func()
#define q q_func()

/*!
    \class QItemDelegate

    \brief The QItemDelegate class provides display and editing facilities for
    data items from a model.

    \ingroup model-view
    \mainclass

    A QItemDelegate can be used to provide an editor for an item view class
    that is subclassed from QAbstractItemView. Using a delegate for this
    purpose allows the editing mechanism to be customized and developed
    independently from the model and view.

    Delegates can be used to manipulate data in two complementary ways:
    by processing events in the normal manner, or by implementing a
    custom editor widget. The item delegate takes the approach of providing
    a widget for editing purposes that can be supplied to
    QAbstractItemView::setDelegate() or the equivalent function in
    subclasses of QAbstractItemView.

    This class demonstrates how to implement the functions for painting
    the delegate, and editing data from the model. The paint() and
    sizeHint() virtual functions defined in QAbstractItemDelegate are
    implemented to ensure that the delegate is presented correctly.
    Only the standard editing functions for widget-based delegates are
    reimplemented here: editor() returns the widget used to change data
    from the model; setEditorData() provides the widget with data to
    manipulate; updateEditorGeometry() ensures that the editor is displayed
    correctly with respect to the item view; setModelData() returns the
    updated data to the model; releaseEditor() indicates that the user has
    completed editing the data, and that the editor widget can be destroyed.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemDelegate

*/

/*!
    Constructs an item delegate with the given \a parent.
*/

QItemDelegate::QItemDelegate(QObject *parent)
    : QAbstractItemDelegate(*new QItemDelegatePrivate(), parent)
{

}

/*!
    Destroys the item delegate.
*/

QItemDelegate::~QItemDelegate()
{
}

/*!
    Renders the delegate using the given \a painter and style \a option for
    the item specified by the \a model and the item \a index.
*/

void QItemDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    static QSize sz(border * 2, border * 2);

    QStyleOptionViewItem opt = option;
    const QAbstractItemModel *model = index.model();

    // set font
    QVariant value = model->data(index, QAbstractItemModel::FontRole);
    if (value.isValid())
        opt.font = value.toFont();

    // set text alignment
    value = model->data(index, QAbstractItemModel::TextAlignmentRole);
    if (value.isValid())
        opt.displayAlignment = value.toInt();

    // set text color
    value = model->data(index, QAbstractItemModel::TextColorRole);
    if (value.isValid() && value.toColor().isValid())
        opt.palette.setColor(QPalette::Text, value.toColor());

    // do layout
    value = model->data(index, QAbstractItemModel::DecorationRole);
    QPixmap pixmap = decoration(opt, value);
    QString text = model->data(index, QAbstractItemModel::DisplayRole).toString();

    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, painter->fontMetrics().size(0, text) + sz);
    doLayout(opt, &pixmapRect, &textRect, false);

    // draw the background color
    value = model->data(index, QAbstractItemModel::BackgroundColorRole);
    if (value.isValid() && value.toColor().isValid())
        painter->fillRect(option.rect, value.toColor());

    // draw the item
    drawDecoration(painter, opt, pixmapRect, pixmap);
    drawDisplay(painter, opt, textRect, text);
    drawFocus(painter, opt, textRect);
}

/*!
    Returns the size needed by the delegate to display the item specified by
    the \a model and item \a index, taking into account the style information
    provided by \a option.
*/

QSize QItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    static QSize sz(border * 2, border * 2);

    const QAbstractItemModel *model = index.model();

    QVariant value = model->data(index, QAbstractItemModel::FontRole);
    QFont fnt = value.isValid() ? value.toFont() : option.font;

    value = model->data(index, QAbstractItemModel::DecorationRole);
    QPixmap pixmap = decoration(option, value);
    QString text = model->data(index, QAbstractItemModel::DisplayRole).toString();

    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, QFontMetrics(fnt).size(0, text) + sz);
    doLayout(option, &pixmapRect, &textRect, true);

    return pixmapRect.unite(textRect).size();
}

/*!
    Returns the widget used to edit the item specified by the \a model and
    item \a index for editing. The \a parent widget and style \a option are
    used to control how the editor widget appears.

    \sa QAbstractItemDelegate::editor()
*/

QWidget *QItemDelegate::editor(QWidget *parent,
                               const QStyleOptionViewItem &,
                               const QModelIndex &index)
{
    QVariant::Type t = index.model()->data(index, QAbstractItemModel::EditRole).type();
    QWidget *w = QItemEditorFactory::defaultFactory()->createEditor(t, parent);
    if (w) w->installEventFilter(this);
    return w;
}

/*!
    Releases the \a editor.

    \sa QAbstractItemDelegate::releaseEditor()
*/

void QItemDelegate::releaseEditor(QWidget *editor)
{
    editor->removeEventFilter(this);
    delete editor; // FIXME: editor->deleteLater();
}

/*!
    Sets the data to be displayed and edited by the \a editor for the
    item specified by the \a model and item \a index.
*/

void QItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant v = index.model()->data(index, QAbstractItemModel::EditRole);
    QByteArray n = d->editorFactory()->valuePropertyName(v.type());
    if (!n.isEmpty())
        editor->setProperty(n, v);
}

/*!
    Sets the data for the specified \a model and item \a index from that
    supplied by the \a editor.
*/

void QItemDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QVariant::Type t = model->data(index, QAbstractItemModel::EditRole).type();
    QByteArray n = d->editorFactory()->valuePropertyName(t);
    if (!n.isEmpty())
        model->setData(index, editor->property(n), QAbstractItemModel::EditRole);
}

/*!
    Updates the \a editor for the item specified by the \a model and
    item \a index according to the style \a option given.
 */

void QItemDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    if (editor) {
        const QAbstractItemModel *model = index.model();
        QPixmap pixmap = decoration(option, model->data(index, QAbstractItemModel::DecorationRole));
        QString text = model->data(index, QAbstractItemModel::EditRole).toString();
        QRect pixmapRect = pixmap.rect();
        QRect textRect(pt, editor->fontMetrics().size(0, text));
        doLayout(option, &pixmapRect, &textRect, false);
        editor->setGeometry(textRect);
    }
}

/*!
  Returns the editor factory used by the item delegate.
  If no editor factory is set, the function will return null.
*/
QItemEditorFactory *QItemDelegate::itemEditorFactory() const
{
    return d->f;
}

/*!
  Sets the editor factory to be used by the item delegate to be the \a factory
  specified. If no editor factory is set, the item delegate will use the
  default editor factory.
*/
void QItemDelegate::setItemEditorFactory(QItemEditorFactory *factory)
{
    d->f = factory;
}

/*!
   Renders the item view \a text within the rectangle specified by \a rect
   using the given \a painter and style \a option.
*/

void QItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QString &text) const
{
    QPen pen = painter->pen();
    QPalette::ColorGroup cg = option.state & QStyle::Style_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
    if (option.state & QStyle::Style_Selected) {
        QRect fill(rect.x() - border, rect.y() - border,
                   rect.width() + border * 2, rect.height() + border * 2);
        painter->fillRect(fill, option.palette.color(cg, QPalette::Highlight));
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }
    QFont font = painter->font();
    painter->setFont(option.font);
    if (painter->fontMetrics().width(text) > rect.width())
        painter->drawText(rect, option.displayAlignment,
                          ellipsisText(painter->fontMetrics(), rect.width(),
                                       option.displayAlignment, text));
    else
        painter->drawText(rect, option.displayAlignment, text);
    painter->setFont(font);
    painter->setPen(pen);
}

/*!
    Renders the decoration \a pixmap within the rectangle specified by
    \a rect using the given \a painter and style \a option.
*/

void QItemDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QRect &rect, const QPixmap &pixmap) const
{
    if (!pixmap.isNull() && !rect.isEmpty()) {
        if (option.state & QStyle::Style_Selected) {
            bool enabled = option.state & QStyle::Style_Enabled;
            QPixmap *pm = selected(pixmap, option.palette, enabled);
            painter->drawPixmap(rect.topLeft(), *pm);
        } else {
            painter->drawPixmap(rect.topLeft(), pixmap);
        }
    }
}

/*!
    Renders the region within the rectangle specified by \a rect, indicating
    that it has the focus, using the given \a painter and style \a option.
*/

void QItemDelegate::drawFocus(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QRect &rect) const
{
    if (option.state & QStyle::Style_HasFocus) {
        QStyleOptionFocusRect o;
        o.rect.setRect(rect.x() - border, rect.y() - border,
                       rect.width() + border * 2, rect.height() + border * 2);
        o.palette = option.palette;
        o.state = QStyle::Style_None;
        QPalette::ColorGroup cg = option.state & QStyle::Style_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (option.state & QStyle::Style_Selected)
            o.backgroundColor = option.palette.color(cg, QPalette::Highlight);
        else
            o.backgroundColor = option.palette.color(cg, QPalette::Background);
        QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
    }
}

/*!
    \internal
*/

void QItemDelegate::doLayout(const QStyleOptionViewItem &option, QRect *pixmapRect,
                             QRect *textRect, bool hint) const
{
    if (pixmapRect && textRect) {
        int x = option.rect.left();
        int y = option.rect.top();
        int bb = border * 2;
        int w, h;
        if (hint) {
            w = qMax(textRect->width(), pixmapRect->width()) + bb;
            h = qMax(textRect->height(), pixmapRect->height()) + bb;
        } else {
            x += border;
            y += border;
            w = option.rect.width() - bb;
            h = option.rect.height() - bb;
        }
        QRect display;
        QRect decoration;
        switch (option.decorationPosition) {
        case QStyleOptionViewItem::Top: {
            decoration.setRect(x, y, w, pixmapRect->height());
            h = hint ? textRect->height() : h - pixmapRect->height();
            display.setRect(x, y + pixmapRect->height(), w, h);
            break;}
        case QStyleOptionViewItem::Bottom: { // FIXME: doesn't work properly
            decoration.setRect(x, y + h - pixmapRect->height(), w, pixmapRect->height());
            h = hint ? textRect->height() : h - pixmapRect->height();
            display.setRect(x, y, w, h);
            break;}
        case QStyleOptionViewItem::Left: {
            if (option.direction == Qt::RightToLeft){
                decoration.setRect(x + w - pixmapRect->width(), y, pixmapRect->width(), h);
                w = hint ? textRect->width() : w - pixmapRect->width();
                display.setRect(x, y, w, h);
                break;
            }
            decoration.setRect(x, y, pixmapRect->width(), h);
            w = hint ? textRect->width() : w - pixmapRect->width();
            display.setRect(x + pixmapRect->width(), y, w, h);
            break;}
        case QStyleOptionViewItem::Right: {
            if (option.direction == Qt::RightToLeft){
                decoration.setRect(x, y, pixmapRect->width(), h);
                w = hint ? textRect->width() : w - pixmapRect->width();
                display.setRect(x + pixmapRect->width(), y, w, h);
                break;
            }
            decoration.setRect(x + w - pixmapRect->width(), y, pixmapRect->width(), h);
            w = hint ? textRect->width() : w - pixmapRect->width();
            display.setRect(x, y, w, h);
            break;}
        default:
            qWarning("doLayout: decoration positon is invalid");
            decoration = *pixmapRect;
            break;
        }

        *pixmapRect = decoration;
        *textRect = display;
        if (!hint) // we only need to do the internal layout if we are going to paint
            doAlignment(option.direction, decoration, option.decorationAlignment, pixmapRect);
    }
}

/*!
    \internal
*/

void QItemDelegate::doAlignment(Qt::LayoutDirection direction, const QRect &boundingRect, int alignment, QRect *rect) const
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
    case Qt::AlignAuto:
        if (direction == Qt::RightToLeft)
            rect->moveRight(boundingRect.right());
        else
            rect->moveLeft(boundingRect.left());
        break;
    case Qt::AlignJustify:
    case Qt::AlignHCenter:
        rect->translate(boundingRect.center().x() - rect->center().x(), 0);
        break;
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
        rect->translate(0, boundingRect.center().y() - rect->center().y());
        return;
    default:
        return;
    }
}

/*!
    \internal

    Returns the pixmap used to decorate the root of the item view.
    The style \a option controls the appearance of the root; the \a variant
    refers to the data associated with an item.
*/

QPixmap QItemDelegate::decoration(const QStyleOptionViewItem &option, const QVariant &variant) const
{
    switch (variant.type()) {
    case QVariant::Icon:
        return variant.toIcon().pixmap(option.decorationSize == QStyleOptionViewItem::Small
                                          ? Qt::SmallIconSize : Qt::LargeIconSize,
                                          option.state & QStyle::Style_Enabled
                                          ? QIcon::Normal : QIcon::Disabled,
                                          option.state & QStyle::Style_Open
                                          ? QIcon::On : QIcon::Off);
    case QVariant::Bool: {
        static QPixmap checked(QApplication::style()->standardPixmap(
                                   QStyle::SP_ItemChecked, &option));
        static QPixmap unchecked(QApplication::style()->standardPixmap(
                                     QStyle::SP_ItemUnchecked, &option));
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

/*!
  \internal
*/

QPixmap *QItemDelegate::selected(const QPixmap &pixmap, const QPalette &palette, bool enabled) const
{
    QString key;
    key.sprintf("%d-%d", pixmap.serialNumber(), enabled);
    QPixmap *pm = QPixmapCache::find(key);
    if (!pm) {
        QImage img = pixmap.toImage();
        if (img.depth() != 32)
            img = img.convertDepth(32);
        img.setAlphaBuffer(true);
        QColor color = palette.color(enabled
                                     ? QPalette::Normal
                                     : QPalette::Disabled,
                                     QPalette::Highlight);
        uint rgb = color.rgb();
        for (int i = 0; i < img.height(); ++i) {
            uint *p = (uint *)(img.scanLine(i));
            uint *end = p + img.width();
            for (; p < end; ++p) {
                *p = (0xff000000 & *p)
                     | (0xff & static_cast<int>(qRed(*p) * 0.70 + qRed(rgb) * 0.30)) << 16
                     | (0xff & static_cast<int>(qGreen(*p) * 0.70 + qGreen(rgb) * 0.30)) << 8
                     | (0xff & static_cast<int>(qBlue(*p) * 0.70 + qBlue(rgb) * 0.30));
            }
        }
        pm = new QPixmap;
        pm->fromImage(img);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

/*!
  If the \a object is the current editor: if the \a event is an Esc
  key press the current edit is cancelled and ended, or if the \a
  event is an Enter or Return key press the current edit is accepted
  and ended. If editing is ended the event filter returns true to
  signify that it has handled the event; in all other cases it does
  nothing and returns false to signify that the event hasn't been
  handled.

  \sa endEdit()
*/

bool QItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    QWidget *editor = ::qt_cast<QWidget*>(object);
    if (!editor)
        return false;
    if (event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
        case Qt::Key_Tab:
            emit commitData(editor);
            emit closeEditor(editor, EditNextItem);
            return true;
        case Qt::Key_Backtab:
            emit commitData(editor);
            emit closeEditor(editor, EditPreviousItem);
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            emit commitData(editor);
            emit closeEditor(editor, SubmitModelCache);
            return true;
        case Qt::Key_Escape:
            // don't commit data
            emit closeEditor(editor, RevertModelCache);
            return true;
        default:
            break;
        }
    } else if (event->type() == QEvent::FocusOut) {
        emit commitData(editor);
        emit closeEditor(editor, NoHint);
        return true;
    }
    return false;
}

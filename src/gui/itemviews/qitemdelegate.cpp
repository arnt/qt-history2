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
    : QAbstractItemDelegate(parent)
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

void QItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QAbstractItemModel *model, const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    static QSize sz(border * 2, border * 2);
    QVariant variant = model->data(index, QAbstractItemModel::DecorationRole);
    QPixmap pixmap = decoration(option, variant);
    QString text = model->data(index, QAbstractItemModel::DisplayRole).toString();

    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, painter->fontMetrics().size(0, text) + sz);
    doLayout(option, &pixmapRect, &textRect, false);

    drawDecoration(painter, option, pixmapRect, pixmap);
    drawDisplay(painter, option, textRect, text);
    drawFocus(painter, option, textRect);
}

/*!
Returns the size needed by the delegate to display the item specified by
the \a model and item \a index, taking into account the visual
information provided by the font metrics in \a fontMetrics, and the given
style \a option.
*/

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                              const QAbstractItemModel *model, const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    static QSize sz(border * 2, border * 2);

    QVariant variant = model->data(index, QAbstractItemModel::DecorationRole);
    QPixmap pixmap = decoration(option, variant);
    QString text = model->data(index, QAbstractItemModel::DisplayRole).toString();

    QRect pixmapRect = pixmap.rect();
    QRect textRect(pt, fontMetrics.size(0, text) + sz);
    doLayout(option, &pixmapRect, &textRect, true);

    return pixmapRect.unite(textRect).size();
}

/*!
    \fn QItemDelegate::EditorType QItemDelegate::editorType(const QAbstractItemModel *model, const QModelIndex &index) const

    Returns the type of editor that this delegate implements for the item at
    the given \a index in the \a model.

    \sa QAbstractItemDelegate::EditorType QAbstractItemDelegate::editorType()
*/

QItemDelegate::EditorType QItemDelegate::editorType(const QAbstractItemModel *,
                                                    const QModelIndex &) const
{
    return Widget;
}

/*!
    \fn QWidget *QItemDelegate::editor(QAbstractItemDelegate::BeginEditAction action,
                    QWidget *parent,
                    const QStyleOptionViewItem &option,
                    const QAbstractItemModel *model, const QModelIndex &index)

    Returns the widget used to edit the item specified by the \a model and
    item \a index for the editing \a action given. The \a parent widget and
    style \a option are used to control how the editor widget appears.

    \sa QAbstractItemDelegate::BeginEditAction QAbstractItemDelegate::editor()

*/

QWidget *QItemDelegate::editor(BeginEditAction action, QWidget *parent,
                               const QStyleOptionViewItem &option,
                               const QAbstractItemModel *model, const QModelIndex &index)
{
    if (index.type() != QModelIndex::View)
        return 0;
    if (action & (EditKeyPressed | AnyKeyPressed | DoubleClicked | AlwaysEdit)
        || (option.state & QStyle::Style_HasFocus && editorType(model, index) == Widget)) {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setFrame(false);
        lineEdit->installEventFilter(this);
        return lineEdit;
    }
    return 0;
}

/*!
    Releases the \a editor.

    \sa QAbstractItemDelegate::releaseEditor()
*/

void QItemDelegate::releaseEditor(QWidget *editor)
{
    editor->deleteLater();
}

/*!
    Sets the data to be displayed and edited by the \a editor for the
    item specified by the \a model and item \a index.
*/

void QItemDelegate::setEditorData(QWidget *editor,
                                  const QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit) {
        lineEdit->setText(model->data(index, QAbstractItemModel::EditRole).toString());
        lineEdit->selectAll();
    }
}

/*!
    Sets the data for the specified \a model and item \a index from that
    supplied by the \a editor.
*/

void QItemDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit)
        model->setData(index, QAbstractItemModel::EditRole, lineEdit->text());
}

/*!
    Updates the \a editor for the item specified by the \a model and
    item \a index according to the style \a option given.
 */

void QItemDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    static QPoint pt(0, 0);
    if (editor) {
        QPixmap pixmap = decoration(option, model->data(index, QAbstractItemModel::DecorationRole));
        QString text = model->data(index, QAbstractItemModel::EditRole).toString();
        QRect pixmapRect = pixmap.rect();
        QRect textRect(pt, editor->fontMetrics().size(0, text));
        doLayout(option, &pixmapRect, &textRect, false);
        editor->setGeometry(textRect);
    }
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
            QColor col = option.palette.color(enabled ? QPalette::Normal : QPalette::Disabled,
                                              QPalette::Highlight);

            // ### some trickery to get alpha blended selections - revisit this when QPixmap has been fixed
            QPixmap fake(pixmap.width(), pixmap.height());
            fake.setMask(*pixmap.mask());
            QPainter p(&fake);
            p.drawPixmap(0, 0, pixmap);
            p.fillRect(fake.rect(), QBrush(QColor(col.red(), col.green(), col.blue(), 90)));
            p.end();
            painter->drawPixmap(rect.topLeft(), fake);
        } else {
            painter->drawPixmap(rect.topLeft(), pixmap);
        }
    }
}

/*!
    Renders the region within the rectangle specified by \a rect, indicating
    that it has the focus, using the given \a painter and style \a option.
*/

void QItemDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option,
                              const QRect &rect) const
{
    if (option.state & QStyle::Style_HasFocus) {
        QStyleOptionFocusRect o(0);
        o.rect.setRect(rect.x() - border, rect.y() - border,
                       rect.width() + border * 2, rect.height() + border * 2);
        o.palette = option.palette;
        o.state = QStyle::Style_Default;
        QPalette::ColorGroup cg = option.state & QStyle::Style_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (option.state & QStyle::Style_Selected)
            o.backgroundColor = option.palette.color(cg, QPalette::Highlight);
        else
            o.backgroundColor = option.palette.color(cg, QPalette::Background);
        QApplication::style().drawPrimitive(QStyle::PE_FocusRect, &o, painter);
    }
}

/*!
    \internal
*/

void QItemDelegate::doLayout(const QStyleOptionViewItem &option, QRect *pixmapRect,
                             QRect *textRect, bool hint) const
{
    if (pixmapRect && textRect) {
        static const int spacing = 0; // FIXME: for now
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
            h = hint ? textRect->height() : h - pixmapRect->height() - spacing;
            textRect->setRect(x, y + pixmapRect->height() + spacing, w, h);
            break;}
        case QStyleOptionViewItem::Bottom: {
            textRect->setRect(x, y, w, textRect->height());
            h = hint ? pixmapRect->height() : h - textRect->height() - spacing;
            decoration.setRect(x, y + textRect->height() + spacing, w, h);
            break;}
        case QStyleOptionViewItem::Left: {
            decoration.setRect(x, y, pixmapRect->width(), h);
            w = hint ? textRect->width() : w - pixmapRect->width() - spacing;
            textRect->setRect(x + pixmapRect->width() + spacing, y, w, h);
            break;}
        case QStyleOptionViewItem::Right: {
            textRect->setRect(x, y, textRect->width(), h);
            w = hint ? pixmapRect->width() : w - textRect->width() - spacing;
            decoration.setRect(x + textRect->width() + spacing, y, w, h);
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

/*!
    \internal

*/

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

/*!
    \internal

    Returns the pixmap used to decorate the root of the item view.
    The style \a option controls the appearance of the root; the \a variant
    refers to the data associated with an item.
*/

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

/*!
  ### update this
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
    QLineEdit *editor = ::qt_cast<QLineEdit*>(object);
    if (editor && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            emit commitData(editor);
        case Qt::Key_Escape:
            emit doneEditing(editor);
//             editor->hide();
//             editor->parentWidget()->setFocus();
//             editor->deleteLater();
            return true;
        default:
            break;
        }
    }
    return false;
}

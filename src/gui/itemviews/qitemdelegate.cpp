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
#include <private/qdnd_p.h>
#include <qdebug.h>

static const int textMargin = 2;

class QItemDelegatePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QItemDelegate)

public:
    QItemDelegatePrivate() : f(0) {}

    inline const QItemEditorFactory *editorFactory() const
        { return f ? f : QItemEditorFactory::defaultFactory(); }

    QItemEditorFactory *f;
};

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

    The QItemDelegate class is one of the \l{Model/View Classes}
    and is part of Qt's \l{Model/View Programming}{model/view framework}.

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
    Q_ASSERT(index.isValid());
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QStyleOptionViewItem opt = option;

    // set font
    QVariant value = model->data(index, Qt::FontRole);
    if (value.isValid())
        opt.font = qvariant_cast<QFont>(value);

    // set text alignment
    value = model->data(index, Qt::TextAlignmentRole);
    if (value.isValid())
        opt.displayAlignment = QFlag(value.toInt());

    // set text color
    value = model->data(index, Qt::TextColorRole);
    if (value.isValid() && qvariant_cast<QColor>(value).isValid())
        opt.palette.setColor(QPalette::Text, qvariant_cast<QColor>(value));

    // do layout
    value = model->data(index, Qt::DecorationRole);
    QPixmap pixmap = decoration(opt, value);
    QRect pixmapRect = pixmap.rect();

    QFontMetrics fontMetrics(opt.font);
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect textRect(0, 0, fontMetrics.width(text), fontMetrics.lineSpacing());

    value = model->data(index, Qt::CheckStateRole);
    QRect checkRect = check(opt, value);
    Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());

    doLayout(opt, &checkRect, &pixmapRect, &textRect, false);

    // draw the background color
    value = model->data(index, Qt::BackgroundColorRole);
    if (value.isValid() && qvariant_cast<QColor>(value).isValid())
        painter->fillRect(option.rect, qvariant_cast<QColor>(value));

    // draw the item
    drawCheck(painter, opt, checkRect, checkState);
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
    Q_ASSERT(index.isValid());
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QVariant value = model->data(index, Qt::FontRole);
    QFont fnt = value.isValid() ? qvariant_cast<QFont>(value) : option.font;
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect pixmapRect;
    if (model->data(index, Qt::DecorationRole).isValid())
        pixmapRect = QRect(0, 0, option.decorationSize.width(),
                           option.decorationSize.height());

    QFontMetrics fontMetrics(fnt);
    QRect textRect(0, 0, fontMetrics.width(text), fontMetrics.lineSpacing());
    QRect checkRect = check(option, model->data(index, Qt::CheckStateRole));
    doLayout(option, &checkRect, &pixmapRect, &textRect, true);

    return pixmapRect.unite(textRect).size();
}

/*!
    Returns the widget used to edit the item specified by the \a model and
    item \a index for editing. The \a parent widget and style \a option are
    used to control how the editor widget appears.

    \sa QAbstractItemDelegate::createEditor()
*/

QWidget *QItemDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &,
                                     const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    QVariant::Type t = index.model()->data(index, Qt::EditRole).type();
    QWidget *w = QItemEditorFactory::defaultFactory()->createEditor(t, parent);
    if (w) w->installEventFilter(const_cast<QItemDelegate *>(this));
    return w;
}

/*!
    Sets the data to be displayed and edited by the \a editor for the
    item specified by the \a model and item \a index.
*/

void QItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_D(const QItemDelegate);
    QVariant v = index.model()->data(index, Qt::EditRole);
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
    Q_D(const QItemDelegate);
    Q_ASSERT(model);
    QVariant::Type t = model->data(index, Qt::EditRole).type();
    QByteArray n = d->editorFactory()->valuePropertyName(t);
    if (!n.isEmpty())
        model->setData(index, editor->property(n), Qt::EditRole);
}

/*!
    Updates the \a editor for the item specified by the \a model and
    item \a index according to the style \a option given.
 */

void QItemDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    if (editor) {
        Q_ASSERT(index.isValid());
        const QAbstractItemModel *model = index.model();
        Q_ASSERT(model);
        QPixmap pixmap = decoration(option, model->data(index, Qt::DecorationRole));
        QString text = model->data(index, Qt::EditRole).toString();
        QRect pixmapRect = pixmap.rect();
        QRect textRect(0, 0, editor->fontMetrics().width(text), editor->fontMetrics().lineSpacing());
        QRect checkRect = check(option, model->data(index, Qt::CheckStateRole));
        doLayout(option, &checkRect, &pixmapRect, &textRect, false);
        editor->setGeometry(textRect);
    }
}

/*!
  Returns the editor factory used by the item delegate.
  If no editor factory is set, the function will return null.
*/
QItemEditorFactory *QItemDelegate::itemEditorFactory() const
{
    Q_D(const QItemDelegate);
    return d->f;
}

/*!
  Sets the editor factory to be used by the item delegate to be the \a factory
  specified. If no editor factory is set, the item delegate will use the
  default editor factory.
*/
void QItemDelegate::setItemEditorFactory(QItemEditorFactory *factory)
{
    Q_D(QItemDelegate);
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
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, option.palette.color(cg, QPalette::Highlight));
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    if (option.state & QStyle::State_Editing) {
        painter->save();
        painter->setPen(option.palette.color(cg, QPalette::Text));
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
        painter->restore();
    }

    QFont font = painter->font();
    painter->setFont(option.font);
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    if (painter->fontMetrics().width(text) > textRect.width())
        painter->drawText(textRect, option.displayAlignment,
                          ellipsisText(painter->fontMetrics(), textRect.width(),
                                       option.displayAlignment, text));
    else
        painter->drawText(textRect, option.displayAlignment, text);
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
        if (option.state & QStyle::State_Selected) {
            bool enabled = option.state & QStyle::State_Enabled;
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
    if (option.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        o.rect = rect;
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ?
                                  QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected) ?
                                                 QPalette::Highlight : QPalette::Background);
        QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
    }
}

/*!
    Renders a check indicator within the rectangle specified by \a rect,
    using the given \a painter and style \a option.
*/

void QItemDelegate::drawCheck(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QRect &rect, Qt::CheckState state) const
{
    if (!rect.isValid())
        return;

    QStyleOptionViewItem opt;
    opt.QStyleOption::operator=(option);
    opt.rect = rect;
    opt.state = opt.state & ~QStyle::State_HasFocus;

    switch (state) {
    case Qt::Unchecked:
        opt.state |= QStyle::State_Off;
        break;
    case Qt::PartiallyChecked:
        opt.state |= QStyle::State_NoChange;
        break;
    case Qt::Checked:
        opt.state |= QStyle::State_On;
        break;
    }

    QApplication::style()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt, painter);
}

/*!
    \internal
*/

void QItemDelegate::doLayout(const QStyleOptionViewItem &option,
                             QRect *checkRect, QRect *pixmapRect, QRect *textRect,
                             bool hint) const
{
    Q_ASSERT(checkRect && pixmapRect && textRect);
    int x = option.rect.left();
    int y = option.rect.top();
    int w, h;

    textRect->adjust(-textMargin, 0, textMargin, 0); // add width padding

    QSize pm(0, 0);
    if (pixmapRect->isValid())
        pm = option.decorationSize;
    if (hint) {
        w = qMax(textRect->width(), pm.width());
        h = qMax(textRect->height(), pm.height());
    } else {
        w = option.rect.width();
        h = option.rect.height();
    }

    int cw = 0;
    QRect check;
    if (checkRect->isValid()) {
        check.setRect(x, y, checkRect->width() + textMargin * 2, h);
        cw = check.width();
        if (option.direction == Qt::LeftToRight)
            x += cw;
    }

    QRect display;
    QRect decoration;
    QStyleOptionViewItem::Position position = option.decorationPosition;
    if (option.direction == Qt::RightToLeft) {
        if (position == QStyleOptionViewItem::Right)
            position = QStyleOptionViewItem::Left;
        else if (position == QStyleOptionViewItem::Left)
            position = QStyleOptionViewItem::Right;
    }
    switch (position) {
    case QStyleOptionViewItem::Top: {
        if (!pm.isEmpty())
            pm.setHeight(pm.height() + textMargin); // add space
        decoration.setRect(x, y, w, pm.height());
        h = hint ? textRect->height() : h - pm.height();
        display.setRect(x, y + pm.height(), w, h);
        break; }
    case QStyleOptionViewItem::Bottom: {
        if (!textRect->isEmpty())
            textRect->setHeight(textRect->height() + textMargin); // add space
        h = hint ? textRect->height() + pm.height() : h;
        decoration.setRect(x, y + h - pm.height(), w, pm.height());
        h = hint ? textRect->height() : h - pm.height();
        display.setRect(x, y, w, h);
        break; }
    case QStyleOptionViewItem::Left: {
        if (!pm.isEmpty())
            pm.setWidth(pm.width() + textMargin); // add space
        decoration.setRect(x, y, pm.width(), h);
        w = hint ? textRect->width() : w - pm.width() - cw;
        display.setRect(x + pm.width(), y, w, h);
        break; }
    case QStyleOptionViewItem::Right: {
        if (!textRect->isEmpty())
            textRect->setWidth(textRect->width() + textMargin); // add space
        w = hint ? textRect->width() + pm.width() : w;
        decoration.setRect(x + w - pm.width() - cw, y, pm.width(), h);
        w = hint ? textRect->width() : w - pm.width() - cw;
        display.setRect(x, y, w, h);
        break; }
    default:
        qWarning("doLayout: decoration positon is invalid");
        decoration = *pixmapRect;
        break;
    }

    if (!hint) { // we only need to do the internal layout if we are going to paint
        *checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                         checkRect->size(), check);
        *pixmapRect = QStyle::alignedRect(option.direction, option.decorationAlignment,
                                          pixmapRect->size(), decoration);
    } else {
        *checkRect = check;
        *pixmapRect = decoration;
    }
    *textRect = display;
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
        return qvariant_cast<QIcon>(variant).pixmap(option.decorationSize,
                                       option.state & QStyle::State_Enabled
                                       ? QIcon::Normal : QIcon::Disabled,
                                       option.state & QStyle::State_Open
                                       ? QIcon::On : QIcon::Off);
    case QVariant::Color: {
        static QPixmap pixmap(20, 20);
        pixmap.fill(qvariant_cast<QColor>(variant));
        return pixmap; }
    default:
        break;
    }
    return qvariant_cast<QPixmap>(variant);
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
        QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

        QColor color = palette.color(enabled
                                     ? QPalette::Normal
                                     : QPalette::Disabled,
                                     QPalette::Highlight);
        color.setAlphaF(0.3);

        QPainter painter(&img);
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.fillRect(0, 0, img.width(), img.height(), color);
        painter.end();

        pm = new QPixmap(QPixmap::fromImage(img));
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

/*!
  \internal
*/

QRect QItemDelegate::check(const QStyleOptionViewItem &option,
                           const QVariant &value) const
{
    if (value.isValid()) {
        QStyleOptionButton opt;
        opt.QStyleOption::operator=(option);
        opt.rect.setWidth(opt.rect.height());
        return QApplication::style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt);
    }
    return QRect();
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
    QWidget *editor = ::qobject_cast<QWidget*>(object);
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
    } else if (event->type() == QEvent::FocusOut && !editor->isActiveWindow()) {
        // The window may loose focus during an drag operation.
        // i.e when dragging involves the task bar on Windows.
        if (QDragManager::self() && QDragManager::self()->object != 0)
            return false;

        emit commitData(editor);
        emit closeEditor(editor, NoHint);
        return true;
    }
    return false;
}

/*!
  \reimp
*/

bool QItemDelegate::editorEvent(QEvent *event,
                                QAbstractItemModel *model,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    // make sure that we have the right event type and that the item is checkable
    if ((event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonDblClick)
        || ((model->flags(index) & Qt::ItemIsUserCheckable) == 0))
        return false;

    // check if the event happened in the right place
    QVariant value = model->data(index, Qt::CheckStateRole);
    QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                          check(option, value).size(),
                                          QRect(option.rect.x(), option.rect.y(),
                                                option.rect.height(), option.rect.height()));
    if (checkRect.contains(static_cast<QMouseEvent*>(event)->pos())) {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        return model->setData(index, (state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked),
                              Qt::CheckStateRole);
    }

    return false;
}

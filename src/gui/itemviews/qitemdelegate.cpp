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

#ifndef QT_NO_ITEMVIEWS
#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
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

void qt_format_text(const QFont&, const QRectF&,
                    int, const QString&, QRectF *,
                    int, int*, int, QPainter*);

static const int textMargin = 1;
static const bool doEliding = true;

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

    \section2 Subclassing

    When subclassing QItemDelegate to create a delegate that displays items
    using a custom renderer, it is important to ensure that the delegate can
    render items suitably for all the required states; e.g. selected,
    disabled, checked. The documentation for the paint() function contains
    some hints to show how this can be achieved.

    \sa {Model/View Programming}, QAbstractItemDelegate
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
    the item specified by \a index.

    When reimplementing this function in a subclass, you should use the
    \a option to determine the state of the item to be displated and adjust
    the way it is painted accordingly.

    For example, a selected item may need to be displayed differently to
    unselected items, as shown in the following code:

    \quotefromfile itemviews/pixelator/pixeldelegate.cpp
    \skipto QStyle::State_Selected
    \printuntil else
    \dots

    \sa QStyle::State
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

    // decoration
    value = model->data(index, Qt::DecorationRole);
    QPixmap pixmap = decoration(opt, value);
    QRect pixmapRect = (pixmap.isNull() ? QRect(0, 0, 0, 0)
                        : QRect(QPoint(0, 0), option.decorationSize));

    // display
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect textRect;
    if (!text.contains('\n')) {
        QFontMetrics fontMetrics(opt.font);
        textRect = QRect(0, 0, fontMetrics.width(text), fontMetrics.lineSpacing());
    } else {
        QRectF result;
        qt_format_text(opt.font, option.rect, Qt::TextDontPrint|Qt::TextDontClip,
                       text, &result, 0, 0, 0, painter);
        textRect = result.toRect();
    }

    // check
    value = model->data(index, Qt::CheckStateRole);
    QRect checkRect = check(opt, opt.rect, value);
    Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());

    doLayout(opt, &checkRect, &pixmapRect, &textRect, false);

    // draw the background color
    if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    } else {
        value = model->data(index, Qt::BackgroundColorRole);
        if (value.isValid() && qvariant_cast<QColor>(value).isValid())
            painter->fillRect(option.rect, qvariant_cast<QColor>(value));
    }

    // draw the item
    drawCheck(painter, opt, checkRect, checkState);
    drawDecoration(painter, opt, pixmapRect, pixmap);
    drawDisplay(painter, opt, textRect, text);
    drawFocus(painter, opt, textRect);
}

/*!
    Returns the size needed by the delegate to display the item
    specified by \a index, taking into account the style information
    provided by \a option.
*/

QSize QItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QVariant value = model->data(index, Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);

    // display
    value = model->data(index, Qt::FontRole);
    QFont fnt = value.isValid() ? qvariant_cast<QFont>(value) : option.font;
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect textRect;
    if (!text.contains('\n')) {
        QFontMetrics fontMetrics(fnt);
        textRect = QRect(0, 0, fontMetrics.width(text), fontMetrics.lineSpacing());
    } else {
        QRectF result;
        qt_format_text(fnt, option.rect, Qt::TextDontPrint|Qt::TextDontClip,
                       text, &result, 0, 0, 0, 0);
        textRect = result.toRect();
    }

    // decoration
    QRect pixmapRect;
    if (model->data(index, Qt::DecorationRole).isValid())
        pixmapRect = QRect(0, 0, option.decorationSize.width(),
                           option.decorationSize.height());

    // checkbox
    QRect checkRect = check(option, textRect, model->data(index, Qt::CheckStateRole));

    doLayout(option, &checkRect, &pixmapRect, &textRect, true);

    return (pixmapRect|textRect|checkRect).size();
}

/*!
    Returns the widget used to edit the item specified by \a index
    for editing. The \a parent widget and style \a option are used to
    control how the editor widget appears.

    \sa QAbstractItemDelegate::createEditor()
*/

QWidget *QItemDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &,
                                     const QModelIndex &index) const
{
    Q_D(const QItemDelegate);

    if (!index.isValid())
        return 0;
    QVariant::Type t = index.model()->data(index, Qt::EditRole).type();
    const QItemEditorFactory *factory = d->f;
    if (factory == 0)
        factory = QItemEditorFactory::defaultFactory();
    QWidget *w = factory->createEditor(t, parent);
    if (w) w->installEventFilter(const_cast<QItemDelegate *>(this));
    return w;
}

/*!
    Sets the data to be displayed and edited by the \a editor for the
    item specified by \a index.
*/

void QItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#ifndef QT_NO_PROPERTIES
    Q_D(const QItemDelegate);
    QVariant v = index.model()->data(index, Qt::EditRole);
    QByteArray n = d->editorFactory()->valuePropertyName(v.type());
    if (!n.isEmpty())
        editor->setProperty(n, v);
#endif
}

/*!
    Sets the data for the specified \a model and item \a index from that
    supplied by the \a editor.
*/

void QItemDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
#ifndef QT_NO_PROPERTIES
    Q_D(const QItemDelegate);
    Q_ASSERT(model);
    QVariant::Type t = model->data(index, Qt::EditRole).type();
    QByteArray n = d->editorFactory()->valuePropertyName(t);
    if (!n.isEmpty())
        model->setData(index, editor->property(n), Qt::EditRole);
#endif
}

/*!
    Updates the \a editor for the item specified by \a index
    according to the style \a option given.
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
        QRect checkRect = check(option, textRect, model->data(index, Qt::CheckStateRole));
        doLayout(option, &checkRect, &pixmapRect, &textRect, false);
        editor->setGeometry(textRect);
    }
}

/*!
  Returns the editor factory used by the item delegate.
  If no editor factory is set, the function will return null.

  \sa setItemEditorFactory()
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

  \sa itemEditorFactory()
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
        painter->fillRect(rect, option.palette.brush(cg, QPalette::Highlight));
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

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    QString str = text;
    if (painter->fontMetrics().width(text) > textRect.width() && !text.contains('\n'))
        str = elidedText(option.fontMetrics, textRect.width(), option.textElideMode, text);
    qt_format_text(option.font, textRect, option.displayAlignment, str, 0, 0, 0, 0, painter);
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
        o.state |= QStyle::State_KeyboardFocusChange;
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                                  ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected)
                                                 ? QPalette::Highlight : QPalette::Background);
        QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
    }
}

/*!
    Renders a check indicator within the rectangle specified by \a
    rect, using the given \a painter and style \a option, using the
    given \a state.
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
    if (pixmapRect->isValid()) {
        pm = option.decorationSize;
        pm.rheight() += 2 * textMargin;
        pm.rwidth() += 2 * textMargin;
    }
    if (hint) {
        h = qMax(textRect->height(), pm.height());
        if (option.decorationPosition == QStyleOptionViewItem::Left
            || option.decorationPosition == QStyleOptionViewItem::Right) {
            w = textRect->width() + pm.width();
        } else {
            w = qMax(textRect->width(), pm.width());
        }
    } else {
        w = option.rect.width();
        h = option.rect.height();
    }

    int cw = 0;
    QRect check;
    if (checkRect->isValid()) {
        cw = checkRect->width() + 2 * textMargin;
        if (hint) w += cw;
        if (option.direction == Qt::RightToLeft) {
            check.setRect(x + w - cw, y, cw, h);
        } else {
            check.setRect(x, y, cw, h);
        }
    }
    
    // at this point w should be the *total* width

    QRect display;
    QRect decoration;
    switch (option.decorationPosition) {
    case QStyleOptionViewItem::Top: {
        if (!pm.isEmpty())
            pm.setHeight(pm.height() + textMargin); // add space
        h = hint ? textRect->height() : h - pm.height();
        
        if (option.direction == Qt::RightToLeft) {
            decoration.setRect(x, y, w - cw, pm.height());
            display.setRect(x, y + pm.height(), w - cw, h);
        } else {
            decoration.setRect(x + cw, y, w - cw, pm.height());
            display.setRect(x + cw, y + pm.height(), w - cw, h);
        }
        break; }
    case QStyleOptionViewItem::Bottom: {
        if (!textRect->isEmpty())
            textRect->setHeight(textRect->height() + textMargin); // add space
        h = hint ? textRect->height() + pm.height() : h;
        
        if (option.direction == Qt::RightToLeft) {
            display.setRect(x, y, w - cw, textRect->height());
            decoration.setRect(x, y + h, w - cw, h - textRect->height());
        } else {
            display.setRect(x + cw, y, w - cw, textRect->height());
            decoration.setRect(x + cw, y + h, w - cw, h - textRect->height());        
        }
        break; }
    case QStyleOptionViewItem::Left: {
        if (option.direction == Qt::LeftToRight) {
            decoration.setRect(x + cw, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
        } else {
            display.setRect(x, y, w - pm.width() - cw, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
        }
        break; }
    case QStyleOptionViewItem::Right: {
        if (option.direction == Qt::LeftToRight) {
            display.setRect(x + cw, y, w - pm.width() - cw, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
        } else {
            decoration.setRect(x, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
        }
        break; }
    default:
        qWarning("doLayout: decoration position is invalid");
        decoration = *pixmapRect;
        break;
    }

    if (!hint) { // we only need to do the internal layout if we are going to paint
        *checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                         checkRect->size(), check);
        *pixmapRect = QStyle::alignedRect(option.direction, option.decorationAlignment,
                                          pixmapRect->size(), decoration);
        // the text takes up all awailable space, unless the decoration is not shown as selected
        if (option.showDecorationSelected)
            *textRect = display;
        else
            *textRect = QStyle::alignedRect(option.direction, option.displayAlignment,
                                            textRect->size().boundedTo(display.size()), display);
    } else {
        *checkRect = check;
        *pixmapRect = decoration;
        *textRect = display;
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

        QColor color = palette.color(enabled ? QPalette::Normal : QPalette::Disabled,
                                     QPalette::Highlight);
        color.setAlphaF(0.3);

        QPainter painter(&img);
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.fillRect(0, 0, img.width(), img.height(), color);
        painter.end();

        pm = new QPixmap(QPixmap::fromImage(img));
        QPixmapCache::insert(key, *pm);
    }
    return pm;
}

/*!
  \internal
*/

QRect QItemDelegate::check(const QStyleOptionViewItem &option,
                           const QRect &bounding, const QVariant &value) const
{
    if (value.isValid()) {
        QStyleOptionButton opt;
        opt.QStyleOption::operator=(option);
        opt.rect = bounding;
        return QApplication::style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt);
    }
    return QRect();
}

/*!
    \internal

    If the \a object is the current editor: if the \a event is an Esc
    key press the current edit is cancelled and ended, or if the \a
    event is an Enter or Return key press the current edit is accepted
    and ended. If editing is ended the event filter returns true to
    signify that it has handled the event; in all other cases it does
    nothing and returns false to signify that the event hasn't been
    handled.

    \sa closeEditor()
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
            emit closeEditor(editor, QAbstractItemDelegate::EditNextItem);
            return true;
        case Qt::Key_Backtab:
            emit commitData(editor);
            emit closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            emit commitData(editor);
            emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
            return true;
        case Qt::Key_Escape:
            // don't commit data
            emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
            return true;
        default:
            break;
        }
    } else if (event->type() == QEvent::FocusOut && !editor->isActiveWindow()) {
#ifndef QT_NO_DRAGANDDROP
        // The window may loose focus during an drag operation.
        // i.e when dragging involves the task bar on Windows.
        if (QDragManager::self() && QDragManager::self()->object != 0)
            return false;
#endif
        
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
    QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignLeft | Qt::AlignVCenter,
                                          check(option, option.rect, value).size(),
                                          QRect(option.rect.x(), option.rect.y(),
                                                option.rect.width(), option.rect.height()));
    if (checkRect.contains(static_cast<QMouseEvent*>(event)->pos())) {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        return model->setData(index, (state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked),
                              Qt::CheckStateRole);
    }

    return false;
}

#endif // QT_NO_ITEMVIEWS

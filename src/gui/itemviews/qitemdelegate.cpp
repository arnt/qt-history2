#include "qitemdelegate.h"
#include <qabstractitemview.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qimage.h>
#include <qstyle.h>

// FIXME make these static const  int
#define ItemSpacing 4
#define InternalSpacing 4

/*!
  \class QItemDelegate qitemdelegate.h

  \brief Renders and edits a data item from a model

  This class provides the functionality to render, lay out and offer
  editor functionality for a data item. Using supports() it decides
  which kind of items it can render and edit.

  To render an item reimplement paint() and sizeHint().

  A QItemDelegate can offer different ways for editing a data item.

  One way to edit an item is to create a widget on top of the item,
  which allows editing the contents of the item. For that
  createEditor() has to be reimplemented to create the editor widget,
  setContentFromEditor() has to be reimplemented to set the edited
  contents back to the data item and updateEditor() to update the
  editor in case the data of the item changed while being edited.

  The other way is to not create a widget but handle user events
  directly. For that keyPressEvent(), keyReleaseEvent(),
  mousePressEvent(), mouseMoveEvent(), mouseReleaseEvent() and
  mouseDoubleClickEvent() can be reimplemented.
*/

QItemDelegate::QItemDelegate(QGenericItemModel *model)
    : m(model)
{
}

QItemDelegate::~QItemDelegate()
{
}

bool QItemDelegate::supports(const QModelIndex &) const
{
//     bool stringSupport = model()->typeIndex(index, QVariant::String) >= 0;
//     bool iconsSupport = model()->typeIndex(index, QVariant::IconSet) >= 0;
    return true; // this is the default delegate, so we always return true
}

void QItemDelegate::paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const
{
    // FIXME: needs optimization
    QRect itemRect(0, 0, options.itemRect.width(), options.itemRect.height());
    QRect textRect = itemRect;
//    int textAlignment = AlignVCenter;
    int textElement = model()->element(item, QVariant::String);
    int iconElement = model()->element(item, QVariant::IconSet);

    if (iconElement >= 0) {
 	QIconSet::Mode mode = options.disabled ? QIconSet::Disabled : QIconSet::Normal; // FIXME: open
 	QIconSet::Size size = options.small ? QIconSet::Small : QIconSet::Large;
 	QIconSet::State state = options.selected ? QIconSet::On : QIconSet::Off;
	QIconSet icons = model()->data(item,iconElement).toIconSet();
	QPixmap icon = icons.pixmap(size, mode, state);
	QRect iconRect = icon.rect();
	if (options.iconAlignment == Qt::AlignTop) {
	    iconRect.moveLeft((textRect.width() - icon.width()) >> 1);
 	    textRect.setTop(icon.height() + ItemSpacing);
	    if (options.selected)
		painter->fillRect(textRect.x(), textRect.y(),
				  textRect.width(), textRect.height(),
				  options.palette.highlight());
	    // FIXME: use the following untill we have proper shaded icons
	    if (options.selected)
		painter->fillRect(iconRect, QBrush(options.palette.highlight(), QBrush::Dense4Pattern));
	    painter->drawPixmap(iconRect.x(), iconRect.y(), icon);
 	} else { // AlignLeft
	    iconRect.moveTop((textRect.height() - icon.height()) >> 1);
 	    textRect.setLeft(icon.width());
	    if (options.selected)
		painter->fillRect(itemRect.x(), itemRect.y(), itemRect.width(), itemRect.height(),
				  options.palette.highlight());
	    painter->drawPixmap(iconRect.x(), iconRect.y(), icon);
 	}


    } else if (options.selected) {
	painter->fillRect(textRect.x(), textRect.y(), textRect.width(), textRect.height(),
			  options.palette.highlight());
    }

    if (options.selected)
	painter->setPen(options.palette.highlightedText());
    else
	painter->setPen(options.palette.text());

    if (textElement >= 0)
	painter->drawText(textRect.x() + InternalSpacing/2, textRect.y() + InternalSpacing/2,
			  textRect.width() - InternalSpacing, textRect.height() - InternalSpacing,
			  options.textAlignment, model()->data(item, textElement).toString());

    if (options.selected)
	painter->setPen(options.palette.foreground());

    if (options.focus)
	if (options.editing)
	    painter->drawRect(textRect);
	else
	    QApplication::style().drawPrimitive(QStyle::PE_FocusRect, painter, textRect, options.palette);
}

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
			      const QModelIndex &item) const
{
    int textElement = model()->element(item, QVariant::String);
    int width = textElement >= 0 ?
		fontMetrics.width(model()->data(item, textElement).toString()) : 0;
    int height = fontMetrics.height();
    int iconElement = model()->element(item, QVariant::IconSet);

    if (iconElement >= 0) {
	QIconSet::Mode mode = options.disabled ? QIconSet::Disabled : QIconSet::Normal; // FIXME: open
	QIconSet::Size size = options.small ? QIconSet::Small : QIconSet::Large;
	QIconSet::State state = options.selected ? QIconSet::On : QIconSet::Off;
	QIconSet icons = model()->data(item, iconElement).toIconSet();
	QRect iconRect = icons.pixmap(size, mode, state).rect();
	if (options.iconAlignment == Qt::AlignTop) {
	    height += iconRect.height() + ItemSpacing;
	    width = qMax(width, iconRect.width());
	} else {
	    height = qMax(height, iconRect.height());
	    width += iconRect.width() + ItemSpacing;
	}
    }
    return QSize(width + ItemSpacing, height + ItemSpacing);
}

QItemDelegate::EditType QItemDelegate::editType(const QModelIndex &) const
{
    return WidgetOnTyping; // default
}

QWidget *QItemDelegate::createEditor(StartEditAction, QWidget *parent,
				     const QItemOptions &options, const QModelIndex &item) const
{
    QLineEdit *lineEdit = new QLineEdit(parent, "QItemDelegate_lineedit");
    lineEdit->setFrame(false);
    int textElement = model()->element(item, QVariant::String);
    lineEdit->setText(model()->data(item, textElement).toString());
    updateEditorGeometry(lineEdit, options, item);
    return lineEdit;
}

void QItemDelegate::setContentFromEditor(QWidget *editor, const QModelIndex &item) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit) {
	int textElement = model()->element(item, QVariant::String);
	model()->setData(item, textElement, lineEdit->text());
    }
}

void QItemDelegate::updateEditorContents(QWidget *editor, const QModelIndex &item) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit) {
	int textElement = model()->element(item, QVariant::String);
	lineEdit->setText(model()->data(item, textElement).toString());
    }
}

void QItemDelegate::updateEditorGeometry(QWidget *editor, const QItemOptions &options,
					 const QModelIndex &item) const
{
    QLineEdit *lineEdit = ::qt_cast<QLineEdit*>(editor);
    if (lineEdit) {
	QRect rect = textRect(options, item);
 	rect.setX(rect.x() + 1);
 	rect.setY(rect.y() + 1);
 	rect.setWidth(rect.width() - 1);
 	rect.setHeight(rect.height() - 1);
	lineEdit->setGeometry(rect);
    }
}

QRect QItemDelegate::textRect(const QItemOptions &options, const QModelIndex &item) const
{
    QRect txtRect = options.itemRect;
    int iconElement = model()->element(item, QVariant::IconSet);
    if (iconElement >= 0) {
	QIconSet::Mode mode = options.disabled ? QIconSet::Disabled : QIconSet::Normal; // FIXME: open
	QIconSet::Size size = options.small ? QIconSet::Small : QIconSet::Large;
	QIconSet::State state = options.selected ? QIconSet::On : QIconSet::Off;
	QIconSet icons = model()->data(item, iconElement).toIconSet();
	QRect iconRect = icons.pixmap(size, mode, state).rect();
	if (options.iconAlignment == Qt::AlignTop)
	    txtRect.setTop(txtRect.top() + iconRect.height());
	else
	    txtRect.setLeft(txtRect.left() + iconRect.width());
    }
    return txtRect;
}

#include "qitemdelegate.h"
#include <qabstractitemview.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qpoint.h>
#include <qrect.h>

QItemDelegate::QItemDelegate(QGenericItemModel *model)
    : QAbstractItemDelegate(model), spacing(4)
{
}

QItemDelegate::~QItemDelegate()
{
}

void QItemDelegate::paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const
{
    static QPoint pt(0, 0);

    int textElement = model()->element(item, QVariant::String);
    int iconElement = model()->element(item, QVariant::IconSet);
    QString text = model()->data(item,textElement).toString();
    QIconSet icons = model()->data(item,iconElement).toIconSet();

    QRect iconRect(pt, iconSize(options, icons));
    QRect textRect(pt, textSize(painter->fontMetrics(), options, text));
    doLayout(options, &iconRect, &textRect, true);

    drawIcon(painter, options, iconRect, icons);
    drawText(painter, options, textRect, text);
    drawFocus(painter, options, textRect);
}

QSize QItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
			      const QModelIndex &item) const
{
    static QPoint pt(0, 0);
    int iconElement = model()->element(item, QVariant::IconSet);
    int textElement = model()->element(item, QVariant::String);
    QRect iconRect(pt, iconSize(options, model()->data(item,iconElement).toIconSet()));
    QRect textRect(pt, textSize(fontMetrics, options, model()->data(item,textElement).toString()));
    doLayout(options, &iconRect, &textRect, false); // makes the rects valid
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
	QLineEdit *lineEdit = new QLineEdit(parent, "QItemDelegate_lineedit");
	lineEdit->setFrame(false);
	int textElement = model()->element(item, QVariant::String);
	lineEdit->setText(model()->data(item, textElement).toString());
	lineEdit->selectAll();
	updateEditorGeometry(lineEdit, options, item);
	return lineEdit;
    }
    return 0;
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

void QItemDelegate::updateEditorGeometry(QWidget *editor, const QItemOptions &options, const QModelIndex &item) const
{
    static QPoint pt(0, 0);
    if (editor) {
	int iconElement = model()->element(item, QVariant::IconSet);
	int textElement = model()->element(item, QVariant::String);
	QRect iconRect(pt, iconSize(options, model()->data(item,iconElement).toIconSet()));
 	QRect textRect(pt, textSize(editor->fontMetrics(), options, model()->data(item,textElement).toString()));
	doLayout(options, &iconRect, &textRect, true); // makes the rects valid
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
    if (options.iconAlignment & Qt::AlignLeft) // FIXME: spacing hack
	painter->drawText(QRect(rect.x() + spacing, rect.y(), rect.width() - spacing, rect.height()),
			  options.textAlignment, text);
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
    QIconSet::Mode mode = options.disabled ? QIconSet::Disabled : QIconSet::Normal; // FIXME: open
    QIconSet::Size size = options.smallItem ? QIconSet::Small : QIconSet::Large;
    QIconSet::State state = options.selected ? QIconSet::On : QIconSet::Off;
    painter->drawPixmap(rect, icons.pixmap(size, mode, state));
}

void QItemDelegate::drawFocus(QPainter *painter, const QItemOptions &options, const QRect &rect) const
{
    if (options.focus)
	QApplication::style().drawPrimitive(QStyle::PE_FocusRect, painter, rect, options.palette);    
}

void QItemDelegate::doLayout(const QItemOptions &options, QRect *iconRect, QRect *textRect, bool bound) const
{
    if (iconRect && textRect) {
	int x = options.itemRect.left();
	int y = options.itemRect.top();
	if (options.iconAlignment & Qt::AlignLeft) {
	    int height = bound ? options.itemRect.height() : qMax(textRect->height(), iconRect->height());
	    QRect leftRect(x, y, iconRect->width(), height);
	    QRect rightRect(x + iconRect->width(), y, textRect->width(), height);
	    iconRect->moveCenter(leftRect.center());
	    textRect->setRect(rightRect.x(), rightRect.y(),
			      bound ? options.itemRect.width() : rightRect.width() + spacing, rightRect.height());
	    return;
	}
	if (options.iconAlignment & Qt::AlignTop) {
	    int width = bound ? options.itemRect.width() : qMax(textRect->width(), iconRect->width());
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

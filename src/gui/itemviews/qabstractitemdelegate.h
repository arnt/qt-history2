#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#ifndef QT_H
#include <qobject.h>
#include <qpalette.h>
#include <qrect.h>
#endif

class QPainter;
class QAbstractItemView;
class QGenericItemModel;
class QModelIndex;

class Q_GUI_EXPORT QItemOptions
{
public:
    QItemOptions()
	: palette(), itemRect(), selected(false), open(false),
	  focus(false), disabled(false), smallItem(true), editing(false),
	  iconAlignment(Qt::AlignLeft|Qt::AlignVCenter),
	  textAlignment(Qt::AlignLeft|Qt::AlignVCenter) {}

    QPalette palette;
    QRect itemRect;
    uint selected : 1;
    uint open : 1;
    uint focus : 1;
    uint disabled : 1;
    uint smallItem : 1;
    uint editing : 1;
    int iconAlignment;
    int textAlignment;
};

class QAbstractItemDelegatePrivate;

class Q_GUI_EXPORT QAbstractItemDelegate : public QObject
{
    Q_DECLARE_PRIVATE(QAbstractItemDelegate);
    
public:
    QAbstractItemDelegate(QGenericItemModel *model, QObject *parent = 0);
    virtual ~QAbstractItemDelegate();

    QGenericItemModel *model() const;

    enum EditType {
	NoEditType,
	PersistentWidget,
	WidgetOnTyping,
	WidgetWhenCurrent,
	NoWidget
    };

    enum StartEditAction {
	NoAction = 0,
	CurrentChanged = 1,
	DoubleClicked = 2,
	SelectedClicked = 4,
	EditKeyPressed = 8,
	AnyKeyPressed = 16
    };

    // painting
    virtual void paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const = 0;
    virtual QSize sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
			   const QModelIndex &item) const = 0;

    // editing
    virtual EditType editType(const QModelIndex &item) const;
    virtual QWidget *createEditor(StartEditAction action, QWidget *parent, const QItemOptions &options,
				  const QModelIndex &item) const;
    virtual void setContentFromEditor(QWidget *editor, const QModelIndex &item) const;
    virtual void updateEditorContents(QWidget *editor, const QModelIndex &item) const;
    virtual void updateEditorGeometry(QWidget *editor, const QItemOptions &options, const QModelIndex &item) const;
    
protected:
    QAbstractItemDelegate(QAbstractItemDelegatePrivate &, QGenericItemModel* model, QObject *parent = 0);
};

#endif

#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#ifndef QT_H
#include <qpalette.h>
#include <qgenericitemmodel.h>
#endif

class QPainter;
class QAbstractItemView;

class Q_GUI_EXPORT QItemOptions
{
public:
    QItemOptions()
	: palette(), itemRect(), selected(false), open(false),
	  focus(false), disabled(false), smallItem(true), editing(false),
	  iconAlignment(Qt::AlignLeft|Qt::AlignVCenter),
	  textAlignment(Qt::AlignLeft|Qt::AlignVCenter) {}
	  //iconPosition(Left), textPosition(Center) {}

//     enum Position {
// 	Center = 0,
// 	Top = 1,
// 	Left = 2,
// 	Bottom = 4,
// 	Right = 8,
// 	TopLeft = Top|Left,
// 	TopRight = Top|Right,
// 	BottomLeft = Bottom|Left,
// 	BottomRight = Bottom|Right
//     };

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
//     Position iconPosition;
//     Position textPosition;
};

class Q_GUI_EXPORT QAbstractItemDelegate
{
public:
    QAbstractItemDelegate(QGenericItemModel *model) : m(model) {}
    virtual ~QAbstractItemDelegate() {}

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
    virtual QWidget *createEditor(StartEditAction action, QWidget *parent,
				  const QItemOptions &options, const QModelIndex &item) const;
    virtual void setContentFromEditor(QWidget *editor, const QModelIndex &item) const;
    virtual void updateEditorContents(QWidget *editor, const QModelIndex &item) const;
    virtual void updateEditorGeometry(QWidget *editor, const QItemOptions &options, const QModelIndex &item) const;

    inline const QGenericItemModel *itemModel() const { return m; }

    // non-widget editors
//     virtual void event( QEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void keyPressEvent( QKeyEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void keyReleaseEvent( QKeyEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void mousePressEvent( QMouseEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void mouseReleaseEvent( QMouseEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void mouseDoubleClickEvent( QMouseEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void mouseMoveEvent( QMouseEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void dragEnterEvent( QDragEnterEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void dragMoveEvent( QDragMoveEvent *e, QItemOptions *options, const QModelIndex &item );
//     virtual void dropEvent( QDropEvent *e, QItemOptions *options, const QModelIndex &item );

protected:
    inline QGenericItemModel *model() const { return m; }

private:
    QGenericItemModel *m;
};

#endif

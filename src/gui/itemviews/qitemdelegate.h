#ifndef QITEMDELEGATE_H
#define QITEMDELEGATE_H

#include <qpalette.h>
#include <qgenericitemmodel.h>

class QPainter;
class QAbstractItemView;

class QItemOptions
{
public:
    QItemOptions()
	: palette(), itemRect(), selected(false), open(false),
	  focus(false), disabled(false), small(true), editing(false),
	  iconAlignment(Qt::AlignLeft|Qt::AlignVCenter),
	  textAlignment(Qt::AlignLeft|Qt::AlignVCenter) {}

    QPalette palette;
    QRect itemRect;
    uint selected : 1;
    uint open : 1;
    uint focus : 1;
    uint disabled : 1;
    uint small : 1;
    uint editing : 1;
    int iconAlignment;
    int textAlignment;
};

class QItemDelegate
{
public:
    QItemDelegate(QGenericItemModel *model);
    virtual ~QItemDelegate();

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
	RenameKeyPressed = 8,
	AnyKeyPressed = 16
    };

    virtual bool supports(const QModelIndex &item) const;

    virtual void paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const;
    virtual QSize sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
			   const QModelIndex &item) const;

    virtual EditType editType(const QModelIndex &item) const;
    virtual QWidget *createEditor(StartEditAction action, QWidget *parent,
				  const QItemOptions &options, const QModelIndex &item) const;
    virtual void setContentFromEditor(QWidget *editor, const QModelIndex &item) const;
    virtual void updateEditorContents(QWidget *editor, const QModelIndex &item) const;
    virtual void updateEditorGeometry(QWidget *editor, const QItemOptions &options,
				      const QModelIndex &item) const;

    // for non-widget editors
//     virtual void event( QEvent *e, QItemOptions *options );
//     virtual void keyPressEvent( QKeyEvent *e, QItemOptions *options );
//     virtual void keyReleaseEvent( QKeyEvent *e, QItemOptions *options );
//     virtual void mousePressEvent( QMouseEvent *e, QItemOptions *options );
//     virtual void mouseReleaseEvent( QMouseEvent *e, QItemOptions *options );
//     virtual void mouseDoubleClickEvent( QMouseEvent *e, QItemOptions *options );
//     virtual void mouseMoveEvent( QMouseEvent *e, QItemOptions *options );
//     virtual void dragEnterEvent( QDragEnterEvent *e, QItemOptions *options );
//     virtual void dragMoveEvent( QDragMoveEvent *e, QItemOptions *options );
//     virtual void dropEvent( QDropEvent *e, QItemOptions *options );

protected:
    QRect textRect(const QItemOptions &options, const QModelIndex &item) const;
    inline QGenericItemModel *model() const { return m; }

private:
    QGenericItemModel *m;
};

#endif

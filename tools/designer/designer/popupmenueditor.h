#ifndef POPUPMENUEDITOR_H
#define POPUPMENUEDITOR_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qaction.h>

class PopupMenuEditor;

class PopupMenuEditorItem : public QObject
{
    Q_OBJECT

    friend class PopupMenuEditor;

    PopupMenuEditorItem( PopupMenuEditor * menu = 0 );

public:
    enum ItemType {
	Unknown = -1,
	Separator = 0,
	Action = 1,
	ActionGroup = 3,
	Widget = 4
    };

    // FIXME: Qt-ify the constructors
    PopupMenuEditorItem( QAction * action, PopupMenuEditor * menu );
    PopupMenuEditorItem( QActionGroup * actionGroup, PopupMenuEditor * menu );
    PopupMenuEditorItem( QWidget * widget, PopupMenuEditor * menu );
    PopupMenuEditorItem( PopupMenuEditorItem * item, PopupMenuEditor * menu );
    ~PopupMenuEditorItem();

    void init();

    ItemType type();

    QAction * action();
    QActionGroup * actionGroup();
    QAction * anyAction();
    QWidget * widget();

    void setSeparator( bool enable );
    bool isSeparator();

    void setVisible( bool enable );
    bool isVisible();

    void setRemovable( bool enable );
    bool isRemovable();

    void setAutoDelete( bool enable );
    bool isAutoDelete();

    void showMenu( int x, int y );
    void hideMenu();
    void focusMenu();
    PopupMenuEditor * menu() { return s; }

    int count();

    bool eventFilter( QObject *, QEvent * event );
    
public slots:
    void selfDestruct();

protected:

private:
    QAction * a;
    QActionGroup * g;
    QWidget * w;
    PopupMenuEditor * s;
    PopupMenuEditor * m;
    bool separator;
    bool removable;
    bool autodelete;
};

class FormWindow;
class MainWindow;
class QLineEdit;

#include <qpopupmenu.h>

class PopupMenuEditor : public QWidget
{
    Q_OBJECT

    friend class PopupMenuEditorItem;
    friend class MenuBarEditor;
    friend class Resource;

public:
    PopupMenuEditor( FormWindow * fw, QWidget * parent = 0, const char * name = 0 );
    PopupMenuEditor( FormWindow * fw, PopupMenuEditor * menu, QWidget * parent, const char * name = 0 );
    ~PopupMenuEditor();

    void init();

    void insert( PopupMenuEditorItem * item, int index = -1 );
    void insert( QAction * action, int index = -1 );
    void insert( QActionGroup * actionGroup, int index = -1 );
    void insert( QWidget * widget, int index = -1 );
    int find( QAction * action );
    int count();
    PopupMenuEditorItem * at( int index );
    void exchange( int a, int b );

    void cut( int index );
    void copy( int index );
    void paste( int index );

    void insertedActions( QPtrList<QAction> & list );

    void show();
    void choosePixmap( int index = -1 );
    void showLineEdit( int index = -1);
    void setAccelerator( int key, Qt::ButtonState state, int index = -1 );
    void resizeToContents();

    void showCurrentItemMenu();
    void hideCurrentItemMenu();
    void focusCurrentItemMenu();

    FormWindow * formWindow() { return formWnd; }
    bool isCreatingAccelerator() { return ( currentField == 2 ); }

    QPtrList<PopupMenuEditorItem> * items() { return &itemList; }

signals:
    void inserted( QAction * );
    void removed(  QAction * );
    
public slots:
    void remove( int index );
    void remove( QAction * );
    
protected:
    PopupMenuEditorItem * createItem( QAction * a = 0 );
    void deleteCurrentItem();
    PopupMenuEditorItem * currentItem();
    PopupMenuEditorItem * itemAt( const int y );
    void setFocusAt( const QPoint & pos );

    void paintEvent( QPaintEvent * e );
    void mousePressEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );
    void dragEnterEvent( QDragEnterEvent * e );
    void dragLeaveEvent( QDragLeaveEvent * e );
    void dragMoveEvent( QDragMoveEvent * e );
    void dropEvent( QDropEvent * e );
    void keyPressEvent( QKeyEvent * e );
    void focusInEvent( QFocusEvent * e );
    void focusOutEvent( QFocusEvent * e );

    void drawPopup( QPainter & p );
    void drawItems( QPainter & p );
    int drawAction( QPainter & p, QAction * a, int x, int y );
    int drawActionGroup( QPainter & p, QActionGroup * g, int x, int y );
    int drawSeparator( QPainter & p, const int y );
    void drawArrow( QPainter & p, const int y );
    void drawToggle( QPainter & p, const int y );
    void drawWinFocusRect( QPainter & p, const int y );

    QSize contentsSize();
    int itemSize( QPainter & p, PopupMenuEditorItem * i );
    int actionSize( QPainter & p, QAction * a );
    int actionGroupSize( QPainter & p, QActionGroup * g );

    int currentItemYCoord();
    int snapToItem( int y );
    void dropInPlace( PopupMenuEditorItem * i, int y );

    void safeDec();
    void safeInc();

    void clearCurrentField();
    void navigateUp( bool ctrl );
    void navigateDown( bool ctrl );
    void navigateLeft();
    void navigateRight();
    void enterEditMode( QKeyEvent * e );
    void leaveEditMode( QKeyEvent * e );

private:
    FormWindow * formWnd;
    QLineEdit * lineEdit;
    QWidget * dropLine;
    QPtrList<PopupMenuEditorItem> itemList;
    PopupMenuEditorItem addItem;
    PopupMenuEditorItem addSeparator;
    QWidget * parentMenu;
    int iconWidth;
    int textWidth;
    int acceleratorWidth;
    int arrowWidth;
    int widgetWidth;
    int separatorHeight;
    int itemHeight;
    int borderSize;
    int currentField;
    uint currentIndex;
    QPoint mousePressPos;
    static PopupMenuEditorItem * draggedItem;

    enum ClipboardOperation {
	None = 0,
	Cut = 1,
	Copy = 2
    };
    static int clipboardOperation;
    static PopupMenuEditorItem * clipboardItem;
};

#endif //POPUPMENUEDITOR_H

#ifndef POPUPMENUEDITOR_H
#define POPUPMENUEDITOR_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qaction.h>

class PopupMenuEditor;
class QMenuItem;

class PopupMenuEditorItem : public QObject
{
    Q_OBJECT

    friend class PopupMenuEditor;

    PopupMenuEditorItem( PopupMenuEditor * menu = 0, QObject * parent = 0, const char * name = 0 );

public:
    enum ItemType {
	Unknown = -1,
	Separator = 0,
	Action = 1,
	ActionGroup = 3
    };

    PopupMenuEditorItem( QAction * action, PopupMenuEditor * menu,
			 QObject * parent = 0, const char * name = 0 );
    PopupMenuEditorItem( QActionGroup * actionGroup, PopupMenuEditor * menu,
			 QObject * parent = 0, const char * name = 0 );
    PopupMenuEditorItem( PopupMenuEditorItem * item, PopupMenuEditor * menu,
			 QObject * parent = 0, const char * name = 0 );
    ~PopupMenuEditorItem();

    void init();

    ItemType type() const;
    
    QAction * action() const { return a; }
    QActionGroup * actionGroup() const { return g; }
    QAction * anyAction() const { return ( a ? a : ( QAction * ) g ); }

    void setVisible( bool enable );
    bool isVisible() const;

    void setSeparator( bool enable ) { separator = enable; }
    bool isSeparator() const { return separator; }

    void setRemovable( bool enable ) { removable = enable; }
    bool isRemovable() const { return removable; }
    
    void setDirty( bool enable ) { dirty = enable; }
    bool isDirty() const { return dirty; }

    void showMenu( int x, int y );
    void hideMenu();
    void focusOnMenu();
    PopupMenuEditor * subMenu() const { return s; }

    int count() const;

    bool eventFilter( QObject *, QEvent * event );
    
public slots:
    void selfDestruct();

protected:

private:
    QAction * a;
    QActionGroup * g;
    PopupMenuEditor * s;
    PopupMenuEditor * m;
    int separator : 1;
    int removable : 1;
    int dirty : 1;
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

    void insert( PopupMenuEditorItem * item, const int index = -1 );
    void insert( QAction * action, const int index = -1 );
    void insert( QActionGroup * actionGroup, const int index = -1 );
    int find( const QAction * action );
    int find( PopupMenuEditor * menu );
    int count();
    PopupMenuEditorItem * at( const int index );
    void exchange( const int a, const int b );

    void cut( const int index );
    void copy( const int index );
    void paste( const int index );

    void insertedActions( QPtrList<QAction> & list );

    void show();
    void choosePixmap( const int index = -1 );
    void showLineEdit( const int index = -1);
    void setAccelerator( const int key, const Qt::ButtonState state, const int index = -1 );
    void resizeToContents();

    void showSubMenu( const int index = - 1 );
    void hideSubMenu( const int index = - 1 );
    void focusOnSubMenu( const int index = - 1 );

    FormWindow * formWindow() { return formWnd; }
    bool isCreatingAccelerator() { return ( currentField == 2 ); }

    QPtrList<PopupMenuEditorItem> * items() { return &itemList; }

    QWidget * parentEditor() { return parentMenu; }

signals:
    void inserted( QAction * );
    void removed(  QAction * );
    
public slots:

    void cut() { cut( currentIndex ); }
    void copy() { copy( currentIndex ); }
    void paste() { paste( currentIndex ); }

    void remove( const int index );
    void remove( QAction * a ) { remove( find( a ) ); }
    
protected:
    PopupMenuEditorItem * createItem( QAction * a = 0 );
    void removeItem( const int index = -1 );
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

    void drawItems( QPainter * p );
    void drawItem( QPainter * p, PopupMenuEditorItem * i, const QRect & r, const int f ) const;
    void drawWinFocusRect( QPainter * p, const QRect & r ) const;

    QSize contentsSize();
    int itemHeight( const PopupMenuEditorItem * item ) const;
    int itemPos( const PopupMenuEditorItem * item ) const;
    
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
    int accelWidth;
    int arrowWidth;
    int borderSize;
    
    int currentField;
    int currentIndex;
    int drawAll;
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

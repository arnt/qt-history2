#ifndef MENUBAREDITOR_H
#define MENUBAREDITOR_H

/*#include <qwidget.h>*/
#include <qmenubar.h>
#include <qptrlist.h>

class PopupMenuEditor;
class MenuBarEditor;
class QActionGroup;

class MenuBarEditorItem : public QObject
{
    Q_OBJECT

    friend class MenuBarEditor;

    // FIXME: Qt-ify the constructors
    MenuBarEditorItem( MenuBarEditor * bar = 0, int id = -1 );
    
public:
    MenuBarEditorItem( PopupMenuEditor * menu, MenuBarEditor * bar, int id = -1 );
    MenuBarEditorItem( QActionGroup * actionGroup, MenuBarEditor * bar, int id = -1 );
    MenuBarEditorItem( MenuBarEditorItem * item, int id = -1 );
    ~MenuBarEditorItem();

    PopupMenuEditor * menu();//FIXME: rename to popup ?
    int id();

    void setMenuText( const QString t );
    QString menuText();

    void setVisible( bool enable );
    bool isVisible();

    void setRemovable( bool enable );
    bool isRemovable();

    void setAutoDelete( bool enable );
    bool isAutoDelete();

     bool isSeparator();
protected:
    void setSeparator( bool enable );
    
private:
    MenuBarEditor * menuBar;
    PopupMenuEditor * popupMenu;
    QString text;
    bool visible;
    bool separator;
    bool removable;
    bool autodelete;
    int identity;
};

class QLineEdit;
class FormWindow;

class MenuBarEditor : public QMenuBar
{
    Q_OBJECT
    
public:
    MenuBarEditor( FormWindow * fw, QWidget * parent = 0, const char * name = 0 );
    ~MenuBarEditor();

    FormWindow * formWindow();

    MenuBarEditorItem * createItem( int index = -1 );
    void insertItem( MenuBarEditorItem * item, int index = -1 );
    void insertItem( QString text, PopupMenuEditor * menu, int id = -1, int index = -1 );
    void insertItem( QString text, QActionGroup * group, int id = -1, int index = -1 );

    void insertSeparator( int index = -1 );
    
    void removeItemAt( int index );
    void removeItem( MenuBarEditorItem * item );
    void removeItem( int id );
    
    int findItem( MenuBarEditorItem * item );
    int findItem( PopupMenuEditor * menu );
    int findItem( int id );
    int findItem( QPoint & pos );
    
    MenuBarEditorItem * item( int index = -1 );

    int count();
    int current();

    void cut( int index );
    void copy( int index );
    void paste( int index );
    void exchange( int a, int b );

    void showLineEdit( int index = -1);
    void showItem( int index = -1 );
    void hideItem( int index = -1 );
    void focusItem( int index = -1 );
    void deleteItem( int index = -1 );

    QSize sizeHint() const;
    QSize minimumSize() const { return sizeHint(); }
    QSize minimumSizeHint() const { return sizeHint(); }
    int heightForWidth( int max_width ) const;

    void show();

    void checkAccels( QMap<QChar, QWidgetList > &accels );
    
protected:

    void paintEvent( QPaintEvent * e );
    void mousePressEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );
    void dragEnterEvent( QDragEnterEvent * e );
    void dragLeaveEvent( QDragLeaveEvent * e );
    void dragMoveEvent( QDragMoveEvent * e );
    void dropEvent( QDropEvent * e );
    void keyPressEvent( QKeyEvent * e );
    void focusOutEvent( QFocusEvent * e );
    void resizeEvent( QResizeEvent * e ) { QFrame::resizeEvent( e ); }

    void resizeInternals();
    
    void drawItems( QPainter & p );
    void drawItem( QPainter & p, MenuBarEditorItem * i, int & x, int & y, uint & c );
    
    QSize itemSize( QPainter & p, MenuBarEditorItem * i );
    void addItemSizeToCoords( QPainter & p, MenuBarEditorItem * i, int & x, int & y, const int w );

    QPoint itemPos( const int index );
    QPoint snapToItem( const QPoint & pos );
    void dropInPlace( MenuBarEditorItem * i, const QPoint & pos );

    void safeDec();
    void safeInc();

    void navigateLeft( bool ctrl );
    void navigateRight( bool ctrl );
    void enterEditMode();
    void leaveEditMode();
    
private:
    FormWindow * formWnd;
    QLineEdit * lineEdit;
    QWidget * dropLine;
    QPtrList<MenuBarEditorItem> itemList;
    MenuBarEditorItem addItem;
    MenuBarEditorItem addSeparator;
    MenuBarEditorItem * draggedItem;
    QPoint mousePressPos;
    uint currentIndex;
    int itemHeight;
    int separatorWidth;
    int borderSize;
    bool hideWhenEmpty;
    bool hasSeparator;
    
    enum ClipboardOperation {
	None = 0,
	Cut = 1,
	Copy = 2
    };
    static int clipboardOperation;
    static MenuBarEditorItem * clipboardItem;
};

#endif //MENUBAREDITOR_H

/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    void insert( QActionGroup * actionGroup, int index = -1, bool children = TRUE );
    int find( const QAction * action );
    int find( PopupMenuEditor * menu );
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

    void showSubMenu( int index = - 1 );
    void hideSubMenu( int index = - 1 );
    void focusOnSubMenu( int index = - 1 );

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

    void remove( int index );
    void remove( QAction * a ) { remove( find( a ) ); }
    
protected:
    PopupMenuEditorItem * createItem( QAction * a = 0 );
    void removeItem( int index = -1 );
    PopupMenuEditorItem * currentItem();
    PopupMenuEditorItem * itemAt( int y );
    void setFocusAt( const QPoint & pos );

    bool eventFilter( QObject * o, QEvent * e );
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
    void drawItem( QPainter * p, PopupMenuEditorItem * i, const QRect & r, int f ) const;
    void drawWinFocusRect( QPainter * p, const QRect & r ) const;

    QSize contentsSize();
    int itemHeight( const PopupMenuEditorItem * item ) const;
    int itemPos( const PopupMenuEditorItem * item ) const;
    
    int snapToItem( int y );
    void dropInPlace( PopupMenuEditorItem * i, int y );
    void dropInPlace( QActionGroup * g, int y );

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

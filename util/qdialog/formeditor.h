#ifndef __dformeditor_h__
#define __dformeditor_h__

#include <qwidget.h>
#include <qrect.h>
#include <qptrdict.h>
#include <qpoint.h>
#include <qlist.h>
#include <qlayout.h>
#include <qcolor.h>
#include <qsizepolicy.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include "widgetsbar.h"
#include "gridopt.h"

class QXMLTag;
class QResource;

class DObjectInfo
{
public:
  struct Connection
  {
    QString senderName;
    QString receiverName;
    QString slot;
    QString signal;

    bool operator== ( const Connection& _c ) const { return ( senderName == _c.senderName && slot == _c.slot && signal == _c.signal && receiverName == _c.receiverName ); }
  };

  DObjectInfo( QWidget* _widget );
  ~DObjectInfo();

  bool isSelected() const { return m_selected; }
  void setSelected( bool _mode );

  QWidget* widget() { return m_widget; }

  void startMove();
  void move( int _orientation, int _dx, int _dy );
  void endMove();

  void update() { updateSizeHandles(); }

  QValueList<Connection>::Iterator connectionsBegin() { return m_connections.begin(); }
  QValueList<Connection>::Iterator connectionsEnd() { return m_connections.end(); }
  /**
   * The sender of the connection has to be the widget managed by this instance.
   */
  void addConnection( const Connection& _connection ) { m_connections.append( _connection ); }
  void removeConnection( const Connection& _connection );

  void addCustomSignal( const QString& _signal ) { m_customSignals.append( _signal ); }
  void addCustomSlot( const QString& _slot ) { m_customSlots.append( _slot ); }
  void removeCustomSignal( const QString& _signal ) { m_customSignals.remove( _signal ); }
  void removeCustomSlot( const QString& _slot ) { m_customSlots.remove( _slot ); }

  const QStringList& customSignals() const { return m_customSignals; }
  const QStringList& customSlots() const { return m_customSlots; }

protected:
  void updateSizeHandles();

private:
  QWidget* m_widget;
  QWidget* m_sizeHandles[8];
  bool m_selected;
  QRect m_originalGeometry;  // Used during move
  /**
   * Holds all connections in which the widget mannaged by this instance is
   * acting as a sender.
   */
  QValueList<Connection> m_connections;
  /**
   * Holds all custom signals.
   */
  QStringList m_customSignals;
  QStringList m_customSlots;
};

class DSizeHandle : public QWidget
{
  Q_OBJECT
public:
  DSizeHandle( int _orientation, bool _active, DObjectInfo* _info, QWidget* _parent = 0, const char* _name = 0 );
  ~DSizeHandle();

protected:
  void mousePressEvent( QMouseEvent* _ev );
  void mouseMoveEvent( QMouseEvent* _ev );
  void mouseReleaseEvent( QMouseEvent* _ev );

private:
  DObjectInfo* m_info;
  int m_orientation;
  QPoint m_mousePos;
  bool m_active;
};

class DFormEditor;

class DFormWidget : public QWidget
{
  Q_OBJECT
  Q_BUILDER( "", "", DefaultInspector )

public:
  enum Layout { NoLayout, VBoxLayout, HBoxLayout, GridLayout };
  enum Mode { TopMost, Container, LayoutHelper, GridHelper };

  DFormWidget( Mode _mode, DFormEditor* _editor, QWidget* _parent = 0, const char* _name = 0 );
  DFormWidget( QWidget* parent, const QResource& _resource );

  ~DFormWidget();

  QSizePolicy sizePolicy() const;

  bool isTopMostForm() const { return m_mode == DFormWidget::TopMost; }
  DFormWidget::Mode mode() const { return m_mode; }

  void setLayout( DFormWidget::Layout _l, bool _force = FALSE );
  Layout layoutType() { return m_layout; }
  
  bool isLayouted() const { return ( m_layout != DFormWidget::NoLayout ); }
  void updateLayout() { setLayout( m_layout, TRUE ); }

  void replace( QWidget* _old, QWidget *_new );

  /**
   * The returned tag is of type "Widget" or "Layout".
   * But if this is a topmost form then it always returns "QWidget"
   */
  QXMLTag* save();

  bool configure( const QResource& _resource );

protected:
  void dragEnterEvent ( QDragEnterEvent *_ev );
  void dragMoveEvent ( QDragMoveEvent *_ev );
  void dragLeaveEvent ( QDragLeaveEvent *_ev );
  void dropEvent ( QDropEvent *_ev ); 

  void mousePressEvent( QMouseEvent* _ev );
  void mouseMoveEvent( QMouseEvent* _ev );
  void mouseReleaseEvent( QMouseEvent* _ev );

  void paintEvent( QPaintEvent* _ev );
  void resizeEvent( QResizeEvent* _ev );
  
  void drawDragShadow( const QRect& _rect );
  void clearDragShadow();

  void popupMenu( const QPoint& _pos );

protected slots:
  void slotAlignVertical();
  void slotAlignHorizontal();
  void slotAlignGrid();
  void slotNoAlign();
  void slotCreateContainer();
  void slotDeleteContainer();
  
private:
  void replace( int _row, int _col, QWidget* _w );

  QColor bgcolor() const;
  
  QPoint m_mousePress;
  QRect m_dragShadow;
  DWidgetDrag::Widget m_dragInfo;
  // bool m_drag;

  DFormEditor* m_editor;
  Mode m_mode;

  QList<QObject> m_selectedChildren;

  DFormWidget::Layout m_layout;
  QVBoxLayout* m_vBoxLayout;
  QHBoxLayout* m_hBoxLayout;
  DGridLayout* m_gridLayout;
};

class DFormEditor : public QWidget
{
  Q_OBJECT
public:
  DFormEditor( QWidget* _parent = 0, const char* _name = 0 );
  ~DFormEditor();  

  void addWidget( QWidget* _w );

  void unselectAll() { selectObjectExclusive( 0 ); }
  void updateSizeHandles();

  DObjectInfo* findInfo( QObject* _o ) { return m_widgets.find( _o ); }
  DObjectInfo* info( const QString& _obj_name );

  void save( QXMLTag* _tag );
  bool load( const QResource& _resource );

  bool isTopLevelWidget( QWidget* _widget ) { return _widget == m_topLevelWidget; }

  /**
   * Returns the list of all connections which have '_receiver' as their receiver.
   */
  QValueList<DObjectInfo::Connection> connectionsToReceiver( const QString& _receiver );

  static DFormEditor* loadingInstance() { return s_pLoadingInstance; }

public slots:
  /***********************
   * Menu slots
   ***********************/
  void slotAutoArrange();
  void slotGridArrange();
  void slotVArrange();
  void slotHArrange(); 
  void slotApplySizeHint();

protected slots:
  /***********************
   * popupmenu slots
   ***********************/
  void slotConnect();

signals:
  void objectSelected( DFormEditor* _editor, DObjectInfo* _obj );
  
protected:
  bool eventFilter( QObject* _obj, QEvent* _ev );

  void selectObjectExclusive( DObjectInfo* _info );

  void simpleArrange( DFormWidget::Layout _l );

private:
  QWidget* m_form;
  QWidget* m_topLevelWidget;
  QPtrDict<DObjectInfo> m_widgets;
  DObjectInfo* m_pPopupWidget;
  DObjectInfo* m_pConnectWidget;

  static DFormEditor* s_pLoadingInstance;
};

#endif

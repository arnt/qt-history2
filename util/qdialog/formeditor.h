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

#include "widgetsbar.h"
#include "gridopt.h"

class DObjectInfo
{
public:
  DObjectInfo( QWidget* _widget );
  ~DObjectInfo();

  bool isSelected() { return m_selected; }
  void setSelected( bool _mode );

  QWidget* widget() { return m_widget; }

  void move( int _orientation, int _dx, int _dy );

  void update() { updateSizeHandles(); }

protected:
  void updateSizeHandles();

private:
  QWidget* m_widget;
  QWidget* m_sizeHandles[8];
  bool m_selected;
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
public:
  enum Layout { NoLayout, VBoxLayout, HBoxLayout, GridLayout };
  enum Mode { TopMost, Container, LayoutHelper };

  DFormWidget( Mode _mode, DFormEditor* _editor, QWidget* _parent = 0, const char* _name = 0 );
  ~DFormWidget();

  QSizePolicy sizePolicy() const;

  bool isTopMostForm() const { return m_mode == DFormWidget::TopMost; }
  DFormWidget::Mode mode() const { return m_mode; }

  void setLayout( DFormWidget::Layout _l, bool _force = FALSE );
  bool isLayouted() const { return ( m_layout != DFormWidget::NoLayout ); }
  void updateLayout() { setLayout( m_layout, TRUE ); }

protected:
  void dragEnterEvent ( QDragEnterEvent *_ev );
  void dragMoveEvent ( QDragMoveEvent *_ev );
  void dragLeaveEvent ( QDragLeaveEvent *_ev );
  void dropEvent ( QDropEvent *_ev ); 

  void mousePressEvent( QMouseEvent* _ev );
  void mouseMoveEvent( QMouseEvent* _ev );
  void mouseReleaseEvent( QMouseEvent* _ev );

  void drawDragShadow( const QRect& _rect );
  void clearDragShadow();

  void popupMenu( const QPoint& _pos );

protected slots:
  void slotAlignVertical();
  void slotAlignHorizontal();
  void slotAlignGrid();
  void slotNoAlign();

private:
  QPoint m_mousePress;
  QRect m_dragShadow;
  DWidgetDrag::Widget m_dragInfo;
  bool m_drag;

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

  DObjectInfo* findInfo( QObject* _o ) { return m_widgets.find( _o ); }

public slots:
  /***********************
   * Menu slots
   ***********************/
  void slotAutoArrange();
  void slotGridArrange();
  void slotVArrange();
  void slotHArrange(); 
  void slotApplySizeHint();

protected:
  bool eventFilter( QObject* _obj, QEvent* _ev );

  void selectObjectExclusive( DObjectInfo* _info );

  void simpleArrange( DFormWidget::Layout _l );

private:
  DFormWidget* m_form;
  QPtrDict<DObjectInfo> m_widgets;
};

#endif

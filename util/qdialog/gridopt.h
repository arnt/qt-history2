#ifndef __gridopt_h__
#define __gridopt_h__

#include <qwidget.h>
#include <qlist.h>
#include <qvaluelist.h>
#include <qlayout.h>
#include <qpoint.h>
#include <qrect.h>

#include "qtable.h"

// #define GRID_TEST

class QXMLTag;
class QResource;
class DFormEditor;

class DRange
{
public:
  DRange( int _left, int _right ) { m_left = _left; m_right = _right; swap(); }
  DRange( const DRange& _r ) { m_left = _r.m_left; m_right = _r.m_right; }
  DRange() { m_left = -1; m_right = -2; }

  bool isValid() const { return ( m_left <= m_right ); }

  void setCoords( int _left, int _right ) { m_left = _left; m_right = _right; swap(); }
  void setGeometry( int _left, int _width );

  int left() const { return m_left; }
  int right() const { return m_right; }
  int width() const { return m_right - m_left + 1; }

  bool intersects( const DRange& _r ) const;
  bool contains( const DRange& _r ) const;
  bool contains( int point ) const;

  bool isLeftEdge( const DRange& _r ) const;
  bool isRightEdge( const DRange& _r ) const;
  bool isLeftEdge( int point ) const;
  bool isRightEdge( int point ) const;

  void alignLeft( int point );
  void alignRight( int point );

private:
  void swap();

  int m_left;
  int m_right;
};

class DFormEditor;

class DGridLayout : public QGridLayout
{
  Q_OBJECT
  Q_BUILDER( "", "" )

public:
  struct Cell
  {
    Cell() { align = 0; w = 0; multicol = 1; multirow = 1; isOverlapped = FALSE; }
    Cell( const Cell& _w ) { align = _w.align; w = _w.w; multicol = _w.multicol;
                             multirow = _w.multirow; isOverlapped = _w.isOverlapped; }
    QWidget* w;
    int multicol;
    int multirow;
    bool isOverlapped;
    int align;
  };
  struct Row
  {
    Row() { stretch = 0; }
    Row( const Row& r ) { top = r.top; bottom = r.bottom; stretch = r.stretch; }
    int top;
    int bottom;
    int stretch;
  };
  struct Col
  {
    Col() { stretch = 0; }
    Col( const Col& r ) { left = r.left; right = r.right; stretch = r.stretch; }
    int left;
    int right;
    int stretch;
  };

  typedef QTable<Row,Col,Cell> Matrix;

  enum Insert { InsertCol, InsertRow, InsertNone };
  
  DGridLayout( QWidget* _parent, int _rows = 1, int _cols = 1, int _outborder = 6, int _innerspace = 6 );
  DGridLayout( QWidget* _parent, const QResource& _resource );
  DGridLayout( QWidget* _parent, const Matrix& _m, int _outborder = 6, int _innerspace = 6 );

  void addWidget2( QWidget* _widget, int _row, int _col, int _align = 0);
  void addMultiCellWidget2( QWidget* _widget, int _fromrow, int _torow,
			    int _fromcol, int _tocol, int _align = 0 );

  const Cell& cell( uint _row, uint _col ) const { return m_matrix.cell( _row, _col ); }
  const Row& row( uint _row ) const { return m_matrix.row( _row ); }
  const Col& col( uint _col ) const { return m_matrix.col( _col ); }
  const Matrix& matrix() const { return m_matrix; }
  
  uint rows() const { return m_matrix.rows(); }
  uint cols() const { return m_matrix.cols(); }
  
  void fillWithForms( DFormEditor* _editor );
  void updateGeometry();

  Insert insertTest( const QPoint& _p, int *_row, int *_col );
  QRect insertRect( Insert _ins, uint _row, uint _col );

  QXMLTag* save( DFormEditor* ) const;

  bool configure( const QResource& _resource );
 
private:  
  Matrix m_matrix;
};

DGridLayout* dGuessGrid( QWidget* _parent, QList<QWidget>& _widgets );

#endif

#ifndef __gridopt_h__
#define __gridopt_h__

#include <qwidget.h>
#include <qlist.h>
#include <qvaluelist.h>
#include <qlayout.h>

// #define GRID_TEST

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
public:
  DGridLayout( QWidget* _parent, int _rows, int _cols, int _outborder = 0, int _innerspace = -1 );

  void addWidget2( QWidget* _widget, int _row, int _col, int _align = 0);
  void addMultiCellWidget2( QWidget* _widget, int _fromrow, int _torow, int _fromcol, int _tocol, int _align = 0 );

  QWidget* widget( int _row, int _col );
  int multicol( int _row, int _col );
  int multirow( int _row, int _col );

  void fillWithForms( DFormEditor* _editor, QWidget* _parent );

private:
  struct Widget
  {
    Widget() { w = 0; multicol = 1; multirow = 1; isOverlapped = FALSE; }
    Widget( const Widget& _w ) { w = _w.w; multicol = _w.multicol; multirow = _w.multirow; }
    QWidget* w;
    int multicol;
    int multirow;
    bool isOverlapped;
  };

  QValueList<Widget> m_widgets;
};

DGridLayout* dGuessGrid( QWidget* _parent, QList<QWidget>& _widgets );

#endif

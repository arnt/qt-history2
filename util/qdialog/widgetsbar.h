#ifndef __dwidgetsbar_h__
#define __dwidgetsbar_h__

#include <qtabwidget.h>
#include <qlabel.h>
#include <qpoint.h>
#include <qdragobject.h>

#include "widgetinfo.h"

class QTabBar;
class QWidgetStack;
class QHBoxLayout;

class DWidgetDrag : public QDragObject
{
  Q_OBJECT
public:
  struct Widget
  {
    QSize sizeHint;
    QString className;
  };

  DWidgetDrag( const QString& _class, const QSize& _size, QWidget * _dragSource, const char* _name = 0 );
  ~DWidgetDrag();

  const char* format(int i) const;
  QByteArray encodedData(const char* mime) const;
  
  static bool canDecode( QMimeSource* e );
  static bool decode( QMimeSource* e, Widget& _list );

private:
  Widget data;
};

class DWidgetButton : public QLabel
{
  Q_OBJECT
public:
  DWidgetButton( const DWidgetInfo& _info, QWidget* _parent = 0, const char* _name = 0 );
  ~DWidgetButton();

protected:
  void mousePressEvent( QMouseEvent* _ev );
  void mouseMoveEvent( QMouseEvent* _ev );
  void mouseReleaseEvent( QMouseEvent* _ev );

private:
  QPoint m_pressedPos;
  bool m_pressed;
  DWidgetInfo m_info;
};

class DWidgetsPage : public QFrame
{
  Q_OBJECT
public:
  DWidgetsPage( QWidget* _parent = 0, const char* _name = 0 );
  ~DWidgetsPage();

private:
  QHBoxLayout* m_layout;
};

class DWidgetsBar : public QTabWidget
{
  Q_OBJECT
public:
  DWidgetsBar( QWidget* _parent = 0, const char* _name = 0 );
  ~DWidgetsBar();

protected:
  DWidgetsPage* addPage( const QString& _label );
};

#endif

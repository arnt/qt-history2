#ifndef __windowmanager_h__
#define __windowmanager_h__

#include <qframe.h>

class QPixmap;

class DWindowManager : public QFrame
{
  Q_OBJECT
public:
  DWindowManager( QWidget* parent = 0, const char* name = 0 );
  ~DWindowManager();

protected:
  void paintEvent( QPaintEvent* );

private:
  static QPixmap* s_titleLeft;
  static QPixmap* s_titleRight;
};

#endif

#ifndef __dinspector_h__
#define __dinspector_h__

#include <qframe.h>

class DInspector : public QFrame
{
  Q_OBJECT
public:
  DInspector( QWidget* _parent = 0, const char* _name = 0 );
  ~DInspector();
};

#endif

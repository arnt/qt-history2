#ifndef DLAYOUT_H
#define DLAYOUT_H

#include <qwidget.h>

class QResource;

class DStretch : public QWidget
{
  Q_OBJECT
  Q_BUILDER( "A stretch", "dstretch.xpm", DefaultInspector )
public:
  DStretch( QWidget* _parent = 0, const char* _name = 0 );
  DStretch( QWidget* _parent, const QResource& _resource );
  ~DStretch();

  virtual QSizePolicy sizePolicy() const;
  virtual QSize sizeHint() const;

  void setOrientation( Qt::Orientation );
  Qt::Orientation orientation() { return m_orientation; }

q_properties:
  void setStretch( int );
  int stretch();

private:
  int m_stretch;
  Qt::Orientation m_orientation;
};

class DSpacing : public QWidget
{
  Q_OBJECT
  Q_BUILDER( "A spacing", "dspacing.xpm", DefaultInspector )
public:
  DSpacing( QWidget* _parent = 0, const char* _name = 0 );
  DSpacing( QWidget* _parent, const QResource& _resource );
  ~DSpacing();

  virtual QSizePolicy sizePolicy() const;
  virtual QSize sizeHint() const;
  
  void setOrientation( Qt::Orientation );
  Qt::Orientation orientation() { return m_orientation; }
  
q_properties:
  void setSpacing( int );
  int spacing();

private:
  int m_spacing;
  Qt::Orientation m_orientation;
};

#endif

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <qwidget.h>
#include "dlldefs.h"

class QChildEvent;
class MarkerWidget;
class QHBoxLayout;

class EDITOR_EXPORT ViewManager : public QWidget
{
    Q_OBJECT

public:
    ViewManager( QWidget *parent, const char *name );

    void addView( QWidget *view );
    QWidget *currentView() const;

    void setError( int line );

protected:
    void childEvent( QChildEvent *e );
    void resizeEvent( QResizeEvent *e );

private:
    QWidget *curView;
    MarkerWidget *markerWidget;
    QHBoxLayout *layout;

};

#endif

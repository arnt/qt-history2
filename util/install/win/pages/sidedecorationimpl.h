#ifndef SIDEDECORATIONIMPL_H
#define SIDEDECORATIONIMPL_H
#include "sidedecoration.h"
#include "pages.h"
#include <qpixmap.h>
#include <qlabel.h>

class SideDecorationImpl : public SideDecoration
{ 
    Q_OBJECT

public:
    SideDecorationImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~SideDecorationImpl();

public slots:
    void wizardPages( const QPtrList<Page>& );
    void wizardPageShowed( int );
    void wizardPageFailed( int );

private:
    QPixmap checkPix;
    QPixmap arrowPix;
    QPixmap crossPix;
    QPtrList<QLabel> bullets;
    int activeBullet;
};

#endif // SIDEDECORATIONIMPL_H

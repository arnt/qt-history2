#ifndef ICONLOADER_H
#define ICONLOADER_H

#include "qiconset.h"
#include "qpixmap.h"

class IconLoader
{
public:
    IconLoader();
    
    static QIconSet iconSet( const QString &icon );
    static QPixmap pixmap( const QString &pixmap );
    
};

#endif

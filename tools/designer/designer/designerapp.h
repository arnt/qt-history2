#ifndef DESIGNERAPP_H
#define DESIGNERAPP_H

#include <qguardedptr.h>
#include <qdatetime.h>

#if defined(HAVE_KDE)
#include <kapp.h>
class DesignerApplication : public KApplication
#else
#include <qapplication.h>
class DesignerApplication : public QApplication
#endif
{
public:
#if defined(HAVE_KDE)
    DesignerApplication( int &argc, char **argv, const QCString &rAppName );
#else
    DesignerApplication( int &argc, char **argv );
#endif
    
protected:
    QDateTime lastMod;
    
#if defined(Q_WS_WIN) 
    bool winEventFilter( MSG *msg );
    uint DESIGNER_OPENFILE;
#endif
    
};


#endif

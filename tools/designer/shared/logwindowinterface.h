#ifndef LOGWINDOWINTERFACE_H
#define LOGWINDOWINTERFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>
#include <qwidget.h>

// {bb92356f-a8f7-4c42-9967-4961a72d0ef2}
Q_UUID( IID_LogWindowInterface,
	0xbb92356f, 0xa8f7, 0x4c42, 0x99, 0x67, 0x49, 0x61, 0xa7, 0x2d, 0x0e, 0xf2 );

class LogWindowInterface : public QUnknownInterface
{
public:
    enum Window {
	Error = 0,
	Debug = 1,
	All = Error | Debug
    };

    virtual QStringList featureList() const = 0;
    virtual QWidget *logWindow( QWidget *parent ) = 0;
    virtual void clear( Window win ) = 0;
    virtual void setError( const QString &message, int line ) = 0;
    virtual void setDebug( const QString &message ) = 0;

};

#endif

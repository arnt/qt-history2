#include <qevent.h>

class InstallEvent : public QCustomEvent
{
public:
    InstallEvent( QString, int );

    QString fileName;
    int progress;

    enum {
	eventID = QEvent::User
    };
};
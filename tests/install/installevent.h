#include <qevent.h>

class InstallEvent : public QCustomEvent
{
public:
    InstallEvent( int, QString message = QString::null, int progress = 0 );

    QString fileName;
    int progress;
	int eType;

	enum {
		updateProgress,
		moveNext
	} eventType;

    enum {
		eventID = QEvent::User
    };
};
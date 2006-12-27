
#define QT_NO_KEYWORDS
#undef signals
#undef slots
#undef emit
#define signals FooBar
#define slots Baz
#define emit Yoyodyne

#include <QtGui/QtGui>
#include <QtCore>
#include <QtNetwork/QtNetwork>
#include <QtSql/QtSql>
#include <QtSvg>
#if defined(QT3_SUPPORT)
#include <Qt3Support>
#endif
#if defined(WITH_DBUS)
#include <QtDBus>
#endif

#undef signals
#undef slots
#undef emit

class MyBooooooostishClass : public QObject
{
    Q_OBJECT
public:
    inline MyBooooooostishClass() {}

Q_SIGNALS:
    void mySignal();

public Q_SLOTS:
    inline void mySlot() { mySignal(); }

private:
    int signals;
    double slots;
};

#define signals protected
#define slots
#define emit
#undef QT_NO_KEYWORDS


#ifndef QAXSCRIPT_H
#define QAXSCRIPT_H

#include <qobject.h>
#include <qvariant.h>

class QAxBase;
class QAxScriptSite;
class QAxScriptInstance;

class QAxScript : public QObject
{
    Q_OBJECT

public:
    QAxScript( QObject *parent = 0, const char *name = 0 );

    enum ScriptState {
	Uninitialized = 0,
	Initialized = 5,
	Started = 1,
	Connected = 2,
	Disconnected = 3,
	Closed = 4
    };

    void addObject(QAxBase *object);

    QStringList functions() const;

    bool load(const QString &file, const QString &name = QString());
    bool load(const QString &code, const QString &language, const QString &name );
    void unload(const QString &name);

    QVariant call(const QString &function, QValueList<QVariant> &arguments = QValueList<QVariant>());

signals:
    void entered();
    void finished();
    void finished(const QVariant &result);
    void finished(int code, const QString &source,const QString &description, const QString &help);
    void stateChanged(int state);
    void error(int code, const QString &description, int sourcePosition, const QString &sourceText);

private slots:
    void objectDestroyed(QObject *o);

private:
    void updateScripts();
    void updateScript(QAxScriptInstance*);
    QAxScriptInstance *script(const QString &function) const;

    friend class QAxScriptSite;
    friend class QAxScriptInstance;
    QAxScriptSite *scriptSite;
};

#endif // QAXSCRIPT_H

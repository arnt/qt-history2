#ifndef QAXSCRIPT_H
#define QAXSCRIPT_H

#include <qobject.h>
#include <qvariant.h>

class QAxBase;
class QAxObject;
class QAxScriptSite;
class QAxScriptEngine;

class QAxScript : public QObject
{
    Q_OBJECT

public:
    enum ScriptState {
	Uninitialized = 0,
	Initialized = 5,
	Started = 1,
	Connected = 2,
	Disconnected = 3,
	Closed = 4
    };


    QAxScript( QObject *parent = 0, const char *name = 0 );

    void addObject(QAxBase *object);

    QStringList functions() const;
    QStringList scriptNames() const;
    QAxObject *scriptEngine(const QString &name) const;

    QAxObject *load(const QString &code, const QString &language, const QString &name );
    QAxObject *load(const QString &file, const QString &name = QString());
    void unload(const QString &name);

    QVariant call(const QString &function, QValueList<QVariant> &arguments = QValueList<QVariant>());

    static bool registerEngine(const QString &name, const QString &extension, const QString &code = QString());
    static QString scriptFileFilter();

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
    friend class QAxScriptSite;
    friend class QAxScriptEngine;

    void updateScripts();
    void updateScript(QAxScriptEngine*);
    QAxScriptEngine *script(const QString &function) const;

    QAxScriptSite *scriptSite;
};

#endif // QAXSCRIPT_H

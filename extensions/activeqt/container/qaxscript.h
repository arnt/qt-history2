#ifndef QAXSCRIPT_H
#define QAXSCRIPT_H

#include <qaxobject.h>
#include <qvariant.h>

class QAxScriptSite;
class QAxScript;

class QAxScriptManager : public QObject
{
    Q_OBJECT

public:
    QAxScriptManager( QObject *parent = 0, const char *name = 0 );

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

public slots:
    QVariant call(const QString &function, QValueList<QVariant> &arguments = QValueList<QVariant>());

signals:
    void entered();
    void finished();
    void finished(const QVariant &result);
    void finished(int code, const QString &source,const QString &description, const QString &help);
    void stateChanged(int state);
    void error(int code, const QString &description, int sourcePosition, const QString &sourceText);

protected:
    virtual void updateScripts();

private slots:
    void objectDestroyed(QObject *o);

private:
    void updateScript(QAxScript*);
    QAxScript *script(const QString &function) const;

    friend class QAxScriptSite;
    friend class QAxScript;
    QAxScriptSite *scriptSite;
};

#endif // QAXSCRIPT_H

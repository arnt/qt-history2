#ifndef QAXSCRIPT_H
#define QAXSCRIPT_H

#include <qaxobject.h>
#include <qvariant.h>

class QAxBase;
class QAxScriptSite;
class QAxScriptEngine;
struct IActiveScript;

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

    enum FunctionFlags {
	FunctionNames = 0,
	FunctionPrototypes	
    };

    QAxScript( QObject *parent = 0, const char *name = 0 );

    void addObject(QAxBase *object);

    QStringList functions(FunctionFlags = FunctionNames) const;
    QStringList scriptNames() const;
    QAxScriptEngine *scriptEngine(const QString &name) const;

    QAxScriptEngine *load(const QString &code, const QString &name, const QString &language);
    QAxScriptEngine *load(const QString &file, const QString &name);

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

    void updateScript(QAxScriptEngine*);
    QAxScriptEngine *script(const QString &function) const;

    QAxScriptSite *scriptSite;
};

class QAxScriptEngine : public QAxObject
{
public:
    QAxScriptEngine(const QString &name, QAxScript *manager);
    ~QAxScriptEngine();

    virtual bool load(const QString &code, const QString &language);

    bool isValid() const;
    bool hasIntrospection() const;

    QString scriptName() const;
    QString scriptCode() const;
    QString scriptLanguage() const;

    QStringList functions(QAxScript::FunctionFlags = QAxScript::FunctionNames) const;

    long queryInterface( const QUuid &, void** ) const;

protected:
    bool initialize(IUnknown** ptr);

private:
    friend class QAxScript;

    QAxScript *script_manager;
    IActiveScript *script;

    QString script_name;
    QString script_code;
    QString script_language;
};

inline bool QAxScriptEngine::isValid() const
{
    return script != 0;
}

inline QString QAxScriptEngine::scriptName() const
{
    return script_name;
}

inline QString QAxScriptEngine::scriptCode() const
{
    return script_code;
}

inline QString QAxScriptEngine::scriptLanguage() const
{
    return script_language;
}

#endif // QAXSCRIPT_H

#include "qaxscript.h"

#include <qaxobject.h>
#include <qdict.h>
#include <qfile.h>
#include <qmetaobject.h>
#include <qvaluelist.h>
#include <qwidget.h>

#include <qt_windows.h>
#include <activscp.h>
#include "..\shared\types.h"

struct QAxEngineDescriptor { QString name, extension, code; };
static QValueList<QAxEngineDescriptor> engines;

/*
    \class QAxScriptEngine
    \brief The QAxScriptEngine class provides a wrapper around scripts.
    \internal
*/
class QAxScriptEngine : public QAxObject
{
public:
    QAxScriptEngine(const QString &name, QAxScript *manager);
    ~QAxScriptEngine();

    bool isValid() const;
    QStringList functions() const;

    bool initialize(IUnknown** ptr);
    void addItem(const QString &name);
    bool load(const QString &code, const QString &language);

    QString scriptName() const;

private:
    QAxScript *script_manager;
    IActiveScript *script;

    QString script_code;
    QString script_language;
    QString script_name;
};


/*
    \class QAxScriptSite
    \brief The QAxScriptSite class implements a Windows Scripting Host
    \internal

    The QAxScriptSite is used internally to communicate callbacks from the script
    engine to the script manager.
*/

class QAxScriptSite : public IActiveScriptSite,
		      public IActiveScriptSiteWindow
{
public:
    QAxScriptSite(QAxScript *manager);

    ULONG WINAPI AddRef();
    ULONG WINAPI Release();
    HRESULT WINAPI QueryInterface(REFIID iid, void **ppvObject);

    HRESULT WINAPI GetLCID(LCID *plcid);
    HRESULT WINAPI GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
    HRESULT WINAPI GetDocVersionString(BSTR *pbstrVersion);

    HRESULT WINAPI OnScriptTerminate(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
    HRESULT WINAPI OnStateChange(SCRIPTSTATE ssScriptState);
    HRESULT WINAPI OnScriptError(IActiveScriptError *pscripterror);
    HRESULT WINAPI OnEnterScript();
    HRESULT WINAPI OnLeaveScript();

    HRESULT WINAPI GetWindow(HWND *phwnd);
    HRESULT WINAPI EnableModeless(BOOL fEnable);

    QPtrList<QAxScriptEngine> scriptList;
    QDict<QAxBase> objectDict;

protected:
    QWidget *topLevelWidget() const;

private:
    QAxScript *scriptManager;
    unsigned long ref;
};

/*
    Constructs the site for the \a manager.
*/
QAxScriptSite::QAxScriptSite(QAxScript *manager)
: scriptManager(manager), ref(1)
{
}

/*
    Implements IUnknown::AddRef
*/
ULONG WINAPI QAxScriptSite::AddRef()
{
    return ++ref;
}

/*
    Implements IUnknown::Release
*/
ULONG WINAPI QAxScriptSite::Release()
{
    if (!--ref) {
	delete this;
	return 0;
    }
    return ref;
}

/*
    Implements IUnknown::QueryInterface
*/
HRESULT WINAPI QAxScriptSite::QueryInterface(REFIID iid, void **ppvObject)
{
    *ppvObject = 0;
    if (iid == IID_IUnknown)
	*ppvObject = (IUnknown*)(IActiveScriptSite*)this;
    else if (iid == IID_IActiveScriptSite)
	*ppvObject = (IActiveScriptSite*)this;
    else if (iid == IID_IActiveScriptSiteWindow)
	*ppvObject = (IActiveScriptSiteWindow*)this;
    else
	return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

/*
    Implements IActiveScriptSite::GetLCID

    This method is not implemented. Use the system-defined locale.
*/
HRESULT WINAPI QAxScriptSite::GetLCID(LCID * /*plcid*/)
{
    return E_NOTIMPL;
}

/*
    Implements IActiveScriptSite::GetItemInfo

    Tries to find the QAxBase for \a pstrName and returns the 
    relevant interfaces in \a item and \a type as requested through \a mask.
*/
HRESULT WINAPI QAxScriptSite::GetItemInfo(LPCOLESTR pstrName, DWORD mask, IUnknown **item, ITypeInfo **type)
{
    if (item)
	*item = 0;
    else if (mask & SCRIPTINFO_IUNKNOWN)
	return E_POINTER;

    if (type)
	*type = 0;
    else if (mask & SCRIPTINFO_ITYPEINFO)
	return E_POINTER;

    QString name(QString::fromUcs2(pstrName));
    QAxBase *object = objectDict.find(name);
    if (!object)
	return TYPE_E_ELEMENTNOTFOUND;

    if (mask & SCRIPTINFO_IUNKNOWN)
	object->queryInterface(IID_IUnknown, (void**)item);
    if (mask & SCRIPTINFO_ITYPEINFO) {
	IProvideClassInfo *classInfo = 0;
	object->queryInterface(IID_IProvideClassInfo, (void**)&classInfo);
	if (classInfo) {
	    classInfo->GetClassInfo(type);
	    classInfo->Release();
	}
    }
    return S_OK;
}

/*
    Implements IActiveScriptSite::GetDocVersionString

    This method is not implemented. The scripting engine should assume 
    that the script is in sync with the document.
*/
HRESULT WINAPI QAxScriptSite::GetDocVersionString(BSTR * /*version*/)
{
    return E_NOTIMPL;
}

/*
    Implements IActiveScriptSite::OnScriptTerminate

    This method is usually not called, but if it is it fires
    QAxScript::finished().
*/
HRESULT WINAPI QAxScriptSite::OnScriptTerminate(const VARIANT *result, const EXCEPINFO *exception)
{
    emit scriptManager->finished();

    if (result && result->vt != VT_EMPTY)
	emit scriptManager->finished(VARIANTToQVariant(*result, 0));
    if (exception)
	emit scriptManager->finished(exception->wCode, 
				     BSTRToQString(exception->bstrSource),
				     BSTRToQString(exception->bstrDescription),
				     BSTRToQString(exception->bstrHelpFile)
				    );
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnEnterScript

    Fires QAxScript::entered() to inform the host that the 
    scripting engine has begun executing the script code.
*/
HRESULT WINAPI QAxScriptSite::OnEnterScript()
{
    emit scriptManager->entered();
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnLeaveScript

    Fires QAxScript::finished() to inform the host that the 
    scripting engine has returned from executing the script code.
*/
HRESULT WINAPI QAxScriptSite::OnLeaveScript()
{
    emit scriptManager->finished();
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnScriptError

    Fires QAxScript::error() to inform the host that an 
    that an execution error occurred while the engine was running the script.
*/
HRESULT WINAPI QAxScriptSite::OnScriptError(IActiveScriptError *error)
{
    EXCEPINFO exception;
    DWORD context;
    ULONG lineNumber;
    LONG charPos;
    BSTR bstrLineText;
    QString lineText;

    error->GetExceptionInfo(&exception);
    error->GetSourcePosition(&context, &lineNumber, &charPos);
    HRESULT hres = error->GetSourceLineText(&bstrLineText);
    if (hres == S_OK)
	lineText = BSTRToQString(bstrLineText);
    else if (context)
	lineText = ((QAxScriptEngine*)context)->scriptName();

    emit scriptManager->error(exception.wCode, BSTRToQString(exception.bstrDescription),
				    lineNumber, lineText);
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnStateChange

    Fires QAxScript::stateChanged() to inform the
    the host that the scripting engine has changed states.
*/
HRESULT WINAPI QAxScriptSite::OnStateChange(SCRIPTSTATE ssScriptState)
{
    emit scriptManager->stateChanged(ssScriptState);
    return S_OK;
}

/*
    \internal
    Returns the toplevel widget parent of this script, or
    null if there is no widget parent.
*/
QWidget *QAxScriptSite::topLevelWidget() const
{
    QWidget *w = 0;
    QObject *p = scriptManager->parent();
    while (!w && p) {
	w = ::qt_cast<QWidget*>(p);
	p = p->parent();
    }

    if (w)
	w = w->topLevelWidget();
    return w;
}

/*
    Implements IActiveScriptSiteWindow::GetWindow

    Retrieves the handle to a window that can act as the owner of a 
    pop-up window that the scripting engine must display.
*/
HRESULT WINAPI QAxScriptSite::GetWindow(HWND *phwnd)
{
    if (!phwnd)
	return E_POINTER;

    *phwnd = 0;
    QWidget *w = topLevelWidget();
    if (!w)
	return E_FAIL;

    *phwnd = w->winId();
    return S_OK;
}

/*
    Implements IActiveScriptSiteWindow::EnableModeless

    Causes the host to enable or disable its main window 
    as well as any modeless dialog boxes.
*/
HRESULT WINAPI QAxScriptSite::EnableModeless(BOOL fEnable)
{
    QWidget *w = topLevelWidget();
    if (!w)
	return E_FAIL;

    EnableWindow(w->winId(), fEnable);
    return S_OK;
}

/*
    Constructs a QAxScriptEngine object with name \a name and registers
    it in the QAxScript \a manager.

    \a name should not be empty.
*/
QAxScriptEngine::QAxScriptEngine(const QString &name, QAxScript *manager )
: QAxObject(0, manager, name.latin1()), script_manager(manager), 
  script(0), script_name(name)
{
    disableClassInfo();
    disableEventSink();

    if (manager)
	manager->scriptSite->scriptList.prepend(this);
}

/*
    Destroys the QAxScriptEngine object, releasing all allocated
    resources and unregistering the script from its manager.
*/
QAxScriptEngine::~QAxScriptEngine()
{
    if (script_manager)
	script_manager->scriptSite->scriptList.removeRef(this);

    if (script) {
	script->SetScriptState( SCRIPTSTATE_DISCONNECTED );
	script->Close();
	script->Release();
    }
}

/*
    Returns the name of the script engine.
*/
inline QString QAxScriptEngine::scriptName() const
{
    return script_name;
}

/*
    Loads the script source \a code into the script, using the script
    engine for \a language. The function returns TRUE if \a code could
    be passed successfully into the script engine, otherwise returns
    FALSE.
*/
bool QAxScriptEngine::load(const QString &code, const QString &language)
{
    if (script)
	return FALSE;

    script_language = language;
    script_code += code;

    metaObject();
    return isValid();
}

/*
    \reimp
*/
bool QAxScriptEngine::initialize(IUnknown **ptr)
{
    *ptr = 0;

    CLSID clsid;
    HRESULT hres = CLSIDFromProgID( script_language.ucs2(), &clsid );
    if(FAILED(hres))
	return FALSE;

    CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**)&script);
    if (!script)
	return FALSE;

    IActiveScriptParse *parser = 0;
    script->QueryInterface(IID_IActiveScriptParse, (void**)&parser);
    if (!parser) {
	script->Release();
	script = 0;
	return FALSE;
    }

    if (script->SetScriptSite(script_manager->scriptSite) != S_OK) {
	script->Release();
	script = 0;
	return FALSE;
    }
    if (parser->InitNew() != S_OK) {
	parser->Release();
	script->Release();
	script = 0;
	return FALSE;
    }

    hres = parser->ParseScriptText(QStringToBSTR(script_code), 0, 0, 0, DWORD(this), 0, SCRIPTTEXT_ISVISIBLE, 0, 0);

    parser->Release();
    parser = 0;

    script_manager->updateScript(this);

    if (script->SetScriptState( SCRIPTSTATE_CONNECTED ) != S_OK) {
	script = 0;
	return FALSE;
    }

    IDispatch *scriptDispatch = 0;
    script->GetScriptDispatch(0, &scriptDispatch);
    if (scriptDispatch) {
	scriptDispatch->QueryInterface(IID_IUnknown, (void**)ptr);
	scriptDispatch->Release();
    }

    return *ptr != 0;
}

/*
    Adds an item \a name to the script engine.
    The item can then be addressed in the script code.

    \sa QAxScript::addObject()
*/
void QAxScriptEngine::addItem(const QString &name)
{
    if (!script)
	return;

    script->AddNamedItem(name.ucs2(), SCRIPTITEM_ISSOURCE|SCRIPTITEM_ISVISIBLE);
}

/*
    Returns TRUE if the script engine has been initialized
    correctly, otherwise returns FALSE.
*/
bool QAxScriptEngine::isValid() const
{
    return script != 0;
}

/*
    Returns a list of all functions this script can run.
*/
QStringList QAxScriptEngine::functions() const
{
    QStringList functions;

    const QMetaObject *mo = metaObject();
    for (int i = 0; i < mo->numSlots(TRUE); ++i) {
	if (i < mo->slotOffset())
	    continue;

	const QMetaData *slot = mo->slot(i, TRUE);
	if (!slot || slot->access != QMetaData::Public)
	    continue;
	QString slotname = QString::fromLatin1(slot->name);
	if (slotname.contains('_'))
	    continue;

	functions << slotname;
    }

    return functions;
}

/*!
    \class QAxScript qaxscript.h
    \brief The QAxScript class provides an interface to the Windows Script Host.

    The QAxScript acts as a bridge between the COM objects embedded in the Qt 
    application through QAxObject or QAxWidget, and the scripting languages
    available through the Windows Script technologies, usually JScript and VBScript.

    Create one QAxScript for each separate document in your application, and
    add the COM objects the script need to access using addObject(). Then load()
    the script sources and invoke the functions using call().

    Every script can have a name which can later be used to unload or access the
    script engine created for each script. The script engines provide feedback to 
    the application through signals. The most important signal is the error() 
    signal.
*/

/*!
    \enum QAxScript::ScriptState

    The ScriptState enumeration defines the different states a script engine can
    be in.

    \value Uninitialized The script is created, but not yet initialized
    \value Initialized The script has been initialized, but is not running
    \value Started The script can execute code, but does not yet handle events
    \value Connected The script can execute code and connected to handle events    
    \value Disconnected The script is loaded, but is not connected to event sources
    \value Closed The script has been closed.
*/

/*! \fn void QAxScript::entered()

    This signal is emitted when a script engine has begun executing code.
*/

/*! \fn void QAxScript::finished()

    This signal is emitted when a script engine has returned from executing code.
*/

/*! \overload void QAxScript::finished(const QVariant &result)

    \a result contains the script's result if appropriate.
*/

/*! \overload void QAxScript::finished(int code, const QString &source,
				    const QString &description, const QString &help)

    \a code, \a source, \a description and \a help contain exception information
    when the script terminated.
*/

/*! \fn void QAxScript::stateChanged(int state);

    This signal is emitted when a script engine changes state.
    \a state can be any value in the ScriptState enumeration.
*/

/*!
    \fn void QAxScript::error(int code, const QString &description, 
				    int sourcePosition, const QString &sourceText)

    This signal is emitted when an execution error occured while running a script.

    \a code, \a description, \a sourcePosition and \a sourceText contain information
    about the execution error.
*/

/*!
    Creates a QAxScript object. \a parent and \a name 
    are propagated to the QObject constructor.

    You usually create one QAxScript for each document in
    your application.
*/
QAxScript::QAxScript(QObject *parent, const char *name)
: QObject( parent, name )
{
    scriptSite = new QAxScriptSite(this);
}

/*!
    Returns a list with all functions available. Functions provided by
    script engines that don't support introspection will not be listed.
*/
QStringList QAxScript::functions() const
{
    QStringList functions;

    QPtrListIterator<QAxScriptEngine> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScriptEngine *script = scriptIt.current();
	++scriptIt;

	functions += script->functions();
    }

    return functions;
}

/*!
    Returns a list with the name of all scripts.
*/
QStringList QAxScript::scriptNames() const
{
    QStringList scripts;

    QPtrListIterator<QAxScriptEngine> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScriptEngine *script = scriptIt.current();
	++scriptIt;

	scripts << script->scriptName();
    }

    return scripts;
}

/*!
    Returns the script engine for \a name.

    You can use the returned pointer to connect to functions as slots, 
    or to call functions directly through dynamicCall().

    \warning The returned object is destroyed when \a name is unloaded.

    \sa unload()
*/
QAxObject *QAxScript::scriptEngine(const QString &name) const
{
    QPtrListIterator<QAxScriptEngine> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScriptEngine *script = scriptIt.current();
	++scriptIt;

	if (script->scriptName() == name)
	    return script;
    }

    return 0;
}

/*!
    Adds \a object to the script manager. Scripts
    handled by this manager can access the object
    in the code using the object's \link QObject::name 
    name \endlink property.

    You need to add all objects before loading any scripts.
*/
void QAxScript::addObject(QAxBase *object)
{
    QObject *obj = object->qObject();
    QString name = QString::fromLatin1(obj->name());
    if (scriptSite->objectDict.find(name))
	return;

    scriptSite->objectDict.insert(name, object);
    connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
}

/*!
    Loads the script source \a code using the script engine for \a language.
    The script can later be referred to useing \a name.

    The function returns a pointer to the script engine if \a code could be 
    loaded successfully, otherwise returns 0.

    If \a language is empty it will be determined heuristically. If \a code
    contains the string "End Sub" it will be interpreted as VBScript, otherwise
    as JScript. Additional script engines can be registered using registerEngine().

    You need to add all objects necessary before loading any scripts. If \a code 
    declares a function that is already available (no matter in which language) 
    the first function is overloaded and can no longer be called via call(), but 
    it will still be available calling the \link scriptEngine() script engine 
    \endlink directly.

    \sa addObject(), scriptNames(), functions()
*/
QAxObject *QAxScript::load(const QString &code, const QString &language, const QString &name)
{
    QString script_language(language);
    if (script_language.isEmpty()) {
	if (code.contains("End Sub", FALSE))
	    script_language = "VBScript";

	QValueList<QAxEngineDescriptor>::ConstIterator it;
	for (it = engines.begin(); it != engines.end(); ++it) {
	    QAxEngineDescriptor engine = *it;
	    if (engine.code.isEmpty())
		continue;

	    if (code.contains(engine.code)) {
		script_language = engine.name;
		break;
	    }
	}
    }
    if (script_language.isEmpty())
	script_language = "JScript";

    QAxScriptEngine *script = new QAxScriptEngine(name, this);
    if (script->load(code, script_language))
	return script;

    delete script;
    return 0;
}

/*!
    \overload

    Loads the source code in \a file.

    The function returns a pointer to the script engine if \a file could 
    be loaded successfully, otherwise returns 0.

    The code can later be referred to useing \a name.

    The script engine used is determined from the file extension. By
    default ".js" files are interpreted as JScript files, and ".vbs"
    and ".dsm" files are interpreted as VBScript. Additional script engines 
    can be registered using registerEngine().
*/
QAxObject *QAxScript::load(const QString &file, const QString &name)
{
    QFile f(file);
    if (!f.open(IO_ReadOnly))
	return FALSE;
    QByteArray data = f.readAll();
    QString contents = QString::fromLocal8Bit(data, data.size());
    f.close();

    if (contents.isEmpty())
	return FALSE;

    QString script_language;
    if (file.endsWith(".js")) {
	script_language = "JScript";
    } else {
	QValueList<QAxEngineDescriptor>::ConstIterator it;
	for (it = engines.begin(); it != engines.end(); ++it) {
	    QAxEngineDescriptor engine = *it;
	    if (engine.extension.isEmpty())
		continue;

	    if (file.endsWith(engine.extension)) {
		script_language = engine.name;
		break;
	    }
	}
    }

    if (script_language.isEmpty())
	script_language = "VBScript";

    QAxScriptEngine *script = new QAxScriptEngine(name, this);
    if (script->load(contents, script_language))
	return script;

    delete script;
    return 0;
}

/*!
    Adds all objects knows to this manager to all scripts handled
    by this manager.
*/
void QAxScript::updateScripts()
{
    QDictIterator<QAxBase> objectIt(scriptSite->objectDict);
    while (objectIt.current()) {
	QString name = objectIt.getKeyString();
	++objectIt;

	QPtrListIterator<QAxScriptEngine> scriptIt(scriptSite->scriptList);
	while (scriptIt.current()) {
	    QAxScriptEngine *script = scriptIt.current();
	    ++scriptIt;

	    script->addItem(name);
	}
    }
}

/*!
    Unloads all scripts with the name \a name. The functions
    in those scripts are no longer available.

    \warning Calling this function deletes the script engine
    responsible for \a name.

    \sa scriptEngine()
*/
void QAxScript::unload(const QString &name)
{
    QPtrListIterator<QAxScriptEngine> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScriptEngine *script = scriptIt.current();
	++scriptIt;

	if (script->scriptName() == name)
	    script->deleteLater();
    }
}

/*!
    Calls \a function passing \a arguments as parameters, and returns
    the result.

    Functions provided by script engines that don't support introspection 
    are not available and need to be called directly using 
    \link QAxBase::dynamicCall() dynamicCall \endlink on the respective 
    \link scriptEngine() script engine \endlink object.

    The call returns when the script execution is finished.
*/
QVariant QAxScript::call(const QString &function, QValueList<QVariant> &arguments)
{
    QAxScriptEngine *s = script(function);
    if (!s)
	return QVariant();

    updateScripts();
    QValueList<QVariant> args(arguments);
    return s->dynamicCall(function.latin1(), args);
}

/*!
    Registers the script engine \a name and returns TRUE if the engine
    could be located on the system, otherwise returns FALSE and does nothing.

    The script engine will be used when loading files with \a extension,
    or when loading source code that contains the string \a code.
*/
bool QAxScript::registerEngine(const QString &name, const QString &extension, const QString &code)
{
    if (name.isEmpty())
	return FALSE;

    CLSID clsid;
    HRESULT hres = CLSIDFromProgID( name.ucs2(), &clsid );
    if (hres != S_OK)
	return FALSE;

    QAxEngineDescriptor engine;
    engine.name = name;
    engine.extension = extension;
    engine.code = code;

    engines.prepend(engine);
    return TRUE;
}

/*!
    Returns a file filter listing all script languages supported.
    This filter string can conveniently be used in calls to QFileDialog.
*/
QString QAxScript::scriptFileFilter()
{
    QString allFiles = "Script Files (*.js *.vbs *.dsm";
    QString specialFiles = ";;VBScript Files (*.vbs *.dsm)"
			   ";;JavaScript Files (*.js)";

    QValueList<QAxEngineDescriptor>::ConstIterator it;
    for (it = engines.begin(); it != engines.end(); ++it) {
	QAxEngineDescriptor engine = *it;
	if (engine.extension.isEmpty())
	    continue;

	allFiles += " *" + engine.extension;
	specialFiles += ";;" + engine.name + " Files (*" + engine.extension + ")";
    }
    allFiles += ")";

    return allFiles + specialFiles + ";;All Files (*.*)";
}

/*!
    \internal

    Returns a pointer to the first QAxScriptEngine that knows 
    about \a function, or 0 if this function is unknown.
*/
QAxScriptEngine *QAxScript::script(const QString &function) const
{
    QPtrListIterator<QAxScriptEngine> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScriptEngine *script = scriptIt.current();
	++scriptIt;

	if (script->functions().contains(function))
	    return script;
    }

    return 0;
}

/*!
    \internal
*/
void QAxScript::updateScript(QAxScriptEngine *script)
{
    QDictIterator<QAxBase> objectIt(scriptSite->objectDict);
    while (objectIt.current()) {
	QString name = objectIt.getKeyString();
	++objectIt;

	script->addItem(name);
    }
}

/*!
    \internal
*/
void QAxScript::objectDestroyed(QObject *o)
{
    QString name = QString::fromLatin1(o->name());
    scriptSite->objectDict.take(name);
}

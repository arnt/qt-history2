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

    QDict<QAxScriptEngine> scriptDict;
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


/*!
    \class QAxScriptEngine
    \brief The QAxScriptEngine class provides a wrapper around script code.
    \module QAxContainer
    \extension ActiveQt

    Every instance of the QAxScriptEngine class represents a piece of
    scripting code in a particular scripting language. The class is
    not usually used directly. The QAxScript class provides convenient
    functions to handle and call script code.

    The functions declared in the code can be called using
    dynamicCall(). If the script engine supports introspection it can
    be queried for the list of functions in the code using
    functions().

    Direct access to the script engine is provided through
    queryInterface().
*/

/*!
    Constructs a QAxScriptEngine object called \a name and registers
    it with the QAxScript \a manager. This is usually done by the
    QAxScript class when \link QAxScript::load() loading a script.
    \endlink

    Instances of QAxScriptEngine should always have both a name and a
    manager.
*/
QAxScriptEngine::QAxScriptEngine(const QString &name, QAxScript *manager )
: QAxObject(manager, name.latin1()), script_manager(manager), 
  script(0), script_name(name)
{
#ifdef QT_CHECK_STATE
    if (name.isEmpty())
	qWarning("QAxScriptEngine without name created! "
		 "Script engines should have a name.");

    if (!manager)
	qWarning("QAxScriptEngine without manager created! "
		 "Script engines should have a manager.");
#endif
    disableClassInfo();
    disableEventSink();

    if (manager)
	manager->scriptSite->scriptDict.insert(name, this);
}

/*!
    Destroys the QAxScriptEngine object, releasing all allocated
    resources and unregistering the script from its manager.
*/
QAxScriptEngine::~QAxScriptEngine()
{
    if (script_manager)
	script_manager->scriptSite->scriptDict.remove(script_name);

    if (script) {
	script->SetScriptState( SCRIPTSTATE_DISCONNECTED );
	script->Close();
	script->Release();
    }
}

/*! \fn QString QAxScriptEngine::scriptName() const
    Returns the name of the script.

    \sa QAxScript::load()
*/

/*! \fn QString QAxScriptEngine::scriptCode() const
    Returns the script's code.

    \sa QAxScript::load()
*/

/*! \fn QString QAxScriptEngine::scriptLanguage() const
    Returns the name of the script's language, for example "VBScript",
    or "JScript".

    \sa QAxScript::load()
*/

/*!
    Loads the script source \a code written in language \a language
    into the script engine. Returns TRUE if \a code was successfully
    entered into the script engine; otherwise returns FALSE.

    If \a language is empty it will be determined heuristically. If \a
    code contains the string "End Sub" it will be interpreted as
    VBScript, otherwise as JScript. Additional scripting languages can
    be registered using QAxScript::registerEngine().

    This function can only be called once for each QAxScriptEngine object.
*/
bool QAxScriptEngine::load(const QString &code, const QString &language)
{
    if (script)
	return FALSE;

    script_language = language;
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

    script_code += code;

    metaObject();
    return isValid();
}

/*!
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

/*!
    \fn bool QAxScriptEngine::isValid() const

    Returns TRUE if the script engine has been initialized
    correctly; otherwise returns FALSE.
*/

/*!
    Returns TRUE if the script engine supports introspection;
    otherwise returns FALSE.
*/
bool QAxScriptEngine::hasIntrospection() const
{
    if (!isValid())
	return FALSE;

    IDispatch *scriptDispatch = 0;
    QAxBase::queryInterface(IID_IDispatch, (void**)&scriptDispatch);
    if (!scriptDispatch)
	return FALSE;

    UINT tic = 0;
    HRESULT hres = scriptDispatch->GetTypeInfoCount(&tic);
    scriptDispatch->Release();
    return hres == S_OK && tic > 0;
}

/*!
    Returns a list of all the functions this script engine can run if
    it supports introspection; otherwise returns an empty list.
    The functions are either provided with full prototypes or only as 
    names, depending on the value of \a flags.

    \sa hasIntrospection()
*/
QStringList QAxScriptEngine::functions(QAxScript::FunctionFlags flags) const
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

	if (flags == QAxScript::FunctionPrototypes)
	    functions << slotname;
	else
	    functions << slot->method->name;
    }

    return functions;
}

/*!
    Requests the interface \a uuid from the script engine object and
    sets the value of \a iface to the provided interface, or to 0 if
    the requested interface could not be provided.

    Returns the result of the QueryInterface implementation of the COM
    object.
*/
long QAxScriptEngine::queryInterface( const QUuid &uuid, void **iface ) const
{
    *iface = 0;
    if (!script)
	return E_NOTIMPL;

    return script->QueryInterface(uuid, iface);
}

/*!
    \class QAxScript qaxscript.h
    \brief The QAxScript class provides an interface to the Windows Script Host.
    \module QAxContainer
    \extension ActiveQt

    The QAxScript acts as a bridge between the COM objects embedded in
    the Qt application through QAxObject or QAxWidget, and the
    scripting languages available through the Windows Script
    technologies, usually JScript and VBScript.

    Create one QAxScript for each separate document in your
    application, and add the COM objects the script need to access
    using addObject(). Then load() the script sources and invoke the
    functions using call().

    Every script can have a name which can later be used to unload or
    access the script engine created for each script. The script
    engines provide feedback to the application through signals. The
    most important signal is the error() signal.
*/

/*!
    \enum QAxScript::ScriptState

    The ScriptState enumeration defines the different states a script
    engine can be in.

    \value Uninitialized The script has been created, but not yet initialized
    \value Initialized The script has been initialized, but is not running
    \value Started The script can execute code, but does not yet handle events
    \value Connected The script can execute code and is connected so
    that it can handle events
    \value Disconnected The script is loaded, but is not connected to
    event sources
    \value Closed The script has been closed.
*/

/*! \fn void QAxScript::entered()

    This signal is emitted when a script engine has started executing code.
*/

/*! \fn void QAxScript::finished()

    This signal is emitted when a script engine has finished executing code.
*/

/*! \overload void QAxScript::finished(const QVariant &result)

    \a result contains the script's result. This will be an invalid
    QVariant if the script has no return value.
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

    This signal is emitted when an execution error occured while
    running a script.

    \a code, \a description, \a sourcePosition and \a sourceText
    contain information about the execution error.
*/

/*!
    Creates a QAxScript object. \a parent and \a name are passed on
    to the QObject constructor.

    It is usual to create one QAxScript for each document in an
    application.
*/
QAxScript::QAxScript(QObject *parent, const char *name)
: QObject( parent, name )
{
    scriptSite = new QAxScriptSite(this);
}

/*!
    Returns a list with all the functions that are available.
    Functions provided by script engines that don't support
    introspection are not included in the list.
    The functions are either provided with full prototypes or 
    only as names, depending on the value of \a flags.
*/
QStringList QAxScript::functions(FunctionFlags flags) const
{
    QStringList functions;

    QDictIterator<QAxScriptEngine> scriptIt(scriptSite->scriptDict);
    while (scriptIt.current()) {
	QAxScriptEngine *script = scriptIt.current();
	++scriptIt;

	functions += script->functions(flags);
    }

    return functions;
}

/*!
    Returns a list with the names of all the scripts.
*/
QStringList QAxScript::scriptNames() const
{
    QStringList scripts;

    QDictIterator<QAxScriptEngine> scriptIt(scriptSite->scriptDict);
    while (scriptIt.current()) {
	scripts << scriptIt.getKeyString();
	++scriptIt;
    }

    return scripts;
}

/*!
    Returns the script engine for the script called \a name.

    You can use the returned pointer to connect to functions as slots,
    to call functions directly through dynamicCall(), or to delete and
    thus unload the script.
*/
QAxScriptEngine *QAxScript::scriptEngine(const QString &name) const
{
    return scriptSite->scriptDict.find(name);
}

/*!
    Adds \a object to the QAxScript. Scripts handled by this
    QAxScript can access the object in the code using the object's \link
    QObject::name name \endlink property.

    You must add all the necessary objects before loading any scripts.
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
    Loads the script source \a code using the script engine for \a
    language. The script can later be referred to using its \a name
    which should not be empty.

    The function returns a pointer to the script engine for the given
    \a code if the \a code was loaded successfully; otherwise it
    returns 0.

    If \a language is empty it will be determined heuristically. If \a
    code contains the string "End Sub" it will be interpreted as
    VBScript, otherwise as JScript. Additional script engines can be
    registered using registerEngine().

    You must add all the objects necessary (using addObject()) \e
    before loading any scripts. If \a code declares a function that is
    already available (no matter in which language) the first function
    is overloaded and can no longer be called via call(); but it will
    still be available by calling its \link scriptEngine() script
    engine \endlink directly.

    \sa addObject(), scriptNames(), functions()
*/
QAxScriptEngine *QAxScript::load(const QString &code, const QString &name, const QString &language)
{
    QAxScriptEngine *script = new QAxScriptEngine(name, this);
    if (script->load(code, language))
	return script;

    delete script;
    return 0;
}

/*!
    \overload

    Loads the source code from the \a file. The script can later be
    referred to using its \a name which should not be empty.

    The function returns a pointer to the script engine for the code
    in \a file if \a file was loaded successfully; otherwise it
    returns 0.

    The script engine used is determined from the file's extension. By
    default ".js" files are interpreted as JScript files, and ".vbs"
    and ".dsm" files are interpreted as VBScript. Additional script
    engines can be registered using registerEngine().
*/
QAxScriptEngine *QAxScript::load(const QString &file, const QString &name)
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
    Calls \a function passing \a arguments as parameters, and returns
    the result. Returns when the script's execution has finished.

    In most script engines the only supported parameter type is "const
    QVariant&", for example, to call a JavaScript function
    \code
    function setNumber(number)
    {
        n = number;
    }
    \endcode
    use
    \code
    QValueList args;
    args << 5;
    script->call("setNumber(const QVariant&)", args);
    \endcode
    As with \link QAxBase::dynamicCall() dynamicCall \endlink the 
    parameters can directly be embedded in the function string.
    \code
    script->call("setNumber(5)");
    \endcode
    However, this is slower.

    Functions provided by script engines that don't support
    introspection are not available and must be called directly
    using \link QAxBase::dynamicCall() dynamicCall \endlink on the
    respective \link scriptEngine() script engine \endlink object.

    Note that calling this function can be significantely slower than
    calling the script engine directly.
*/
QVariant QAxScript::call(const QString &function, QValueList<QVariant> &arguments)
{
    QAxScriptEngine *s = script(function);
    if (!s) {
#ifdef QT_CHECK_STATE
	qWarning("QAxScript::call(%s): No script provides this function, or the function\n"
	         "\tis provided through an engine that does not support introspection.", function.latin1());
#endif
	return QVariant();
    }

    QValueList<QVariant> args(arguments);
    return s->dynamicCall(function.latin1(), args);
}

/*!
    Registers the script engine called \a name and returns TRUE if the
    engine was found; otherwise does nothing and returns FALSE.

    The script engine will be used when loading files with the given
    \a extension, or when loading source code that contains the string
    \a code.
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
    Returns a file filter listing all the supported script languages.
    This filter string is convenient for use with QFileDialog.
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
    // check full prototypes if included
    if (function.contains('(')) {
	QDictIterator<QAxScriptEngine> scriptIt(scriptSite->scriptDict);
	while (scriptIt.current()) {
	    QAxScriptEngine *script = scriptIt.current();
	    ++scriptIt;

	    if (script->functions(FunctionPrototypes).contains(function))
		return script;
	}
    }

    QString funcName = function;
    funcName = funcName.left(funcName.find('('));
    // second try, checking only names, not prototypes
    QDictIterator<QAxScriptEngine> scriptIt(scriptSite->scriptDict);
    while (scriptIt.current()) {
	QAxScriptEngine *script = scriptIt.current();
	++scriptIt;

	if (script->functions(FunctionNames).contains(funcName))
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

	if (script->script)
	    script->script->AddNamedItem(name.ucs2(), SCRIPTITEM_ISSOURCE|SCRIPTITEM_ISVISIBLE);
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

#include "qaxscript.h"

#include <qaxobject.h>
#include <qdict.h>
#include <qfile.h>
#include <qmetaobject.h>
#include <qwidget.h>

#include <qt_windows.h>
#include <activscp.h>
#include "..\shared\types.h"

// Template specialization, as ~QAxScript is private
template <> void QPtrList<QAxScript>::deleteItem(Item)
{
}

/*!
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
    QAxScriptSite(QAxScriptManager *manager);

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

    QPtrList<QAxScript> scriptList;
    QDict<QAxBase> objectDict;

protected:
    QWidget *topLevelWidget() const;

private:
    QAxScriptManager *scriptManager;
    unsigned long ref;
};

/*!
    \class QAxScript qaxscript.h
    \brief The QAxScript class provides a wrapper around scripts.
    \internal
*/
class QAxScript : public QAxObject
{
public:
    QAxScript(const QString &name, QAxScriptManager *manager);
    ~QAxScript();

    bool isValid() const;
    QStringList functions() const;

    bool initialize(IUnknown** ptr);
    void addItem(const QString &name);
    bool load(const QString &code, const QString &language);

private:
    QAxScriptManager *script_manager;
    IActiveScript *script;

    QString script_code;
    QString script_language;
    QString script_name;
};


/*!
    Constructs the site for the \a manager.
*/
QAxScriptSite::QAxScriptSite(QAxScriptManager *manager)
: scriptManager(manager), ref(1)
{
}

/*!
    Implements IUnknown::AddRef
*/
ULONG WINAPI QAxScriptSite::AddRef()
{
    return ++ref;
}

/*!
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

/*!
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

/*!
    Implements IActiveScriptSite::GetLCID

    This method is not implemented. Use the system-defined locale.
*/
HRESULT WINAPI QAxScriptSite::GetLCID(LCID * /*plcid*/)
{
    return E_NOTIMPL;
}

/*!
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

/*!
    Implements IActiveScriptSite::GetDocVersionString

    This method is not implemented. The scripting engine should assume 
    that the script is in sync with the document.
*/
HRESULT WINAPI QAxScriptSite::GetDocVersionString(BSTR * /*version*/)
{
    return E_NOTIMPL;
}

/*!
    Implements IActiveScriptSite::OnScriptTerminate

    This method is usually not called, but if it is it fires
    QAxScriptManager::scriptFinished().
*/
HRESULT WINAPI QAxScriptSite::OnScriptTerminate(const VARIANT *result, const EXCEPINFO *exception)
{
    emit scriptManager->scriptFinished();

    if (result && result->vt != VT_EMPTY)
	emit scriptManager->scriptFinished(VARIANTToQVariant(*result, 0));
    if (exception)
	emit scriptManager->scriptFinished(exception->wCode, 
					   BSTRToQString(exception->bstrSource),
					   BSTRToQString(exception->bstrDescription),
					   BSTRToQString(exception->bstrHelpFile)
					  );
    return S_OK;
}

/*!
    Implements IActiveScriptSite::OnEnterScript

    Fires QAxScriptManager::scriptEntered() to inform the host that the 
    scripting engine has begun executing the script code.
*/
HRESULT WINAPI QAxScriptSite::OnEnterScript()
{
    emit scriptManager->scriptEntered();
    return S_OK;
}

/*!
    Implements IActiveScriptSite::OnLeaveScript

    Fires QAxScriptManager::scriptFinished() to inform the host that the 
    scripting engine has returned from executing the script code.
*/
HRESULT WINAPI QAxScriptSite::OnLeaveScript()
{
    emit scriptManager->scriptFinished();
    return S_OK;
}

/*!
    Implements IActiveScriptSite::OnScriptError

    Fires QAxScriptManager::scriptError() to inform the host that an 
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
	lineText = ((QAxScript*)context)->name();

    emit scriptManager->scriptError(exception.wCode, BSTRToQString(exception.bstrDescription),
				    lineNumber, lineText);
    return S_OK;
}

/*!
    Implements IActiveScriptSite::OnStateChange

    Fires QAxScriptManager::scriptStateChanged() to inform the
    the host that the scripting engine has changed states.
*/
HRESULT WINAPI QAxScriptSite::OnStateChange(SCRIPTSTATE ssScriptState)
{
    emit scriptManager->scriptStateChanged(ssScriptState);
    return S_OK;
}

/*!
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

/*!
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

/*!
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
    Constructs a QAxScript object with name \a name and registers
    it in the QAxScriptManager \a manager.

    \a name should not be empty.
*/
QAxScript::QAxScript(const QString &name, QAxScriptManager *manager )
: QAxObject(0, manager, name.latin1()), script_manager(manager), 
  script(0), script_name(name)
{
    disableClassInfo();
    disableEventSink();

    if (manager)
	manager->scriptSite->scriptList.prepend(this);
}

/*!
    Destroys the QAxScript object, releasing all allocated
    resources and unregistering the script from its manager.
*/
QAxScript::~QAxScript()
{
    if (script_manager)
	script_manager->scriptSite->scriptList.removeRef(this);

    if (script) {
	script->SetScriptState( SCRIPTSTATE_DISCONNECTED );
	script->Close();
	script->Release();
    }
}

/*!
    Loads the script source \a code into the script, using the script
    engine for \a language. The function returns TRUE if \a code could
    be passed successfully into the script engine, otherwise returns
    FALSE.

    If \a language is empty it will be determined heuristically based 
    on \a code:
    \list
    \i If the code includes the substring "End Sub" \a code is interpreted
    as VBScript
    \i Otherwise the code is interpreted as JScript
    \endlist
*/
bool QAxScript::load(const QString &code, const QString &language)
{
    if (script)
	return FALSE;

    script_language = language;
    if (script_language.isEmpty()) {
	if (code.contains("End Sub", FALSE))
	    script_language = "VBScript";
	else
	    script_language = "JScript";
    }

    script_code += code;

    // This is "some uuid" to trigger the call to initialize()
    setControl("{2ad11b7e-62a9-439c-9724-fb1b686c8457}");
    return isValid();
}

/*!
    \reimp
*/
bool QAxScript::initialize(IUnknown **ptr)
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
    Adds an item \a name to the script engine.
    The item can then be addressed in the script code.

    \sa QAxScriptManager::addObject()
*/
void QAxScript::addItem(const QString &name)
{
    if (!script)
	return;

    script->AddNamedItem(name.ucs2(), SCRIPTITEM_ISSOURCE|SCRIPTITEM_ISVISIBLE);
}

/*!
    Returns TRUE if the script engine has been initialized
    correctly, otherwise returns FALSE.
*/
bool QAxScript::isValid() const
{
    return script != 0;
}

/*!
    Returns a list of all functions this script can run.
*/
QStringList QAxScript::functions() const
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
    \class QAxScriptManager qaxscript.h
    \brief The QAxScriptManager class implements ...###
*/

/*!
    Creates a QAxScriptManager object. \a parent and \a name 
    are propagated to the QObject constructor.
*/
QAxScriptManager::QAxScriptManager(QObject *parent, const char *name)
: QObject( parent, name )
{
    scriptSite = new QAxScriptSite(this);
}

/*!
    Returns a list of all functions known to this manager.

    This is equivalent of calling QAxScript::functions() on
    all QAxScript objects returns by scripts()
*/
QStringList QAxScriptManager::functions() const
{
    QStringList functions;

    QPtrListIterator<QAxScript> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScript *script = scriptIt.current();
	++scriptIt;

	functions += script->functions();
    }

    return functions;
}

/*!
    Adds \a object to the script manager. Scripts
    handled by this manager can access the object
    in the code using the object's \link QObject::name 
    name \endlink property.

    You need to add all objects before loading any scripts.

    \sa QAxScript::addItem()
*/
void QAxScriptManager::addObject(QAxBase *object)
{
    QObject *obj = object->qObject();
    QString name = QString::fromLatin1(obj->name());
    if (scriptSite->objectDict.find(name))
	return;

    scriptSite->objectDict.insert(name, object);
    connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
}

/*!
    Loads the source code in \a file and returns TRUE if successful,
    or FALSE if the code could not be loaded into any script engine.

    The script can later be referred to useing \a name.

    The script engine used is determined from the extension of \a file:
    \list
    \i If the extension is ".js" the contents are interpreted as JScript
    \i Otherwise the contents are interpreted as VBScript
    \endlist

    You need to add all objects necessary before loading any 
    scripts.

    \sa addObject()
*/
bool QAxScriptManager::load(const QString &file, const QString &name)
{
    QFile f(file);
    if (!f.open(IO_ReadOnly))
	return FALSE;
    QByteArray data = f.readAll();
    QString contents = QString::fromLocal8Bit(data, data.size());
    f.close();

    if (contents.isEmpty())
	return FALSE;

    QString language;
    if (file.endsWith(".js"))
	language = "JScript";
    else
	language = "VBScript";

    QAxScript *script = new QAxScript(name, this);
    if (script->load(contents, language))
	return TRUE;

    delete script;
    return 0;
}

/*!
    Loads the script source \a code using the script engine for \a language.
    The script can later be referred to useing \a name.

    The function returns TRUE if \a code could be passed successfully 
    into the script engine, otherwise returns FALSE.

    If \a language is empty it will be determined heuristically based 
    on \a code:
    \list
    \i If the code includes the substring "End Sub" \a code is interpreted
    as VBScript
    \i Otherwise the code is interpreted as JScript
    \endlist
*/
bool QAxScriptManager::load(const QString &code, const QString &language, const QString &name)
{
    QAxScript *script = new QAxScript(name, this);
    if (script->load(code, language))
	return TRUE;

    delete script;
    return FALSE;
}

/*!
    Adds all objects knows to this manager to all scripts handled
    by this manager.

    Reimplementations of this functions should call this 
    implemenation.

    \sa addObject(), QAxScript::addItem()
*/
void QAxScriptManager::updateScripts()
{
    QDictIterator<QAxBase> objectIt(scriptSite->objectDict);
    while (objectIt.current()) {
	QString name = objectIt.getKeyString();
	++objectIt;

	QPtrListIterator<QAxScript> scriptIt(scriptSite->scriptList);
	while (scriptIt.current()) {
	    QAxScript *script = scriptIt.current();
	    ++scriptIt;

	    script->addItem(name);
	}
    }
}

/*!
    Unloads all scripts with the name \a name. The functions
    in those scripts are no longer available.
*/
void QAxScriptManager::unload(const QString &name)
{
    QPtrListIterator<QAxScript> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScript *script = scriptIt.current();
	++scriptIt;

	if (script->name() == name)
	    script->deleteLater();
    }
}

/*!
    \internal
*/
void QAxScriptManager::updateScript(QAxScript *script)
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

    Returns a pointer to the first QAxScript object that knows 
    about \a function, or 0 if this function is unknown.
*/
QAxScript *QAxScriptManager::script(const QString &function) const
{
    QPtrListIterator<QAxScript> scriptIt(scriptSite->scriptList);
    while (scriptIt.current()) {
	QAxScript *script = scriptIt.current();
	++scriptIt;

	if (script->functions().contains(function))
	    return script;
    }

    return 0;
}

/*!
    Calls the first script knowing about \a function and passes
    \a arguments as parameters. The result of calling the function
    is passed back in the return value.
*/
QVariant QAxScriptManager::call(const QString &function, QValueList<QVariant> &arguments)
{
    QAxScript *s = script(function);
    if (!s)
	return QVariant();

    updateScripts();
    QValueList<QVariant> args(arguments);
    return s->dynamicCall(function.latin1(), args);
}

/*!
    \internal
*/
void QAxScriptManager::objectDestroyed(QObject *o)
{
    QString name = QString::fromLatin1(o->name());
    scriptSite->objectDict.take(name);
}

/****************************************************************************
**
** Implementation of the QAxBase class.
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//#define QAX_NO_CLASSINFO
#define QT_NO_CAST_TO_ASCII
//#define QT_NO_CAST_FROM_ASCII
#define QT_CHECK_STATE

#include "qaxobject.h"

#include <quuid.h>
#include <qhash.h>
#include <qpair.h>
#include <qmetaobject.h>
#include <qsettings.h>

#ifdef QT_THREAD_SUPPORT
#   include <qmutex.h>
#endif

#include <qt_windows.h>
#include <ocidl.h>
#include <ctype.h>

#include "../shared/types.h"


/*
    \internal
    \class QAxMetaObject qaxbase.cpp

    \brief The QAxMetaObject class stores extended information
*/
struct QAxMetaObject : public QMetaObject
{
    ~QAxMetaObject()
    {
        delete [] (int*)d.data;
        delete [] (char*)d.stringdata;
    }
    // save information about QAxEventSink connections, and connect when found in cache
    QList<QUuid> connectionInterfaces;
    // DISPID -> signal name
    QMap< QUuid, QMap<DISPID, QByteArray> > sigs;
    // DISPID -> property changed signal name
    QMap< QUuid, QMap<DISPID, QByteArray> > propsigs;
    // DISPID -> property name
    QMap< QUuid, QMap<DISPID, QByteArray> > props;
    
    struct Member
    {
        int numParameter;
    };
    // Prototype -> member info
    QHash<QByteArray, Member> memberInfo;
    
    int numParameter(const QByteArray &prototype);
    QByteArray paramType(const QByteArray &signature, int index, bool *out = 0);
    
    QMap<QByteArray, QByteArray> realPrototype;
};

static QHash<QString, QAxMetaObject*> mo_cache;
static int mo_cache_ref = 0;
static QMutex cache_mutex;

/*
    \internal
    \class QAxEventSink qaxbase.cpp

    \brief The QAxEventSink class implements the event sink for all
	   IConnectionPoints implemented in the COM object.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif
*/

class QAxEventSink : public IDispatch, public IPropertyNotifySink
{
public:
    QAxEventSink(QAxBase *com)
        : cpoint(0), ciid(IID_NULL), combase(com), ref(1)
    {}
    virtual ~QAxEventSink()
    {
        Q_ASSERT(!cpoint);
    }
    
    QUuid connectionInterface() const
    {
        return ciid;
    }
    QMap<DISPID, QByteArray> signalMap() const
    {
        return sigs;
    }
    QMap<DISPID, QByteArray> propertyMap() const
    {
        return props;
    }
    QMap<DISPID, QByteArray> propSignalMap() const
    {
        return propsigs;
    }
    
    // add a connection
    void advise(IConnectionPoint *cp, IID iid)
    {
        cpoint = cp;
        cpoint->AddRef();
        ciid = iid;
        cpoint->Advise((IUnknown*)(IDispatch*)this, &cookie);
    }
    
    // disconnect from all connection points
    void unadvise()
    {
        combase = 0;
        if (cpoint) {
            cpoint->Unadvise(cookie);
            cpoint->Release();
            cpoint = 0;
        }
    }
    
    void addSignal(DISPID memid, const char *name)
    {
        sigs.insert(memid, name);
        QMap<DISPID,QByteArray>::ConstIterator it;
        DISPID id = -1;
        for (it = propsigs.constBegin(); it!= propsigs.constEnd(); ++it) {
            if (it.value() == name) {
                id = it.key();
                break;
            }
        }
        if (id != -1)
            propsigs.remove(id);
    }
    void addProperty(DISPID propid, const char *name, const char *signal)
    {
        props.insert(propid, name);
        propsigs.insert(propid, signal);
    }
    
    // IUnknown
    unsigned long __stdcall AddRef()
    {
        return ref++;
    }
    unsigned long __stdcall Release()
    {
        if (!--ref) {
            delete this;
            return 0;
        }
        return ref;
    }
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject)
    {
        *ppvObject = 0;
        if (riid == IID_IUnknown)
            *ppvObject = (IUnknown*)(IDispatch*)this;
        else if (riid == IID_IDispatch)
            *ppvObject = (IDispatch*)this;
        else if (riid == IID_IPropertyNotifySink)
            *ppvObject = (IPropertyNotifySink*)this;
        else if (ciid == riid)
            *ppvObject = (IDispatch*)this;
        else
            return E_NOINTERFACE;
        
        AddRef();
        return S_OK;
    }
    
    // IDispatch
    HRESULT __stdcall GetTypeInfoCount(unsigned int *) { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo(UINT, LCID, ITypeInfo **) { return E_NOTIMPL; }
    HRESULT __stdcall GetIDsOfNames(const _GUID &, wchar_t **, unsigned int, unsigned long, long *) { return E_NOTIMPL; }
    
    HRESULT __stdcall Invoke(DISPID dispIdMember,
                            REFIID riid,
                            LCID,
                            WORD wFlags,
                            DISPPARAMS *pDispParams,
                            VARIANT*,
                            EXCEPINFO*,
                            UINT*)
    {
        // verify input
        if (riid != IID_NULL)
            return DISP_E_UNKNOWNINTERFACE;
        if (!(wFlags & DISPATCH_METHOD))
            return DISP_E_MEMBERNOTFOUND;
        if (!combase)
            return E_UNEXPECTED;

        QAxMetaObject *meta = const_cast<QAxMetaObject*>(static_cast<const QAxMetaObject*>(combase->metaObject()));
        QByteArray signame = sigs.value(dispIdMember);
        if (!meta || signame.isEmpty())
            return DISP_E_MEMBERNOTFOUND;

        QObject *qobject = combase->qObject();
        if (qobject->signalsBlocked())
            return S_OK;

        int index = -1;
        // emit the generic signal "as is"
        if (((QAxObject*)qobject)->receivers(SIGNAL(signal(QString,int,void*))))
            index = meta->indexOfSignal("signal(QString,int,void*)");

        if (index != -1) {
            QString nameString(signame);
            void *argv[] = {0, &nameString, &pDispParams->cArgs, &pDispParams->rgvarg};
            combase->qt_metacall(QMetaObject::EmitSignal, index, argv);
        }

        HRESULT hres = S_OK;

        // get the signal information from the metaobject
        index = -1;
        if (((QAxObject*)qobject)->receivers(QByteArray::number(QSIGNAL_CODE) + signame))
            index = meta->indexOfSignal(signame);
        if (index != -1) {
            const QMetaMember signal = meta->signal(index);
            Q_ASSERT(signame == signal.signature());
            // verify parameter count
            int pcount = meta->numParameter(signame);
            int argcount = pDispParams->cArgs;
            if (pcount > argcount)
                return DISP_E_PARAMNOTOPTIONAL;
            else if (pcount < argcount)
                return DISP_E_BADPARAMCOUNT;
            
            // setup parameters (no return values in signals)
            bool ok = true;
            void *static_argv[QAX_NUM_PARAMS + 1];
            void *static_argv_pointer[QAX_NUM_PARAMS + 1];
            QVariant static_varp[QAX_NUM_PARAMS + 1];
            
            void **argv = 0;
            void **argv_pointer = 0; // in case we need an additional level of indirection
            QVariant *varp = 0;
            
            if (pcount) {
                if (pcount <= QAX_NUM_PARAMS) {
                    argv = static_argv;
                    argv_pointer = static_argv_pointer;
                    varp = static_varp;
                } else {
                    argv = new void*[pcount + 1];
                    argv_pointer = new void*[pcount + 1];
                    varp = new QVariant[pcount + 1];
                }

                argv[0] = 0;
                argv_pointer[0] = 0;
            }

            int p;
            for (p = 0; p < pcount && ok; ++p) {
                // map the VARIANT to the void*
                QByteArray ptype = meta->paramType(signame, p);
                varp[p + 1] = VARIANTToQVariant(pDispParams->rgvarg[pcount - p - 1], ptype);
                argv_pointer[p + 1] = 0;
                if (varp[p + 1].isValid()) {
                    if (varp[p + 1].type() == QVariant::UserType) {
                        argv[p + 1] = varp[p + 1].data();
                    } else if (ptype == "QVariant") {
                        argv[p + 1] = varp + p + 1;
                    } else {
                        argv[p + 1] = const_cast<void*>(varp[p + 1].constData());
                        if (ptype.endsWith("*")) {
                            argv_pointer[p + 1] = argv[p + 1];
                            argv[p + 1] = argv_pointer + p + 1;
                        }
                    }
                } else if (ptype == "QVariant") {
                    argv[p + 1] = varp + p + 1;
                } else {
                    ok = false;
                }
            }

            if (ok) {
                // emit the generated signal if everything went well
                combase->qt_metacall(QMetaObject::EmitSignal, index, argv);
                // update the VARIANT for references and free memory
                for (p = 0; p < pcount; ++p) {
                    bool out;
                    QByteArray ptype = meta->paramType(signame, p, &out);
                    if (out) {
                        if (!QVariantToVARIANT(varp[p + 1], pDispParams->rgvarg[pcount - p - 1], ptype, out))
                            ok = false;
                    }
                }
            }

            if (argv != static_argv) {
                delete [] argv;
                delete [] argv_pointer;
                delete [] varp;
            }
            hres = ok ? S_OK : (ok ? DISP_E_MEMBERNOTFOUND : DISP_E_TYPEMISMATCH);
        }

        return hres;
    }
    
    // IPropertyNotifySink
    HRESULT __stdcall OnChanged(DISPID dispID)
    {
        // verify input
        if (dispID == DISPID_UNKNOWN || !combase)
            return S_OK;
        
        const QMetaObject *meta = combase->metaObject();
        QByteArray propname = props.value(dispID);
        if (!meta || propname.isEmpty())
            return S_OK;
        
        QObject *qobject = combase->qObject();
        if (qobject->signalsBlocked())
            return S_OK;
        
        // emit the generic signal
        int index = meta->indexOfSignal("propertyChanged(QString)");
        if (index != -1) {
            QString propnameString(propname);
            void *argv[] = {0, &propnameString};
            combase->qt_metacall(QMetaObject::EmitSignal, index, argv);
        }
        
        QByteArray signame = propsigs.value(dispID);
        if (signame.isEmpty())
            return S_OK;
        
        // get the signal information from the metaobject
        index = meta->indexOfSignal(signame);
        if (index != -1 && ((QAxObject*)qobject)->receivers(QByteArray::number(QSIGNAL_CODE) + signame)) {
            // setup parameters
            QVariant var = qobject->property(propname);
            if (!var.isValid())
                return S_OK;
            
            const QMetaProperty metaProp = meta->property(meta->indexOfProperty(propname));
            void *argv[] = {0, var.data()};
            if (metaProp.type() == -1)
                argv[1] = &var;
            
            // emit the "changed" signal
            combase->qt_metacall(QMetaObject::EmitSignal, index, argv);
        }
        return S_OK;
    }
    HRESULT __stdcall OnRequestEdit(DISPID dispID)
    {
        if (dispID == DISPID_UNKNOWN || !combase)
            return S_OK;
        
        QByteArray propname = props.value(dispID);
        return combase->propertyWritable(propname) ? S_OK : S_FALSE;
    }
    
    IConnectionPoint *cpoint;
    IID ciid;
    ULONG cookie;
    
    QMap<DISPID, QByteArray> sigs;
    QMap<DISPID, QByteArray> propsigs;
    QMap<DISPID, QByteArray> props;
    
    QAxBase *combase;
    long ref;
};

/*
    \internal
    \class QAxBasePrivate qaxbase.cpp
*/

class QAxBasePrivate
{
public:
    QAxBasePrivate()
        : useEventSink(true), useMetaObject(true), useClassInfo(true),
        cachedMetaObject(false), initialized(false), tryCache(false),
        ptr(0), disp(0), metaobj(0), staticMetaObject(0)
    {
        // protect initialization
        QMutexLocker locker(&cache_mutex);
        mo_cache_ref++;
        
    }
    
    ~QAxBasePrivate()
    {
        Q_ASSERT(!ptr);
        Q_ASSERT(!disp);
        
        // protect cleanup
        QMutexLocker locker(&cache_mutex);
        if (!--mo_cache_ref) {
            qDeleteAll(mo_cache);
            mo_cache.clear();
        }
    }
    
    IDispatch *dispatch()
    {
        if (disp)
            return disp;
        
        if (ptr)
            ptr->QueryInterface(IID_IDispatch, (void**)&disp);
        return disp;
    }
    
    QHash<QString, QAxEventSink*> eventSink;
    bool useEventSink	    :1;
    bool useMetaObject	    :1;
    bool useClassInfo	    :1;
    bool cachedMetaObject   :1;
    bool initialized	    :1;
    bool tryCache	    :1;
    
    IUnknown *ptr;
    IDispatch *disp;
    
    QMap<QByteArray, bool> propWritable;
    QAxMetaObject *metaobj;
    QMetaObject *staticMetaObject;
};

#ifndef QAXPRIVATE_DECL

int QAxMetaObject::numParameter(const QByteArray &prototype)
{
    if (memberInfo.contains(prototype)) {
        Member member = memberInfo.value(prototype);
        return member.numParameter;
    }
    
    QByteArray parameters = prototype.mid(prototype.indexOf('(') + 1);
    parameters.truncate(parameters.length() - 1);
    int numParameters = 0;
    if (!parameters.isEmpty())
        numParameters = parameters.count(',') + 1;
    Member member = {numParameters};
    memberInfo.insert(prototype, member);
    
    return numParameters;
}

QByteArray QAxMetaObject::paramType(const QByteArray &prototype, int index, bool *out)
{
    if (out)
        *out = false;
   
    QByteArray realProto = realPrototype.value(prototype, prototype);
    QByteArray parameters = realProto.mid(realProto.indexOf('(') + 1);
    parameters.truncate(parameters.length() - 1);
    
    QList<QByteArray> plist = parameters.split(',');
    if (index > plist.count() - 1)
        return "QVariant";
    
    QByteArray param(plist.at(index));
    if (param.isEmpty())
        return "QVariant";
    
    bool byRef = false;
    if (param.endsWith("&")) {
        byRef = true;
        param.truncate(param.length() - 1);
    } else if (param.endsWith("**")) {
        byRef = true;
        param.truncate(param.length() - 1);
    }
    
    if (out)
        *out = byRef;
    
    return param;
}

/*!
    \class QAxBase qaxbase.h

    \brief The QAxBase class is an abstract class that provides an API
    to initalize and access a COM object.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module QAxContainer
    \extension ActiveQt

    QAxBase is an abstract class that cannot be used directly, and is
    instantiated through the subclasses QAxObject and QAxWidget. This
    class provides the API to access the COM object directly
    through its IUnknown implementation. If the COM object implements
    the IDispatch interface, the properties and methods of that object
    become available as Qt properties and slots.

    \code
    connect(buttonBack, SIGNAL(clicked()), webBrowser, SLOT(GoBack()));
    \endcode

    Properties exposed by the object's IDispatch implementation can be
    read and written through the property system provided by the Qt
    Object Model (both subclasses are QObjects, so you can use \link
    QObject::setProperty() setProperty() \endlink and \link
    QObject::property() property() \endlink as with QObject). Properties
    with multiple parameters are not supported.

    \code
    activeX->setProperty("text", "some text");
    int value = activeX->property("value");
    \endcode

    Write-functions for properties and other methods exposed by the
    object's IDispatch implementation can be called directly using
    dynamicCall(), or indirectly as slots connected to a signal.

    \code
    webBrowser->dynamicCall("GoHome()");
    \endcode

    Outgoing events supported by the COM object are emitted as
    standard Qt signals.

    \code
    connect(webBrowser, SIGNAL(TitleChanged(const QString&)),
	     this, SLOT(setCaption(const QString&)));
    \endcode

    QAxBase transparently converts between COM data types and the
    equivalent Qt data types. Some COM types have no equivalent Qt data structure.

    Supported COM datatypes are listed in the first column of following table.
    The second column is the Qt type that can be used with the QObject property
    functions. The third column is the Qt type that is used in the prototype of
    generated signals and slots for in-parameters, and the last column is the Qt
    type that is used in the prototype of signals and slots for out-parameters.
    \table
    \header
    \i COM type
    \i Qt property
    \i in-parameter
    \i out-parameter
    \row
    \i VARIANT_BOOL
    \i bool
    \i bool
    \i bool&
    \row
    \i BSTR
    \i QString
    \i const QString&
    \i QString&
    \row
    \i char, short, int, long
    \i int
    \i int
    \i int&
    \row
    \i uchar, ushort, uint, ulong
    \i uint
    \i uint
    \i uint&
    \row
    \i float, double
    \i double
    \i double
    \i double&
    \row
    \i DATE
    \i QDateTime
    \i const QDateTime&
    \i QDateTime&
    \row
    \i CY
    \i Q_LLONG
    \i Q_LLONG
    \i Q_LLONG&
    \row
    \i OLE_COLOR
    \i QColor
    \i const QColor&
    \i QColor&
    \row
    \i SAFEARRAY(VARIANT)
    \i QList\<QVariant\>
    \i const QList\<QVariant\>&
    \i QList\<QVariant\>&
    \row
    \i SAFEARRAY(BYTE)
    \i QByteArray
    \i const QByteArray&
    \i QByteArray&
    \row
    \i SAFEARRAY(BSTR)
    \i QStringList
    \i const QStringList&
    \i QStringList&
    \row
    \i VARIANT
    \i type-dependent
    \i const QVariant&
    \i QVariant&
    \row
    \i IFontDisp*
    \i QFont
    \i const QFont&
    \i QFont&
    \row
    \i IPictureDisp*
    \i QPixmap
    \i const QPixmap&
    \i QPixmap&
    \row
    \i IDispatch*
    \i QAxObject* (read-only)
    \i \c QAxBase::asVariant()
    \i QAxObject* (return value)
    \row
    \i IUnknown*
    \i QAxObject* (read-only)
    \i \c QAxBase::asVariant()
    \i QAxObject* (return value)
    \row
    \i SCODE, DECIMAL
    \i \e unsupported
    \i \e unsupported
    \i \e unsupported
    \endtable

    Supported are also enumerations, and typedefs to supported types.

    To call the methods of a COM interface described by the following IDL
    \code
    dispinterface IControl
    {
    properties:
        [id(1)] BSTR text;
	[id(2)] IFontDisp *font;

    methods:
	[id(6)] void showColumn([in] int i);
        [id(3)] bool addColumn([in] BSTR t);
	[id(4)] int fillList([in, out] SAFEARRAY(VARIANT) *list);
	[id(5)] IDispatch *item([in] int i);
    };
    \endcode
    use the QAxBase API like this:
    \code
    QAxObject object("<CLSID>");

    QString text = object.property("text").toString();
    object.setProperty("font", QFont("Times New Roman", 12));

    connect(this, SIGNAL(clicked(int)), &object, SLOT(showColumn(int)));
    bool ok = object.dynamicCall("addColumn(const QString&)", "Column 1").toBool();

    QList<QVariant> varlist;
    QList<QVariant> parameters;
    parameters << QVariant(varlist);
    int n = object.dynamicCall("fillList(QList<QVariant>&)", parameters).toInt();

    QAxObject *item = object.querySubItem("item(int)", 5);
    \endcode

    Note that the QList the object should fill has to be provided as an
    element in the parameter list of QVariants.

    If you need to access properties or pass parameters of unsupported
    datatypes you must access the COM object directly through its
    IDispatch implementation or other interfaces. Those interfaces can be 
    retrieved through queryInterface().

    \code
    IUnknown *iface = 0;
    activeX->queryInterface(IID_IUnknown, (void**)&iface);
    if (iface) {
        // use the interface
	iface->Release();
    }
    \endcode

    To get the definition of the COM interfaces you will have to use the header
    files provided with the component you want to use. Some compilers can also
    import type libraries using the #import compiler directive. See the component
    documentation to find out which type libraries you have to import, and how to use
    them.

    If you need to react to events that pass parameters of unsupported
    datatypes you can use the generic signal that delivers the event
    data as provided by the COM event.
*/

/*!
    \enum QAxBase::PropertyBag

    A QMap<QString,QVariant> that can store properties as name:value pairs.
*/

/*!
    Creates a QAxBase object that wraps the COM object \a iface. If \a
    iface is 0 (the default), use setControl() to instantiate a COM
    object.
*/
QAxBase::QAxBase(IUnknown *iface)
{
    d = new QAxBasePrivate();
    d->ptr = iface;
    if (d->ptr) {
        d->ptr->AddRef();
        d->initialized = true;
    }
}

/*!
    Shuts down the COM object and destroys the QAxBase object.

    \sa clear()
*/
QAxBase::~QAxBase()
{
    clear();
    
    delete d;
    d = 0;
}

/*!
    \property QAxBase::control
    \brief the name of the COM object wrapped by this QAxBase object.

    Setting this property initilializes the COM object. Any COM object
    previously set is shut down.

    The most efficient way to set this property is by using the
    registered component's UUID, e.g.
    \code
    ctrl->setControl("{8E27C92B-1264-101C-8A2F-040224009C02}");
    \endcode
    The second fastest way is to use the registered control's class
    name (with or without version number), e.g.
    \code
    ctrl->setControl("MSCal.Calendar");
    \endcode
    The slowest, but easiest way to use is to use the control's full
    name, e.g.
    \code
    ctrl->setControl("Calendar Control 9.0");
    \endcode

    If the component's UUID is used the following patterns can be used
    to initialize the control on a remote machine, to initialize a
    licensed control or to connect to a running object:
    \list
    \i To initialize the control on a different machine use the following
    pattern:
    \code
    <domain/username>:<password>@server/{8E27C92B-1264-101C-8A2F-040224009C02}
    \endcode
    \i To initialize a licensed control use the following pattern:
    \code
    {8E27C92B-1264-101C-8A2F-040224009C02}:<LicenseKey>
    \endcode
    \i To connect to an already running object use the following pattern:
    \code
    {8E27C92B-1264-101C-8A2F-040224009C02}&
    \endcode
    \endlist
    The first two patterns can be combined, e.g. to initialize a licensed
    control on a remote machine:
    \code
    ctrl->setControl("DOMAIN/user:password@server/{8E27C92B-1264-101C-8A2F-040224009C02}:LicenseKey");
    \endcode

    The control's read function always returns the control's UUID, if provided including the license
    key, and the name of the server, but not including the username, the domain or the password.
*/
bool QAxBase::setControl(const QString &c)
{
    if (c == ctrl)
        return !ctrl.isEmpty();
    
    clear();
    ctrl = c;
    // don't waste time for DCOM requests
    if (ctrl.indexOf("/{") != (int)ctrl.length()-39 && !ctrl.endsWith("}&")) {
        QUuid uuid(ctrl);
        if (uuid.isNull()) {
            CLSID clsid;
            HRESULT res = CLSIDFromProgID((WCHAR*)c.utf16(), &clsid);
            if (res == S_OK)
                ctrl = QUuid(clsid).toString();
            else {
                QSettings controls;
                ctrl = controls.readEntry("/Classes/" + c + "/CLSID/Default");
                if (ctrl.isEmpty()) {
                    QStringList clsids = controls.subkeyList("/Classes/CLSID");
                    for (QStringList::Iterator it = clsids.begin(); it != clsids.end(); ++it) {
                        QString clsid = *it;
                        QString name = controls.readEntry("/Classes/CLSID/" + clsid + "/Default");
                        if (name == c) {
                            QStringList subkeys = controls.subkeyList("/Classes/CLSID/" + clsid);
                            if (subkeys.contains("Control") || subkeys.contains("Insertable")) {
                                ctrl = clsid;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (ctrl.isEmpty())
            ctrl = c;
    }
    d->tryCache = true;
    if (!initialize(&d->ptr))
        d->initialized = true;
    if (isNull()) {
#ifndef QT_NO_DEBUG
        qWarning("QAxBase::setControl: requested control %s could not be instantiated.", c.latin1());
#endif
        clear();
        return false;
    }
    return true;
}

QString QAxBase::control() const
{
    return ctrl;
}

/*!
    Disables the event sink implementation for this ActiveX container.
    If you don't intend to listen to the ActiveX control's events use
    this function to speed up the meta object generation.

    Some ActiveX controls might be unstable when connected to an event
    sink. To get OLE events you must use standard COM methods to
    register your own event sink. Use queryInterface() to get access
    to the raw COM object.

    Note that this function should be called immediately after
    construction of the object (without passing an object identifier),
    and before calling QAxWidget->setControl().
*/
void QAxBase::disableEventSink()
{
    d->useEventSink = false;
}

/*!
    Disables the meta object generation for this ActiveX container.
    This also disables the event sink and class info generation. If
    you don't intend to use the Qt meta object implementation call
    this function to speed up the meta object generation.

    Some ActiveX controls might be unstable when used with OLE
    automation. Use standard COM methods to use those controls through
    the COM interfaces provided by queryInterface().

    Note that this function must be called immediately after
    construction of the object (without passing an object identifier),
    and before calling QAxWidget->setControl().
*/
void QAxBase::disableMetaObject()
{
    d->useMetaObject	= false;
    d->useEventSink	= false;
    d->useClassInfo	= false;
}

/*!
    Disables the class info generation for this ActiveX container. If
    you don't require any class information about the ActiveX control
    use this function to speed up the meta object generation.

    Note that this function must be called immediately after
    construction of the object (without passing an object identifier),
    and before calling QAxWidget->setControl().
*/
void QAxBase::disableClassInfo()
{
    d->useClassInfo = false;
}

/*!
    Disconnects and destroys the COM object.

    If you reimplement this function you must also reimplement the
    destructor to call clear(), and call this implementation at the
    end of your clear() function.
*/
void QAxBase::clear()
{
    QHash<QString, QAxEventSink*>::Iterator it =  d->eventSink.begin();
    while (it != d->eventSink.end()) {
        QAxEventSink *eventSink = it.value();
        ++it;
        eventSink->unadvise();
        eventSink->Release();
    }
    d->eventSink.clear();
    if (d->disp) {
        d->disp->Release();
        d->disp = 0;
    }
    if (d->ptr) {
        d->ptr->Release();
        d->ptr = 0;
        d->initialized = false;
    }
    
    ctrl = QString::null;
    
    if (d->metaobj && !d->cachedMetaObject) {
        delete d->metaobj;
        d->metaobj = 0;
    }
}

/*!
    This virtual function is called by setControl() and creates the
    requested COM object. \a ptr is set to the object's IUnknown 
    implementation. The function returns true if the object 
    initialization succeeded; otherwise the function returns false.

    The default implementation interprets the string returned by
    control(), and calls initializeRemote(), initializeLicensed()
    or initializeActive() if the string matches the respective 
    patterns. If no pattern is matched, or if remote or licensed 
    initialization fails, CoCreateInstance is used directly to create 
    the object.

    See the \l control property documentation for details about
    supported patterns.

    The interface returned in \a ptr must be referenced exactly once
    when this function returns. The interface provided by e.g.
    CoCreateInstance is already referenced, and there is no need to
    reference it again.
*/
bool QAxBase::initialize(IUnknown **ptr)
{
    if (*ptr || control().isEmpty())
        return false;
    
    *ptr = 0;
    
    bool res = false;
    
    QString ctrl(control());
    if (ctrl.contains("/{")) // DCOM request
        res = initializeRemote(ptr);
    else if (ctrl.contains("}:")) // licensed control
        res = initializeLicensed(ptr);
    else if (ctrl.contains("}&")) // running object
        res = initializeActive(ptr);
    
    if (!res) // standard
        res = S_OK == CoCreateInstance(QUuid(ctrl), 0, CLSCTX_SERVER, IID_IUnknown, (void**)ptr);
    
    return *ptr != 0;
}

/*!
    Creates an instance of a licensed control, and returns the IUnknown interface
    to the object in \a ptr. This functions returns true if successful, otherwise
    returns false.

    This function is called by initialize() if the control string contains the 
    substring "}:". The license key needs to follow this substring.

    \sa initialize()
*/
bool QAxBase::initializeLicensed(IUnknown** ptr)
{
    int at = control().lastIndexOf("}:");
    
    QString clsid(control().left(at));
    QString key(control().mid(at+2));
    
    IClassFactory *factory = 0;
    CoGetClassObject(QUuid(clsid), CLSCTX_SERVER, 0, IID_IClassFactory, (void**)&factory);
    if (!factory)
        return false;
    initializeLicensedHelper(factory, key, ptr);
    factory->Release();
    
    return *ptr != 0;
}

/* \internal
    Called by initializeLicensed and initializedRemote to create an object
    via IClassFactory2.
*/
bool QAxBase::initializeLicensedHelper(void *f, const QString &key, IUnknown **ptr)
{
    IClassFactory *factory = (IClassFactory*)f;
    IClassFactory2 *factory2 = 0;
    factory->QueryInterface(IID_IClassFactory2, (void**)&factory2);
    if (factory2) {
        BSTR bkey = QStringToBSTR(key);
        factory2->CreateInstanceLic(0, 0, IID_IUnknown, bkey, (void**)ptr);
        SysFreeString(bkey);
        factory2->Release();
    } else {  // give it a shot without license
        factory->CreateInstance(0, IID_IUnknown, (void**)ptr);
    }
    return *ptr != 0;
}

/*!
    Returns an active instance running on the current machine, and returns the
    IUnknown interface to the running object in \a ptr. This function returns true 
    if successful, otherwise returns false.

    This function is called by initialize() if the control string contains the 
    substring "}&".

    \sa initialize()
*/
bool QAxBase::initializeActive(IUnknown** ptr)
{
    int at = control().lastIndexOf("}&");
    QString clsid(control().left(at));
    
    GetActiveObject(QUuid(clsid), 0, ptr);
    
    return *ptr != 0;
}


// There seams to be a naming problem in mingw headers 
#ifdef Q_CC_GNU
#ifndef COAUTHIDENTITY
#define COAUTHIDENTITY AUTH_IDENTITY
#endif
#endif


/*!
    Creates the instance on a remote server, and returns the IUnknown interface
    to the object in \a ptr. This function returns true if successful, otherwise
    returns false.

    This function is called by initialize() if the control string contains the 
    substring "/{". The information about the remote machine needs to be provided
    in front of the substring.

    \sa initialize()
*/
bool QAxBase::initializeRemote(IUnknown** ptr)
{
    int at = control().lastIndexOf("/{");
    
    QString server(control().left(at));
    QString clsid(control().mid(at+1));
    
    QString user;
    QString domain;
    QString passwd;
    QString key;
    
    at = server.indexOf('@');
    if (at != -1) {
        user = server.left(at);
        server = server.mid(at+1);
        
        at = user.indexOf(':');
        if (at != -1) {
            passwd = user.mid(at+1);
            user = user.left(at);
        }
        at = user.indexOf('/');
        if (at != -1) {
            domain = user.left(at);
            user = user.mid(at+1);
        }
    }
    
    at = clsid.lastIndexOf("}:");
    if (at != -1) {
        key = clsid.mid(at+2);
        clsid = clsid.left(at);
    }
    
    ctrl = server + "/" + clsid;
    if (!key.isEmpty())
        ctrl = ctrl + ":" + key;
    
    COAUTHIDENTITY authIdentity;
    authIdentity.UserLength = user.length();
    authIdentity.User = authIdentity.UserLength ? (ushort*)user.utf16() : 0;
    authIdentity.DomainLength = domain.length();
    authIdentity.Domain = authIdentity.DomainLength ? (ushort*)domain.utf16() : 0;
    authIdentity.PasswordLength = passwd.length();
    authIdentity.Password = authIdentity.PasswordLength ? (ushort*)passwd.utf16() : 0;
    authIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    
    COAUTHINFO authInfo;
    authInfo.dwAuthnSvc = RPC_C_AUTHN_WINNT;
    authInfo.dwAuthzSvc = RPC_C_AUTHZ_NONE;
    authInfo.pwszServerPrincName = 0;
    authInfo.dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
    authInfo.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    authInfo.pAuthIdentityData = &authIdentity;
    authInfo.dwCapabilities = 0;
    
    COSERVERINFO serverInfo;
    serverInfo.dwReserved1 = 0;
    serverInfo.dwReserved2 = 0;
    serverInfo.pAuthInfo = &authInfo;
    serverInfo.pwszName = (WCHAR*)server.utf16();
    
    IClassFactory *factory = 0;
    HRESULT res = CoGetClassObject(QUuid(clsid), CLSCTX_REMOTE_SERVER, &serverInfo, IID_IClassFactory, (void**)&factory);
    if (factory) {
        if (!key.isEmpty())
            initializeLicensedHelper(factory, key, ptr);
        else
            res = factory->CreateInstance(0, IID_IUnknown, (void**)ptr);
        factory->Release();
    }
#ifndef QT_NO_DEBUG
    if (res != S_OK)
        qSystemWarning("initializeRemote Failed", res);
#endif
    
    return res == S_OK;
}

/*!
    Requests the interface \a uuid from the COM object and sets the
    value of \a iface to the provided interface, or to 0 if the
    requested interface could not be provided.

    Returns the result of the QueryInterface implementation of the COM object.

    \sa control
*/
long QAxBase::queryInterface(const QUuid &uuid, void **iface) const
{
    *iface = 0;
    if (!d->ptr) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }
    
    if (d->ptr && !uuid.isNull())
        return d->ptr->QueryInterface(uuid, iface);
    
    return E_NOTIMPL;
}

static const char *const type_conversion[][2] =
{
    { "float", "double"},
    { "short", "int"},
    { "char", "int"},
    { 0, 0 }
};

inline QByteArray replaceType(const char *type)
{
    int i = 0;
    while (type_conversion[i][0]) {
        if (!qstrcmp(type, type_conversion[i][0]))
            return type_conversion[i][1];
        ++i;
    }
    return type;
}

class MetaObjectGenerator
{
public:
    MetaObjectGenerator(QAxBase *ax, QAxBasePrivate *dptr);
    ~MetaObjectGenerator();
    
    QMetaObject *metaObject(const QMetaObject *parentObject);
    
private:
    void readClassInfo();
    void readEnumInfo();
    void readInterfaceInfo();
    void readFuncsInfo(ITypeInfo *typeinfo, ushort nFuncs);
    void readVarsInfo(ITypeInfo *typeinfo, ushort nVars);
    void readEventInfo();
    
    QMetaObject *tryCache();
    
    QByteArray createPrototype(FUNCDESC *funcdesc, ITypeInfo *typeinfo, const QList<QByteArray> &names,
        QByteArray &type, QList<QByteArray> &parameters);
    
    QByteArray usertypeToString(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function);
    QByteArray guessTypes(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function);
    
    // ### from qmetaobject.cpp
    enum ProperyFlags  {
        Invalid			= 0x00000000,
            Readable		= 0x00000001,
            Writable		= 0x00000002,
            Resetable		= 0x00000004,
            EnumOrFlag		= 0x00000008,
            StdCppSet		= 0x00000100,
            Override		= 0x00000200,
            Designable		= 0x00001000,
            ResolveDesignable	= 0x00002000,
            Scriptable		= 0x00004000,
            ResolveScriptable	= 0x00008000,
            Stored              = 0x00010000,
            ResolveStored       = 0x00020000,
            Editable		= 0x00040000,
            ResolveEditable     = 0x00080000,
            // And our own
            RequestingEdit      = 0x00100000,
            Bindable            = 0x00200000
    };
    
    QList<QByteArray> paramList(const QByteArray &proto)
    {
        QByteArray prototype(proto);
        QByteArray parameters = prototype.mid(prototype.indexOf('(') + 1);
        parameters.truncate(parameters.length() - 1);
        
        QList<QByteArray> plist = parameters.split(',');
        return plist;
    }
    
    QByteArray replacePrototype(const QByteArray &prototype)
    {
        QByteArray proto(prototype);

        QList<QByteArray> plist = paramList(prototype);
        for (int p = 0; p < plist.count(); ++p) {
            QByteArray param(plist.at(p));
            if (param != replaceType(param)) {
                int type = 0;
                while (type_conversion[type][0]) {
                    int paren = proto.indexOf('(');
                    while ((paren = proto.indexOf(type_conversion[type][0])) != -1) {
                        proto.replace(paren, qstrlen(type_conversion[type][0]), type_conversion[type][1]);
                    }
                    ++type;
                }
                break;
            }
        }
        
        return proto;
    }
    
    QMap<QByteArray, QByteArray> classinfo_list;
    inline void addClassInfo(const char *key, const char *value)
    {
        classinfo_list.insert(key, value);
    }
    
    inline bool hasClassInfo(const char *key)
    {
        return classinfo_list.contains(key);
    }
    
    struct Method {
        QByteArray type;
        QByteArray parameters;
        int flags;
        QByteArray realPrototype;
    };
    QMap<QByteArray, Method> signal_list;
    inline void addSignal(const QByteArray &prototype, const QByteArray &parameters)
    {
        QByteArray proto(replacePrototype(prototype));
        
        Method &signal = signal_list[proto];
        signal.type = 0;
        signal.parameters = parameters;
        signal.flags = QMetaMember::Public;
        if (proto != prototype)
            signal.realPrototype = prototype;
    }
    
    void addChangedSignal(const QByteArray &function, const QByteArray &type, int memid);
    
    inline bool hasSignal(const QByteArray &prototype)
    {
        return signal_list.contains(prototype);
    }
    
    QMap<QByteArray, Method> slot_list;
    inline void addSlot(const QByteArray &type, const QByteArray &prototype, const QByteArray &parameters, int flags = QMetaMember::Public)
    {
        QByteArray proto = replacePrototype(prototype);
        
        Method &slot = slot_list[proto];
        slot.type = type;
        slot.parameters = parameters;
        slot.flags = flags;
        if (proto != prototype)
            slot.realPrototype = prototype;
    }
    
    void addSetterSlot(const QByteArray &property);
    
    inline bool hasSlot(const QByteArray &prototype)
    {
        return slot_list.contains(prototype);
    }
    
    QMap<QByteArray, QPair<QByteArray, int> > property_list;
    void addProperty(const QByteArray &type, const QByteArray &name, int flags)
    {
        QPair<QByteArray, int> &prop = property_list[name];
        if (!type.isEmpty() && type != "HRESULT")
            prop.first = replaceType(type);
        prop.second |= flags;
        QVariant::Type vartype = QVariant::nameToType(prop.first);
        if (vartype != QVariant::Invalid)
            prop.second |= vartype << 24;
        else if (prop.first == "QVariant")
            prop.second |= 0xff << 24;
        else if (prop.first.endsWith('*'))
            prop.second |= QVariant::UserType << 24;
    }
    
    inline bool hasProperty(const QByteArray &name)
    {
        return property_list.contains(name);
    }
    
    inline QByteArray propertyType(const QByteArray &name)
    {
        return property_list.value(name).first;
    }
    
    QMap<QByteArray, QList<QPair<QByteArray, int> > > enum_list;
    inline void addEnumValue(const QByteArray &enumname, const QByteArray &key, int value)
    {
        enum_list[enumname].append(QPair<QByteArray, int>(key, value));
    }
    
    inline bool hasEnum(const QByteArray &enumname)
    {
        return enum_list.contains(enumname);
    }
    
    QAxBase *that;
    QAxBasePrivate *d;
    
    IDispatch *disp;
    ITypeInfo *typeinfo;
    ITypeLib *typelib;
    
    QSettings iidnames;
    QString cacheKey;
    QByteArray debugInfo;
    
    QUuid iid_propNotifySink;
};

MetaObjectGenerator::MetaObjectGenerator(QAxBase *ax, QAxBasePrivate *dptr)
: that(ax), d(dptr), typeinfo(0), typelib(0)
{
    if (d->useClassInfo)
        iidnames.insertSearchPath(QSettings::Windows, "/Classes");
    
    if (d)
        disp = d->dispatch();
    
    iid_propNotifySink = IID_IPropertyNotifySink;
    
    addSignal("signal(QString,int,void*)", "name,argc,argv");
    addSignal("exception(int,QString,QString,QString)", "code,source,disc,help");
    addSignal("propertyChanged(QString)", "name");
    addProperty("QString", "control", Readable|Writable|Designable|Scriptable|Stored|Editable|StdCppSet);
}

MetaObjectGenerator::~MetaObjectGenerator()
{
    if (typeinfo) typeinfo->Release();
    if (typelib) typelib->Release();
}

QByteArray MetaObjectGenerator::usertypeToString(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function)
{
    HREFTYPE usertype = tdesc.hreftype;
    if (tdesc.vt != VT_USERDEFINED)
        return 0;
    
    QByteArray typeName;
    ITypeInfo *usertypeinfo = 0;
    info->GetRefTypeInfo(usertype, &usertypeinfo);
    if (usertypeinfo) {
        ITypeLib *usertypelib = 0;
        UINT index;
        usertypeinfo->GetContainingTypeLib(&usertypelib, &index);
        if (usertypelib) {
            // get type name
            BSTR usertypename = 0;
            usertypelib->GetDocumentation(index, &usertypename, 0, 0, 0);
            QByteArray userTypeName = BSTRToQString(usertypename).latin1();
            SysFreeString(usertypename);
            
            if (hasEnum(userTypeName)) // known enum?
                typeName = userTypeName;
            else if (userTypeName == "OLE_COLOR" || userTypeName == "VB_OLE_COLOR")
                typeName = "QColor";
            else if (userTypeName == "IFontDisp" || userTypeName == "IFontDisp*" || userTypeName == "IFont" || userTypeName == "IFont*")
                typeName = "QFont";
            else if (userTypeName == "Picture" || userTypeName == "Picture*")
                typeName = "QPixmap";
            
            if (typeName.isEmpty()) {
                TYPEATTR *typeattr = 0;
                usertypeinfo->GetTypeAttr(&typeattr);
                if (typeattr) {
                    if (typeattr->typekind == TKIND_ALIAS)
                        userTypeName = guessTypes(typeattr->tdescAlias, usertypeinfo, function);
                    else if (typeattr->typekind == TKIND_DISPATCH || typeattr->typekind == TKIND_COCLASS)
                        userTypeName = "IDispatch";
                }
                
                usertypeinfo->ReleaseTypeAttr(typeattr);
                typeName = userTypeName;
            }
            usertypelib->Release();
        }
        usertypeinfo->Release();
    }
    return typeName;
}

QByteArray MetaObjectGenerator::guessTypes(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function)
{
    QByteArray str;
    switch (tdesc.vt) {
    case VT_VOID:
        break;
    case VT_BSTR:
        str = "QString";
        break;
    case VT_BOOL:
        str = "bool";
        break;
    case VT_I1:
        str = "char";
        break;
    case VT_I2:
        str = "short";
        break;
    case VT_I4:
    case VT_INT:
        str = "int";
        break;
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_UINT:
        str = "uint";
        break;
    case VT_CY:
        str = "Q_LLONG";
        break;
    case VT_R4:
        str = "float";
        break;
    case VT_R8:
        str = "double";
        break;
    case VT_DATE:
        str = "QDateTime";
        break;
    case VT_DISPATCH:
        str = "IDispatch*";
        break;
    case VT_VARIANT:
        str = "QVariant";
        break;
    case VT_UNKNOWN:
        str = "IUnknown*";
        break;
    case VT_HRESULT:
        str = "HRESULT";
        break;
    case VT_PTR:
        str = guessTypes(*tdesc.lptdesc, info, function);
        switch(tdesc.lptdesc->vt) {
        case VT_BSTR:
        case VT_I1:
        case VT_I2:
        case VT_I4:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_BOOL:
        case VT_R4:
        case VT_R8:
        case VT_INT:
        case VT_UINT:
        case VT_CY:
            str += "&";
            break;
        case VT_PTR:
            if (str == "QFont" || str == "QPixmap") {
                str += "&";
                break;
            }
            // FALLTHROUGH
        default:
            if (str == "QColor")
                str += "&";
            else if (str == "QDateTime")
                str += "&";
            else if (str == "QVariantList")
                str += "&";
            else if (str == "QByteArray")
                str += "&";
            else if (str == "QStringList")
                str += "&";
            else if (!str.isEmpty() && hasEnum(str))
                str += "&";
            else if (!str.isEmpty() && str != "QFont" && str != "QPixmap" && str != "QVariant")
                str += "*";
        }
        break;
    case VT_SAFEARRAY:
        if (tdesc.lpadesc->tdescElem.vt == VT_UI1) {
            str = "QByteArray";
            break;
        } else if (tdesc.lpadesc->tdescElem.vt == VT_BSTR) {
            str = "QStringList";
            break;
        } else if (tdesc.lpadesc->tdescElem.vt == VT_VARIANT) {
            str = "QVariantList";
            break;
        }
        str = guessTypes(tdesc.lpadesc->tdescElem, info, function);
        if (!str.isEmpty())
            str = "QList<" + str + ">";
        break;
    case VT_CARRAY:
        str = guessTypes(tdesc.lpadesc->tdescElem, info, function);
        if (!str.isEmpty()) {
            for (int index = 0; index < tdesc.lpadesc->cDims; ++index)
                str += "[" + QByteArray::number(tdesc.lpadesc->rgbounds[index].cElements) + "]";
        }
        break;
    case VT_USERDEFINED:
        str = usertypeToString(tdesc, info, function);
        break;
        
    default:
        break;
    }
    
    if (tdesc.vt & VT_BYREF)
        str += "&";
    
    return str;
}

void MetaObjectGenerator::readClassInfo()
{
    // Read class information
    IProvideClassInfo *classinfo = 0;
    d->ptr->QueryInterface(IID_IProvideClassInfo, (void**)&classinfo);
    if (classinfo) {
        ITypeInfo *info = 0;
        classinfo->GetClassInfo(&info);
        TYPEATTR *typeattr = 0;
        if (info)
            info->GetTypeAttr(&typeattr);
        
        QString coClassID;
        if (typeattr) {
            QUuid clsid(typeattr->guid);
            coClassID = clsid.toString().toUpper();
#ifndef QAX_NO_CLASSINFO
            // UUID
            if (d->useClassInfo && !hasClassInfo("CoClass")) {
                QString coClassIDstr = iidnames.readEntry("/CLSID/" + coClassID + "/Default", coClassID);
                addClassInfo("CoClass", coClassIDstr.isEmpty() ? coClassID.latin1() : coClassIDstr.latin1());
                QByteArray version = QByteArray::number(typeattr->wMajorVerNum) + "." + QByteArray::number(typeattr->wMinorVerNum);
                if (version != "0.0")
                    addClassInfo("Version", version);
            }
#endif
            info->ReleaseTypeAttr(typeattr);
        }
        if (info) info->Release();
        classinfo->Release();
        classinfo = 0;
        
        if (d->tryCache && !coClassID.isEmpty())
            cacheKey = QString("%1$%2$%3").arg(coClassID).arg((int)d->useEventSink).arg((int)d->useClassInfo);
    }
    
    UINT index = 0;
    if (disp && !typeinfo)
        disp->GetTypeInfo(index, LOCALE_USER_DEFAULT, &typeinfo);
    
    if (typeinfo && !typelib)
        typeinfo->GetContainingTypeLib(&typelib, &index);
    
    if (!typelib) {
        QSettings controls;
        QString tlid = controls.readEntry("/Classes/CLSID/" + that->control() + "/TypeLib/.");
        QString tlfile;
        if (!tlid.isEmpty()) {
            QStringList versions = controls.subkeyList("/Classes/TypeLib/" + tlid);
            QStringList::Iterator vit = versions.begin();
            while (tlfile.isEmpty() && vit != versions.end()) {
                QString version = *vit;
                ++vit;
                tlfile = controls.readEntry("/Classes/Typelib/" + tlid + "/" + version + "/0/win32/.");
            }
        } else {
            tlfile = controls.readEntry("/Classes/CLSID/" + that->control() + "/InprocServer32/.");
            if (tlfile.isEmpty())
                tlfile = controls.readEntry("/Classes/CLSID/" + that->control() + "/LocalServer32/.");
        }
        if (!tlfile.isEmpty()) {
            LoadTypeLib((OLECHAR*)tlfile.utf16(), &typelib);
            if (!typelib) {
                tlfile = tlfile.left(tlfile.lastIndexOf('.')) + ".tlb";
                LoadTypeLib((OLECHAR*)tlfile.utf16(), &typelib);
            }
        }
    }
    
    if (!typeinfo && typelib)
        typelib->GetTypeInfoOfGuid(QUuid(that->control()), &typeinfo);
    
    if (!typeinfo || !cacheKey.isEmpty() || !d->tryCache)
        return;
    
    TYPEATTR *typeattr = 0;
    typeinfo->GetTypeAttr(&typeattr);
    
    QString interfaceID;
    if (typeattr) {
        QUuid iid(typeattr->guid);
        interfaceID = iid.toString().toUpper();
        
        typeinfo->ReleaseTypeAttr(typeattr);
        // ### event interfaces!!
        if (!interfaceID.isEmpty())
            cacheKey = QString("%1$%2$%3").arg(interfaceID).arg((int)d->useEventSink).arg((int)d->useClassInfo);
    }
}

void MetaObjectGenerator::readEnumInfo()
{
    if (!typelib)
        return;
    
    int enum_serial = 0;
    UINT index = typelib->GetTypeInfoCount();
    for (UINT i = 0; i < index; ++i) {
        TYPEKIND typekind;
        typelib->GetTypeInfoType(i, &typekind);
        if (typekind == TKIND_ENUM) {
            // Get the type information for the enum
            ITypeInfo *enuminfo = 0;
            typelib->GetTypeInfo(i, &enuminfo);
            if (!enuminfo)
                continue;
            
            // Get the name of the enumeration
            BSTR enumname;
            QByteArray enumName;
            if (typelib->GetDocumentation(i, &enumname, 0, 0, 0) == S_OK) {
                enumName = BSTRToQString(enumname).latin1();
                SysFreeString(enumname);
            } else {
                enumName = "enum" + QByteArray::number(++enum_serial);
            }
            
            // Get the attributes of the enum type
            TYPEATTR *typeattr = 0;
            enuminfo->GetTypeAttr(&typeattr);
            if (typeattr) {
                // Get all values of the enumeration
                for (UINT vd = 0; vd < (UINT)typeattr->cVars; ++vd) {
                    VARDESC *vardesc = 0;
                    enuminfo->GetVarDesc(vd, &vardesc);
                    if (vardesc && vardesc->varkind == VAR_CONST) {
                        int value = vardesc->lpvarValue->lVal;
                        int memid = vardesc->memid;
                        // Get the name of the value
                        BSTR valuename;
                        QByteArray valueName;
                        UINT maxNamesOut;
                        enuminfo->GetNames(memid, &valuename, 1, &maxNamesOut);
                        if (maxNamesOut) {
                            valueName = BSTRToQString(valuename).latin1();
                            SysFreeString(valuename);
                        } else {
                            valueName = "value" + QByteArray::number(vd);
                        }
                        // Store value
                        addEnumValue(enumName, valueName, value);
                    }
                    enuminfo->ReleaseVarDesc(vardesc);
                }
            }
            enuminfo->ReleaseTypeAttr(typeattr);
            enuminfo->Release();
        }
    }
}

void MetaObjectGenerator::addChangedSignal(const QByteArray &function, const QByteArray &type, int memid)
{
    QAxEventSink *eventSink = d->eventSink.value(iid_propNotifySink);
    if (!eventSink && d->useEventSink) {
        eventSink = new QAxEventSink(that);
        d->eventSink.insert(iid_propNotifySink, eventSink);
    }
    // generate changed signal
    QByteArray signalName(function);
    signalName += "Changed";
    QByteArray signalProto = signalName + "(" + replaceType(type) + ")";
    if (!hasSignal(signalProto))
        addSignal(signalProto, function);
    if (eventSink)
        eventSink->addProperty(memid, function, signalProto);
}

void MetaObjectGenerator::addSetterSlot(const QByteArray &property)
{
    QByteArray set;
    QByteArray prototype(property);
    if (isupper(prototype.at(0))) {
        set = "Set";
    } else {
        set = "set";
        prototype[0] = toupper(prototype[0]);
    }
    prototype = set + prototype + "(" + propertyType(property) + ")";
    if (!hasSlot(prototype))
        addSlot(0, prototype, property);
}

QByteArray MetaObjectGenerator::createPrototype(FUNCDESC *funcdesc, ITypeInfo *typeinfo, const QList<QByteArray> &names,
                                             QByteArray &type, QList<QByteArray> &parameters)
{
    QByteArray prototype;
    QByteArray function(names.at(0));
    // get function prototype
    type = guessTypes(funcdesc->elemdescFunc.tdesc, typeinfo, function);
    prototype = function + "(";
    if (funcdesc->invkind == INVOKE_FUNC && type == "HRESULT")
        type = 0;
    
    for (int p = 1; p < names.count(); ++p) {
        // parameter
        QByteArray paramName = names.at(p);
        bool optional = p > (funcdesc->cParams - funcdesc->cParamsOpt);
        TYPEDESC tdesc = funcdesc->lprgelemdescParam[p-1].tdesc;
        PARAMDESC pdesc = funcdesc->lprgelemdescParam[p-1].paramdesc;
        
        QByteArray ptype = guessTypes(tdesc, typeinfo, function);
        if (pdesc.wParamFlags & PARAMFLAG_FRETVAL) {
            if (ptype.endsWith("&")) {
                ptype.truncate(ptype.length() - 1);
            } else if (ptype.endsWith("**")) {
                ptype.truncate(ptype.length() - 1);
            }
            type = ptype;
        } else {
            prototype += ptype;
            if (pdesc.wParamFlags & PARAMFLAG_FOUT && !ptype.endsWith("&") && !ptype.endsWith("**"))
                prototype += "&";
            if (optional)
                paramName += "=0";
            parameters << paramName;
        }
        if (p < funcdesc->cParams && !(pdesc.wParamFlags & PARAMFLAG_FRETVAL))
            prototype += ",";
    }
    
    if (!prototype.isEmpty()) {
        if (prototype.right(1) == ",")
            prototype[prototype.length()-1] = ')';
        else
            prototype += ")";
    }
    
    return prototype;
}

void MetaObjectGenerator::readFuncsInfo(ITypeInfo *typeinfo, ushort nFuncs)
{
    // get information about all functions
    for (ushort fd = 0; fd < nFuncs ; ++fd) {
        FUNCDESC *funcdesc = 0;
        typeinfo->GetFuncDesc(fd, &funcdesc);
        if (!funcdesc)
            break;
        
        // ignore hidden functions
        if (funcdesc->wFuncFlags & FUNCFLAG_FHIDDEN) {
            typeinfo->ReleaseFuncDesc(funcdesc);
            continue;
        }
        
        QByteArray function;
        QByteArray type;
        QByteArray prototype;
        QList<QByteArray> parameters;
        int flags = 0;
        
        // parse function description
        BSTR bstrNames[256];
        UINT maxNames = 255;
        UINT maxNamesOut;
        typeinfo->GetNames(funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut);
        QList<QByteArray> names;
        int p;
        for (p = 0; p < maxNamesOut; ++p) {
            names << BSTRToQString(bstrNames[p]).latin1();
            SysFreeString(bstrNames[p]);
        }
        
        // function name
        function = names.at(0);
        if ((maxNamesOut == 3 && function == "QueryInterface") ||
            (maxNamesOut == 1 && function == "AddRef") ||
            (maxNamesOut == 1 && function == "Release") ||
            (maxNamesOut == 9 && function == "Invoke") ||
            (maxNamesOut == 6 && function == "GetIDsOfNames") ||
            (maxNamesOut == 2 && function == "GetTypeInfoCount") ||
            (maxNamesOut == 4 && function == "GetTypeInfo")) {
            typeinfo->ReleaseFuncDesc(funcdesc);
            continue;
        }
        
        prototype = createPrototype(/*in*/ funcdesc, typeinfo, names, /*out*/type, parameters);
        
        // get type of function
        switch(funcdesc->invkind) {
        case INVOKE_PROPERTYGET: // property
        case INVOKE_PROPERTYPUT:
            if (funcdesc->cParams <= 1) {
                flags = Readable;
                if (funcdesc->invkind == INVOKE_PROPERTYGET)
                    flags |= Readable;
                else
                    flags |= Writable;
                if (!(funcdesc->wFuncFlags & FUNCFLAG_FNONBROWSABLE))
                    flags |= Designable;
                if (!(funcdesc->wFuncFlags & FUNCFLAG_FRESTRICTED))
                    flags |= Scriptable;
                if (funcdesc->wFuncFlags & FUNCFLAG_FREQUESTEDIT)
                    flags |= RequestingEdit;
                if (hasEnum(type))
                    flags |= EnumOrFlag;
                
                if (funcdesc->wFuncFlags & FUNCFLAG_FBINDABLE) {
                    addChangedSignal(function, type, funcdesc->memid);
                    flags |= Bindable;
                }
                addProperty(type, function, flags);
                // don't generate slots for incomplete properties
                if (type.isEmpty())
                    break;
                
                // Done for getters
                if (funcdesc->invkind == INVOKE_PROPERTYGET)
                    break;
                
                // generate setter slot
                if (funcdesc->invkind == INVOKE_PROPERTYPUT && hasProperty(function)) {
                    addSetterSlot(function);
                    break;
                }
            }
            // FALL THROUGH to support multi-variat properties
        case INVOKE_FUNC: // method
            {
                bool cloned = false;
                bool defargs;
                do {
                    QByteArray pnames;
                    for (p = 0; p < parameters.count(); ++p) {
                        pnames += parameters.at(p);
                        if (p < parameters.count() - 1)
                            pnames += ',';
                    }
                    defargs = pnames.contains("=0");
                    int flags = QMetaMember::Public;
                    if (cloned)
                        flags |= QMetaMember::Cloned;
                    cloned |= defargs;
                    addSlot(type, prototype, pnames.replace("=0", ""), flags);
                    
                    if (defargs) {
                        parameters.takeLast();
                        int lastParam = prototype.lastIndexOf(",");
                        if (lastParam == -1)
                            lastParam = prototype.indexOf("(") + 1;
                        prototype.truncate(lastParam);
                        prototype += ")";
                    }
                } while (defargs);
            }
            break;
            
        default:
            break;
        }
#if 0 // documentation in metaobject would be cool?
        // get function documentation
        BSTR bstrDocu;
        info->GetDocumentation(funcdesc->memid, 0, &bstrDocu, 0, 0);
        QString strDocu = BSTRToQString(bstrDocu);
        if (!!strDocu)
            desc += "[" + strDocu + "]";
        desc += "\n";
        SysFreeString(bstrDocu);
#endif
        typeinfo->ReleaseFuncDesc(funcdesc);
    }
}

void MetaObjectGenerator::readVarsInfo(ITypeInfo *typeinfo, ushort nVars)
{
    // get information about all variables
    for (ushort vd = 0; vd < nVars; ++vd) {
        VARDESC *vardesc;
        typeinfo->GetVarDesc(vd, &vardesc);
        if (!vardesc)
            break;
        
        // no use if it's not a dispatched variable
        if (vardesc->varkind != VAR_DISPATCH ||
            vardesc->wVarFlags & VARFLAG_FHIDDEN) {
            typeinfo->ReleaseVarDesc(vardesc);
            continue;
        }
        
        // get variable name
        BSTR bstrName;
        UINT maxNames = 1;
        UINT maxNamesOut;
        typeinfo->GetNames(vardesc->memid, &bstrName, maxNames, &maxNamesOut);
        if (maxNamesOut != 1) {
            typeinfo->ReleaseVarDesc(vardesc);
            continue;
        }
        QByteArray variableType;
        QByteArray variableName;
        int flags = 0;
        
        variableName = BSTRToQString(bstrName).latin1();
        SysFreeString(bstrName);
        
        // get variable type
        TYPEDESC typedesc = vardesc->elemdescVar.tdesc;
        variableType = guessTypes(typedesc, typeinfo, variableName);
        
        // generate meta property
        if (!hasProperty(variableName)) {
            flags = Readable;
            if (!(vardesc->wVarFlags & VARFLAG_FREADONLY))
                flags |= Writable;
            if (!(vardesc->wVarFlags & VARFLAG_FNONBROWSABLE))
                flags |= Designable;
            if (!(vardesc->wVarFlags & VARFLAG_FRESTRICTED))
                flags |= Scriptable;
            if (vardesc->wVarFlags & VARFLAG_FREQUESTEDIT)
                flags |= RequestingEdit;
            if (hasEnum(variableType))
                flags |= EnumOrFlag;
            
            if (vardesc->wVarFlags & VARFLAG_FBINDABLE) {
                addChangedSignal(variableName, variableType, vardesc->memid);
                flags |= Bindable;
            }
            addProperty(variableType, variableName, flags);
        }
        
        // generate a set slot
        if (!(vardesc->wVarFlags & VARFLAG_FREADONLY))
            addSetterSlot(variableName);
        
#if 0 // documentation in metaobject would be cool?
        // get function documentation
        BSTR bstrDocu;
        info->GetDocumentation(vardesc->memid, 0, &bstrDocu, 0, 0);
        QString strDocu = BSTRToQString(bstrDocu);
        if (!!strDocu)
            desc += "[" + strDocu + "]";
        desc += "\n";
        SysFreeString(bstrDocu);
#endif
        typeinfo->ReleaseVarDesc(vardesc);
    }
}

void MetaObjectGenerator::readInterfaceInfo()
{
    int interface_serial = 0;
    while (typeinfo) {
        ushort nFuncs = 0;
        ushort nVars = 0;
        ushort nImpl = 0;
        // get information about type
        TYPEATTR *typeattr;
        typeinfo->GetTypeAttr(&typeattr);
        bool interesting = true;
        if (typeattr) {
            // get number of functions, variables, and implemented interfaces
            nFuncs = typeattr->cFuncs;
            nVars = typeattr->cVars;
            nImpl = typeattr->cImplTypes;
            
            if ((typeattr->typekind == TKIND_DISPATCH || typeattr->typekind == TKIND_INTERFACE) &&
                (typeattr->guid != IID_IDispatch && typeattr->guid != IID_IUnknown)) {
#ifndef QAX_NO_CLASSINFO
                if (d->useClassInfo) {
                    // UUID
                    QUuid uuid(typeattr->guid);
                    QString uuidstr = uuid.toString().toUpper();
                    uuidstr = iidnames.readEntry("/Interface/" + uuidstr + "/Default", uuidstr);
                    addClassInfo("Interface " + QByteArray::number(++interface_serial), uuidstr.latin1());
                }
#endif
                typeinfo->ReleaseTypeAttr(typeattr);
            } else {
                interesting = false;
            }
        }
        
        if (interesting) {
            readFuncsInfo(typeinfo, nFuncs);
            readVarsInfo(typeinfo, nVars);
        }
        
        if (!nImpl) {
            typeinfo->Release();
            typeinfo = 0;
            break;
        }
        
        // go up one base class
        HREFTYPE pRefType;
        typeinfo->GetRefTypeOfImplType(0, &pRefType);
        ITypeInfo *baseInfo = 0;
        typeinfo->GetRefTypeInfo(pRefType, &baseInfo);
        typeinfo->Release();
        if (typeinfo == baseInfo) { // IUnknown inherits IUnknown ???
            baseInfo->Release();
            typeinfo = 0;
            break;
        }
        typeinfo = baseInfo;
    }
}

void MetaObjectGenerator::readEventInfo()
{
    int event_serial = 0;
    IConnectionPointContainer *cpoints = 0;
    if (d->useEventSink)
        d->ptr->QueryInterface(IID_IConnectionPointContainer, (void**)&cpoints);
    if (cpoints) {
        // Get connection point enumerator
        IEnumConnectionPoints *epoints = 0;
        cpoints->EnumConnectionPoints(&epoints);
        if (epoints) {
            ULONG c = 1;
            IConnectionPoint *cpoint = 0;
            epoints->Reset();
            QList<QUuid> cpointlist;
            do {
                if (cpoint) cpoint->Release();
                cpoint = 0;
                epoints->Next(c, &cpoint, &c);
                if (!c)
                    break;
                
                IID conniid;
                cpoint->GetConnectionInterface(&conniid);
                // workaround for typelibrary bug of Word.Application
                QUuid connuuid(conniid);
                if (cpointlist.contains(connuuid))
                    break;
                
#ifndef QAX_NO_CLASSINFO
                if (d->useClassInfo) {
                    QString uuidstr = connuuid.toString().toUpper();
                    uuidstr = iidnames.readEntry("/Interface/" + uuidstr + "/Default", uuidstr);
                    addClassInfo("Event Interface " + QByteArray::number(++event_serial), uuidstr.latin1());
                }
#endif

                // get information about type
                if (conniid == IID_IPropertyNotifySink) {
                    // test whether property notify sink has been created already, and advise on it
                    QAxEventSink *eventSink = d->eventSink.value(iid_propNotifySink);
                    if (eventSink)
                        eventSink->advise(cpoint, conniid);
                    continue;
                }

                ITypeInfo *eventinfo = 0;
                if (typelib)
                    typelib->GetTypeInfoOfGuid(conniid, &eventinfo);
                
                if (eventinfo) {
                    // avoid recursion (see workaround above)
                    cpointlist.append(connuuid);
                    
                    TYPEATTR *eventattr;
                    eventinfo->GetTypeAttr(&eventattr);
                    if (!eventattr)
                        continue;
                    if (eventattr->typekind != TKIND_DISPATCH) {
                        eventinfo->ReleaseTypeAttr(eventattr);
                        continue;
                    }
                    
                    QAxEventSink *eventSink = d->eventSink.value(QUuid(conniid));
                    if (!eventSink) { 
                        eventSink = new QAxEventSink(that);
                        d->eventSink.insert(QUuid(conniid), eventSink);
                        eventSink->advise(cpoint, conniid);
                    }
                    
                    // get information about all event functions
                    for (UINT fd = 0; fd < (UINT)eventattr->cFuncs; ++fd) {
                        FUNCDESC *funcdesc;
                        eventinfo->GetFuncDesc(fd, &funcdesc);
                        if (!funcdesc)
                            break;
                        if (funcdesc->invkind != INVOKE_FUNC ||
                            funcdesc->funckind != FUNC_DISPATCH) {
                            eventinfo->ReleaseTypeAttr(eventattr);
                            eventinfo->ReleaseFuncDesc(funcdesc);
                            continue;
                        }
                        
                        QByteArray function;
                        QByteArray prototype;
                        QList<QByteArray> parameters;
                        int flags = 0;
                        
                        // parse event function description
                        BSTR bstrNames[256];
                        UINT maxNames = 255;
                        UINT maxNamesOut;
                        eventinfo->GetNames(funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut);
                        QList<QByteArray> names;
                        int p;
                        for (p = 0; p < (int)maxNamesOut; ++p) {
                            names << BSTRToQString(bstrNames[p]).latin1();
                            SysFreeString(bstrNames[p]);
                        }
                        
                        // get event function prototype
                        function = names.at(0);
                        QByteArray type; // dummy - we don't care about return values for signals
                        prototype = createPrototype(/*in*/ funcdesc, eventinfo, names, /*out*/type, parameters);
                        if (!hasSignal(prototype)) {
                            QByteArray pnames;
                            for (p = 0; p < parameters.count(); ++p) {
                                pnames += parameters.at(p);
                                if (p < parameters.count() - 1)
                                    pnames += ',';
                            }
                            addSignal(prototype, pnames);
                        }
                        if (eventSink)
                            eventSink->addSignal(funcdesc->memid, prototype);
                        
#if 0 // documentation in metaobject would be cool?
                        // get function documentation
                        BSTR bstrDocu;
                        eventinfo->GetDocumentation(funcdesc->memid, 0, &bstrDocu, 0, 0);
                        QString strDocu = BSTRToQString(bstrDocu);
                        if (!!strDocu)
                            desc += "[" + strDocu + "]";
                        desc += "\n";
                        SysFreeString(bstrDocu);
#endif
                        eventinfo->ReleaseFuncDesc(funcdesc);
                    }
                    eventinfo->ReleaseTypeAttr(eventattr);
                    eventinfo->Release();
                }
            } while (c);
            if (cpoint) cpoint->Release();
            epoints->Release();
        }
        cpoints->Release();
    }
}

QMetaObject *MetaObjectGenerator::tryCache()
{
    if (!cacheKey.isEmpty()) {
        d->metaobj = mo_cache.value(cacheKey);
        if (d->metaobj) {
            d->cachedMetaObject = true;
            QList<QUuid>::ConstIterator it = d->metaobj->connectionInterfaces.begin();
            while (it != d->metaobj->connectionInterfaces.end()) {
                QUuid iid = *it;
                ++it;
                
                IConnectionPointContainer *cpoints = 0;
                d->ptr->QueryInterface(IID_IConnectionPointContainer, (void**)&cpoints);
                IConnectionPoint *cpoint = 0;
                if (cpoints)
                    cpoints->FindConnectionPoint(iid, &cpoint);
                if (cpoint) {
                    QAxEventSink *sink = new QAxEventSink(that);
                    sink->advise(cpoint, iid);
                    d->eventSink.insert(iid, sink);
                    sink->sigs = d->metaobj->sigs.value(iid);
                    
                    sink->props = d->metaobj->props.value(iid);
                    sink->propsigs = d->metaobj->propsigs.value(iid);
                    cpoints->Release();
                }
            }
            
            return d->metaobj;
        }
    }
    return 0;
}

QMetaObject *MetaObjectGenerator::metaObject(const QMetaObject *parentObject)
{
    readClassInfo();
    if (d->tryCache && tryCache())
        return d->metaobj;
    readEnumInfo();
    readInterfaceInfo();
    readEventInfo();
    
#ifndef QAX_NO_CLASSINFO
    if (!debugInfo.isEmpty() && d->useClassInfo)
        addClassInfo("debugInfo", debugInfo);
#endif
    
    d->metaobj = new QAxMetaObject;
    
    // revision + classname + table + zero terminator
    int int_data_size = 1+1+2+2+2+2+2+1;
    
    int_data_size += classinfo_list.count() * 2;
    int_data_size += signal_list.count() * 5;
    int_data_size += slot_list.count() * 5;
    int_data_size += property_list.count() * 3;
    int_data_size += enum_list.count() * 4;
    for (QMap<QString, QList<QPair<QString, int> > >::ConstIterator it = enum_list.begin(); 
    it != enum_list.end(); ++it) {
        int_data_size += (*it).count() * 2;
    }
    
    uint *int_data = new uint[int_data_size];
    int_data[0] = 1; // revision number
    int_data[1] = 0; // classname index
    int_data[2] = classinfo_list.count(); // num_classinfo
    int_data[3] = 12; // idx_classinfo
    int_data[4] = signal_list.count(); // num_signals
    int_data[5] = int_data[3] + int_data[2] * 2; // idx_signals
    int_data[6] = slot_list.count(); // num_slots
    int_data[7] = int_data[5] + int_data[4] * 5; // idx_slots
    int_data[8] = property_list.count(); // num_properties
    int_data[9] = int_data[7] + int_data[6] * 5; // idx_properties
    int_data[10] = enum_list.count(); // num_enums
    int_data[11] = int_data[9] + int_data[8] * 3; // idx_enums
    int_data[int_data_size - 1] = 0; // eod;
    
    char null('\0');
    // data + zero-terminator
    QByteArray stringdata = that->className();
    stringdata += null;
    stringdata.reserve(8192);
    int string_data_size = stringdata.length();
    
    int offset = int_data[3]; //idx_classinfo
    
    // each class info in form key\0value\0
    for (QMap<QByteArray, QByteArray>::ConstIterator it = classinfo_list.begin(); it != classinfo_list.end(); ++it) {
        QByteArray key(it.key());
        QByteArray value(it.value());
        int_data[offset++] = stringdata.length();
        stringdata += key;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += value;
        stringdata += null;
    }
    Q_ASSERT(offset == int_data[5]);
    
    // each signal in form prototype\0parameters\0type\0tag\0
    for (QMap<QByteArray, Method>::ConstIterator it = signal_list.begin(); it != signal_list.end(); ++it) {
        QByteArray prototype(it.key());
        QByteArray type(it.value().type);
        QByteArray parameters(it.value().parameters);
        if (!it.value().realPrototype.isEmpty())
            d->metaobj->realPrototype[prototype] = it.value().realPrototype;
        QByteArray tag;
        int flags = it.value().flags;
        
        int_data[offset++] = stringdata.length();
        stringdata += prototype;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += parameters;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += type;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += tag;
        stringdata += null;
        int_data[offset++] = flags;
    }
    Q_ASSERT(offset == int_data[7]);
    
    // each slot in form prototype\0parameters\0type\0tag\0
    for (QMap<QByteArray, Method>::ConstIterator it = slot_list.begin(); it != slot_list.end(); ++it) {
        QByteArray prototype(it.key());
        QByteArray type(it.value().type);
        QByteArray parameters(it.value().parameters);
        if (!it.value().realPrototype.isEmpty())
            d->metaobj->realPrototype[prototype] = it.value().realPrototype;
        QByteArray tag;
        int flags = it.value().flags;
        
        int_data[offset++] = stringdata.length();
        stringdata += prototype;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += parameters;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += type;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += tag;
        stringdata += null;
        int_data[offset++] = flags;
    }
    Q_ASSERT(offset == int_data[9]);
    
    // each property in form name\0type\0
    for (QMap<QByteArray, QPair<QByteArray, int> >::ConstIterator it = property_list.begin(); it != property_list.end(); ++it) {
        QByteArray name(it.key());
        QByteArray type(it.value().first);
        int flags = it.value().second;
        
        int_data[offset++] = stringdata.length();
        stringdata += name;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += type;
        stringdata += null;
        int_data[offset++] = flags;
    }
    Q_ASSERT(offset == int_data[11]);
    
    int value_offset = offset + enum_list.count() * 4;
    // each enum in form name\0
    for (QMap<QByteArray, QList<QPair<QByteArray, int> > >::ConstIterator it = enum_list.begin(); it != enum_list.end(); ++it) {
        QByteArray name(it.key());
        int flags = 0x0; // 0x1 for flag?
        int count = it.value().count();
        
        int_data[offset++] = stringdata.length();
        stringdata += name;
        stringdata += null;
        int_data[offset++] = flags;
        int_data[offset++] = count;
        int_data[offset++] = value_offset;
        value_offset += count * 2;
    }
    Q_ASSERT(offset == int_data[11] + enum_list.count() * 4);
    
    // each enum value in form key\0
    for (QMap<QByteArray, QList<QPair<QByteArray, int> > >::ConstIterator it = enum_list.begin(); it != enum_list.end(); ++it) {
        for (QList<QPair<QByteArray,int> >::ConstIterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
            QByteArray key((*it2).first);
            int value = (*it2).second;
            int_data[offset++] = stringdata.length();
            stringdata += key;
            stringdata += null;
            int_data[offset++] = value;
        }
    }
    Q_ASSERT(offset == int_data_size-1);
    
    char *string_data = new char[stringdata.length()];
    memset(string_data, 0, sizeof(string_data));
    memcpy(string_data, stringdata, stringdata.length());
    
    // put the metaobject together
    d->metaobj->d.data = int_data;
    d->metaobj->d.stringdata = string_data;
    d->metaobj->d.superdata = parentObject;
    
    if (!cacheKey.isEmpty()) {
        mo_cache.insert(cacheKey, d->metaobj);
        d->cachedMetaObject = true;
        for (QHash<QString, QAxEventSink*>::Iterator it = d->eventSink.begin(); it != d->eventSink.end(); ++it) {
            QAxEventSink *sink = it.value();
            if (sink) {
                QUuid ciid = sink->connectionInterface();
                
                d->metaobj->connectionInterfaces.append(ciid);
                d->metaobj->sigs.insert(ciid, sink->signalMap());
                d->metaobj->props.insert(ciid, sink->propertyMap());
                d->metaobj->propsigs.insert(ciid, sink->propSignalMap());
            }
        }
    }
    
    return d->metaobj;
}

static const uint qt_meta_data_QAxBase[] = {

 // content:
       1,       // revision
       0,       // classname
       0,   12, // classinfo
       3,   12, // signals
       0,   27, // slots
       1,   27, // properties
       0,   30, // enums/sets

 // signals: signature, parameters, type, tag, flags
      46,   24,   23,   23, 0x0,
      90,   85,   23,   23, 0x0,
     130,  115,   23,   23, 0x0,

 // properties: name, type, flags
     164,  156, 0x03055103,

       0        // eod
};

static const char qt_meta_stringdata_QAxBase[] = {
    "QAxBase\0\0code,source,desc,help\0"
    "exception(int,QString,QString,QString)\0name\0propertyChanged(QString)\0"
    "name,argc,argv\0signal(QString,int,void*)\0QString\0control\0"
};

/*!
    \reimp

    The metaobject is generated on the fly from the information
    provided by the IDispatch and ITypeInfo interface implementations
    in the COM object.
*/
const QMetaObject *QAxBase::metaObject() const
{
    if (d->metaobj)
        return d->metaobj;
    const QMetaObject* parentObject = parentMetaObject();
    
    if (!d->ptr && !d->initialized) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }
    
#ifdef QT_THREAD_SUPPORT
    // only one thread at a time can generate meta objects
    QMutexLocker locker(&cache_mutex);
#endif
    
    // return the default meta object if not yet initialized
    if (!d->ptr || !d->useMetaObject) {
        if (!d->staticMetaObject) {
            d->staticMetaObject = new QMetaObject;
            d->staticMetaObject->d.data = qt_meta_data_QAxBase;
            d->staticMetaObject->d.stringdata = qt_meta_stringdata_QAxBase;
            d->staticMetaObject->d.superdata = parentMetaObject();
        }
        return d->staticMetaObject;
    }
    MetaObjectGenerator generator((QAxBase*)this, d);
    return generator.metaObject(parentObject);
}

/*!
    \fn QString QAxBase::generateDocumentation()

    Returns a rich text string with documentation for the
    wrapped COM object. Dump the string to an HTML-file,
    or use it in e.g. a QTextBrowser widget.
*/

static bool checkHRESULT(HRESULT hres, EXCEPINFO *exc, QAxBase *that, const QString &name, uint argerr)
{
    switch(hres) {
    case S_OK:
        return true;
    case DISP_E_BADPARAMCOUNT:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Bad parameter count.", name.latin1());
#endif
        return false;
    case DISP_E_BADVARTYPE:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Bad variant type.", name.latin1());
#endif
        return false;
    case DISP_E_EXCEPTION:
        {
#if defined(QT_CHECK_STATE)
            qWarning("QAxBase: Error calling IDispatch member %s: Exception thrown by server.", name.latin1());
#endif
            const QMetaObject *mo = that->metaObject();
            int exceptionSignal = mo->indexOfSignal("exception(int,QString,QString,QString)");
            if (exceptionSignal >= 0) {
                if (exc->pfnDeferredFillIn)
                    exc->pfnDeferredFillIn(exc);
                
                unsigned short code = exc->wCode ? exc->wCode : exc->scode;
                QString source = BSTRToQString(exc->bstrSource);
                QString desc = BSTRToQString(exc->bstrDescription);
                QString help = BSTRToQString(exc->bstrHelpFile);
                unsigned long helpContext = exc->dwHelpContext;
                
                if (helpContext && !help.isEmpty())
                    help += QString(" [%1]").arg(helpContext);
                
                void *argv[] = {0, &code, &source, &desc, &help};
                that->qt_metacall(QMetaObject::EmitSignal, exceptionSignal, argv);
            }
        }
        return false;
    case DISP_E_MEMBERNOTFOUND:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Member not found.", name.latin1());
#endif
        return false;
    case DISP_E_NONAMEDARGS:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: No named arguments.", name.latin1());
#endif
        return false;
    case DISP_E_OVERFLOW:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Overflow.", name.latin1());
#endif
        return false;
    case DISP_E_PARAMNOTFOUND:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Parameter %d not found.", name.latin1(), argerr);
#endif
        return false;
    case DISP_E_TYPEMISMATCH:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Type mismatch in parameter %d.", name.latin1(), argerr);
#endif
        return false;
    case DISP_E_UNKNOWNINTERFACE:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown interface.", name.latin1());
#endif
        return false;
    case DISP_E_UNKNOWNLCID:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown locale ID.", name.latin1());
#endif
        return false;
    case DISP_E_PARAMNOTOPTIONAL:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Non-optional parameter missing.", name.latin1());
#endif
        return false;
    default:
#if defined(QT_CHECK_STATE)
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown error.", name.latin1());
#endif
        return false;
    }
}

/*!
    \internal
*/
int QAxBase::internalProperty(QMetaObject::Call call, int index, void **v)
{
    const QMetaObject *mo = metaObject();
    QMetaProperty prop = mo->property(index + mo->propertyOffset());
    QString propname(prop.name());

    // hardcoded control property
    if (propname == QLatin1String("control")) {
        switch(call) {
        case QMetaObject::ReadProperty:
            *(QString*)*v = control();
            break;
        case QMetaObject::WriteProperty:
            setControl(*(QString*)*v);
            break;
        case QMetaObject::ResetProperty:
            clear();
            break;
        }
        return index - d->metaobj->propertyCount();
    }
    
    // get the IDispatch
    if (!d->ptr || !prop.isValid())
        return index;
    IDispatch *disp = d->dispatch();
    if (!disp)
        return index;
    
    // get the Dispatch ID of the function to be called
    DISPID dispid;
    OLECHAR *names = (TCHAR*)propname.utf16();
    disp->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid);
    if (dispid == DISPID_UNKNOWN)
        return index;
    
    // we located the property, so everthing that goes wrong now should not bother the caller
    index -= d->metaobj->propertyCount();
    
    VARIANTARG arg;
    VariantInit(&arg);
    DISPPARAMS params;
    EXCEPINFO excepinfo;
    UINT argerr = 0;
    HRESULT hres = E_FAIL;

    QByteArray proptype(prop.typeName());
    switch (call) {
    case QMetaObject::ReadProperty:
        params.cArgs = 0;
        params.cNamedArgs = 0;
        params.rgdispidNamedArgs = 0;
        params.rgvarg = 0;
        
        hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &arg, &excepinfo, 0);
        
        // map result VARIANTARG to void*
        QVariantToVoidStar(VARIANTToQVariant(arg, proptype, prop.type()), *v, proptype);
        break;
        
    case QMetaObject::WriteProperty:
        {
            QVariant::Type t = (QVariant::Type)prop.type();

            DISPID dispidNamed = DISPID_PROPERTYPUT;
            params.cArgs = 1;
            params.cNamedArgs = 1;
            params.rgdispidNamedArgs = &dispidNamed;
            params.rgvarg = &arg;
            
            arg.vt = VT_ERROR;
            arg.scode = DISP_E_TYPEMISMATCH;
            
            // map void* to VARIANTARG via QVariant
            QVariant qvar;
            if (prop.isEnumType()) {
                qvar = *(int*)v[0];
                proptype = 0;
            } else {
                if (t == -1) {
                    qvar = *(QVariant*)v[0];
                } else if (t == QVariant::UserType) {
                    qVariantSet(qvar, *(void**)v[0], prop.typeName());
                } else {
                    proptype = 0;
                    qvar = QCoreVariant(t, v[0]);
                }
            }

            QVariantToVARIANT(qvar, arg, proptype);
            if (arg.vt == VT_EMPTY || arg.vt == VT_ERROR) {
#ifndef QT_NO_DEBUG
                qWarning("QAxBase::setProperty(): Unhandled property type %s", prop.typeName());
#endif
                break;
            }
        }
        hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, 0, &excepinfo, &argerr);
        break;
        
    default:
        break;
    }
    
    checkHRESULT(hres, &excepinfo, this, propname, argerr);
    clearVARIANT(&arg);
    return index;
}

int QAxBase::internalInvoke(QMetaObject::Call call, int index, void **v)
{
    Q_ASSERT(call == QMetaObject::InvokeSlot);
    
    // get the IDispatch
    IDispatch *disp = d->dispatch();
    if (!disp)
        return index;
    
    // get the slot information
    QMetaMember slot = d->metaobj->slot(index + d->metaobj->slotOffset());
    QByteArray signature(slot.signature());
    QByteArray slotname(signature);
    slotname.truncate(slotname.indexOf('('));
    
    // Get the Dispatch ID of the method to be called
    bool isProperty = false;
    DISPID dispid;
    QString uni_slotname = QString::fromLatin1(slotname);
    OLECHAR *names = (TCHAR*)uni_slotname.utf16();
    disp->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid);
    if (dispid == DISPID_UNKNOWN) {
        // see if we are calling a property set function as a slot
        if (!uni_slotname.startsWith("set", QString::CaseInsensitive))
            return index;
        uni_slotname = uni_slotname.right(uni_slotname.length() - 3);
        names = (TCHAR*)uni_slotname.utf16();
        disp->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid);
        isProperty = true;
    }
    if (dispid == DISPID_UNKNOWN)
        return index;
    
    // we located the property, so everthing that goes wrong now should not bother the caller
    index -= d->metaobj->slotCount();
    
    // setup the parameters
    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    params.cArgs = isProperty ? 1 : d->metaobj->numParameter(signature);
    params.cNamedArgs = isProperty ? 1 : 0;
    params.rgdispidNamedArgs = isProperty ? &dispidNamed : 0;
    params.rgvarg = 0;
    VARIANTARG static_rgvarg[QAX_NUM_PARAMS];
    if (params.cArgs) {
        if (params.cArgs <= QAX_NUM_PARAMS)
            params.rgvarg = static_rgvarg;
        else
            params.rgvarg = new VARIANTARG[params.cArgs];
    }
    
    int p;
    for (p = 0; p < params.cArgs; ++p) {
        bool out;
        QByteArray type = d->metaobj->paramType(signature, p, &out);
        QVariant qvar(QVariant::nameToType(type), v[p + 1]);
        if (!qvar.isValid()) {
            if (type == "IDispatch*")
                qVariantSet(qvar, *(IDispatch**)v[p+1], "IDispatch*");
            else if (type == "IUnknown*")
                qVariantSet(qvar, *(IUnknown**)v[p+1], "IUnknown*");
            else if (d->metaobj->indexOfEnumerator(type) != -1)
                qvar = *(int*)v[p + 1];
        }

        QVariantToVARIANT(qvar, params.rgvarg[params.cArgs - p - 1], type, out);
    }
    
    // return value
    QByteArray rettype(slot.typeName());
    VARIANT ret;
    VARIANT *pret = rettype.isEmpty() ? 0 : &ret;
    
    // call the method
    UINT argerr = 0;
    HRESULT hres = E_FAIL;
    EXCEPINFO excepinfo;
    
    WORD wFlags = (isProperty && params.cArgs == 1) ? DISPATCH_PROPERTYPUT : DISPATCH_METHOD;
    hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, wFlags, &params, pret, &excepinfo, &argerr);
    
    // set return value
    if (pret) {
        QVariantToVoidStar(VARIANTToQVariant(ret, slot.typeName()), v[0], slot.typeName());
        pret = 0;
    }
    
    // update out parameters
    for (p = 0; p < params.cArgs; ++p) {
        bool out;
        QByteArray ptype = d->metaobj->paramType(signature, p, &out);
        if (out)
            QVariantToVoidStar(VARIANTToQVariant(params.rgvarg[params.cArgs - p - 1], ptype), v[p+1], ptype);
    }
    // clean up
    for (p = 0; p < params.cArgs; ++p)
        clearVARIANT(params.rgvarg+p);
    if (params.rgvarg != static_rgvarg)
        delete [] params.rgvarg;
    
    checkHRESULT(hres, &excepinfo, this, slotname, params.cArgs-argerr-1);
    return index;
}

/*!
    \internal
*/
int QAxBase::qt_metacall(QMetaObject::Call call, int id, void **v)
{
    if (!d->metaobj)
        metaObject();
    
    switch(call) {
    case QMetaObject::EmitSignal:
        QMetaObject::activate(qObject(), d->metaobj, id, v);
        id -= d->metaobj->signalCount();
        break;
    case QMetaObject::InvokeSlot:
        id = internalInvoke(call, id, v);
        break;
    case QMetaObject::ReadProperty:
    case QMetaObject::WriteProperty:
    case QMetaObject::ResetProperty:
        id = internalProperty(call, id, v);
        break;
    case QMetaObject::QueryPropertyScriptable:
    case QMetaObject::QueryPropertyDesignable:
    case QMetaObject::QueryPropertyStored:
    case QMetaObject::QueryPropertyEditable:
        id -= d->metaobj->propertyCount();
        break;
    }
    Q_ASSERT(id < 0);
    return id;
}

/*!
    \internal
*/
bool QAxBase::dynamicCallHelper(const QByteArray &name, void *inout, QList<QVariant> &vars, QByteArray &type)
{
    IDispatch *disp = d->dispatch();
    if (!disp)
        return false;
    
    if (!d->metaobj)
        metaObject();
    
    int varc = vars.count();
    
    QByteArray function = name;
    VARIANT staticarg;
    VARIANT *arg = 0;
    VARIANTARG *res = (VARIANTARG*)inout;
    
    unsigned short disptype;
    
    int id = -1;
    bool parse = false;
    
    if (function.contains('(')) {
        disptype = DISPATCH_METHOD;
        id = metaObject()->indexOfSlot(QMetaObject::normalizedSignature(function));
        if (id >= 0) {
            const QMetaMember slot = metaObject()->slot(id);
            function = slot.signature();
            function.truncate(function.indexOf('('));
            type = slot.typeName();
        } else {
            function = function.left(function.indexOf('('));
        }
        parse = !varc && name.length() > function.length() + 2;
        if (parse) {
            QString args = name.mid(function.length() + 1);
            // parse argument string int list of arguments
            QString curArg;
            const QChar *c = args.unicode();
            int index = 0;
            bool inString = false;
            bool inEscape = false;
            while (index < (int)args.length()) {
                QChar cc = *c;
                ++c;
                ++index;
                switch(cc.latin1()) {
                case 'n':
                    if (inEscape)
                        cc = '\n';
                    break;
                case 'r':
                    if (inEscape)
                        cc = '\r';
                    break;
                case 't':
                    if (inEscape)
                        cc = '\t';
                    break;
                case '\\':
                    if (!inEscape && inString) {
                        inEscape = true;
                        continue;
                    }
                    break;
                case '"':
                    if (!inEscape) {
                        inString = !inString;
                        continue;
                    }
                    break;
                case ' ':
                    if (!inString && curArg.isEmpty())
                        continue;
                    break;
                case ',':
                case ')':
                    if (inString)
                        break;
                    curArg = curArg.trimmed();
                    vars << curArg;
                    curArg = QString::null;
                    continue;
                default:
                    break;
                }
                inEscape = false;
                curArg += cc;
            }
            
            varc = vars.count();
        }
    } else {
        id = metaObject()->indexOfProperty(name);
        disptype = DISPATCH_PROPERTYGET;
        
        if (id >= 0) {
            const QMetaProperty prop = metaObject()->property(id);
            type = prop.typeName();
        } else {
            function = name;
        }
        if (varc == 1) {
            res = 0;
            disptype = DISPATCH_PROPERTYPUT;
        }
    }
    if (varc) {
        arg = varc == 1 ? &staticarg : new VARIANT[varc];
        for (int i = 0; i < varc; ++i) {
            VariantInit(arg + (varc - i - 1));
            bool out = false;
            QByteArray paramType;
            if (disptype == DISPATCH_PROPERTYPUT)
                paramType = type;
            else if (parse || disptype == DISPATCH_PROPERTYGET)
                paramType = 0;
            else
                paramType = d->metaobj->paramType(name, i, &out);
            
            QVariantToVARIANT(vars.at(i), arg[varc - i - 1], paramType, out);
        }
    }
    
    DISPID dispid;
    QString uni_function = QLatin1String(function);
    OLECHAR *names = (TCHAR*)uni_function.utf16();
    disp->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid);
    if (dispid == DISPID_UNKNOWN && uni_function.startsWith("set", QString::CaseInsensitive)) {
        uni_function = uni_function.mid(3);
        OLECHAR *names = (TCHAR*)uni_function.utf16();
        disptype = DISPATCH_PROPERTYPUT;
        disp->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid);
    }
    
    if (dispid == DISPID_UNKNOWN) {
#ifdef QT_CHECK_STATE
        const char *coclass = metaObject()->classInfo(metaObject()->indexOfClassInfo("CoClass")).value();
        qWarning("QAxBase::dynamicCallHelper: %s: No such method or property in %s [%s]", name.data(), control().latin1(),
            coclass ? coclass: "unknown");
        
        if (disptype == DISPATCH_METHOD) {
            for (int i = 0; i < metaObject()->slotCount(); ++i) {
                QByteArray signature = metaObject()->slot(i).signature();
                if (signature.startsWith(function))
                    qWarning("\tDid you mean %s?", signature.data());
            }
        }
#endif
        return false;
    }
    
    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    
    params.cArgs = varc;
    params.cNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? 1 : 0;
    params.rgdispidNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? &dispidNamed : 0;
    params.rgvarg = arg;
    EXCEPINFO excepinfo;
    UINT argerr = 0;
    
    HRESULT hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, disptype, &params, res, &excepinfo, &argerr);
    if (hres == DISP_E_MEMBERNOTFOUND && disptype == DISPATCH_METHOD)
        hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, res, &excepinfo, &argerr);
    
    if (disptype == DISPATCH_METHOD && id >= 0 && varc) {
        for (int i = 0; i < varc; ++i)
            if (arg[varc-i-1].vt & VT_BYREF) // update out-parameters
                vars[i] = VARIANTToQVariant(arg[varc-i-1], vars.at(i).typeName());
    }
    
    // clean up
    for (int i = 0; i < varc; ++i)
        clearVARIANT(params.rgvarg+i);
    if (arg && arg != &staticarg)
        delete[] arg;
    
    return checkHRESULT(hres, &excepinfo, this, function, varc-argerr-1);
}


/*!
    Calls the COM object's method \a function, passing the
    parameters \a var1, \a var1, \a var2, \a var3, \a var4, \a var5,
    \a var6, \a var7 and \a var8, and returns the value returned by
    the method, or an invalid QVariant if the method does not return
    a value or when the function call failed.

    If \a function is a method of the object the string must be provided
    as the full prototype, for example as it would be written in a
    QObject::connect() call.
    \code
    activeX->dynamicCall("Navigate(const QString&)", "www.trolltech.com");
    \endcode

    Alternatively a function can be called passing the parameters embedded
    in the string, e.g. above function can also be invoked using
    \code
    activeX->dynamicCall("Navigate(\"www.trolltech.com\");
    \endcode
    All parameters are passed as strings; it depends on the control whether
    they are interpreted correctly, and is slower than using the prototype
    with correctly typed parameters.

    If \a function is a property the string has to be the name of the
    property. The property setter is called when \a var1 is a valid QVariant,
    otherwise the getter is called.
    \code
    activeX->dynamicCall("Value", 5);
    QString text = activeX->dynamicCall("Text").toString();
    \endcode
    Note that it is faster to get and set properties using
    QObject::property() and QObject::setProperty().

    It is only possible to call functions through dynamicCall() that
    have parameters or return values of datatypes supported by
    QVariant. See the QAxBase class documentation for a list of
    supported and unsupported datatypes. If you want to call functions
    that have unsupported datatypes in the parameter list, use
    queryInterface() to retrieve the appropriate COM interface, and
    use the function directly.

    \code
    IWebBrowser2 *webBrowser = 0;
    activeX->queryInterface(IID_IWebBrowser2, (void**)&webBrowser);
    if (webBrowser) {
        webBrowser->Navigate2(pvarURL);
	webBrowser->Release();
    }
    \endcode

    This is also more efficient.
*/
QVariant QAxBase::dynamicCall(const QString &function,
                              const QVariant &var1,
                              const QVariant &var2,
                              const QVariant &var3,
                              const QVariant &var4,
                              const QVariant &var5,
                              const QVariant &var6,
                              const QVariant &var7,
                              const QVariant &var8)
{
    QList<QVariant> vars;
    QVariant var = var1;
    int argc = 1;
    while(var.isValid()) {
        vars << var;
        switch(++argc) {
        case 2: var = var2; break;
        case 3: var = var3; break;
        case 4: var = var4; break;
        case 5: var = var5; break;
        case 6: var = var6; break;
        case 7: var = var7; break;
        case 8: var = var8; break;
        default:var = QVariant(); break;
        }
    }
    
    return dynamicCall(function, vars);
}

/*!
    \overload

    Calls the COM object's method \a function, passing the
    parameters in \a vars, and returns the value returned by
    the method. If the method does not return a value or when
    the function call failed this function returns an invalid
    QVariant object.

    The QVariant objects in \a vars are updated when the method has
    out-parameters.
*/
QVariant QAxBase::dynamicCall(const QString &function, QList<QVariant> &vars)
{
    VARIANTARG res;
    res.vt = VT_EMPTY;
    
    QByteArray rettype;
    bool ok = dynamicCallHelper(QMetaObject::normalizedSignature(function.latin1()), &res, vars, rettype);
    
    QVariant qvar = VARIANTToQVariant(res, rettype);
    clearVARIANT(&res);
    
    return qvar;
}

/*!
    Returns a pointer to a QAxObject wrapping the COM object provided
    by the method or property \a name, passing passing the parameters
    \a var1, \a var1, \a var2, \a var3, \a var4, \a var5, \a var6,
    \a var7 and \a var8.

    If \a name is provided by a method the string must include the
    full function prototype.

    If \a name is a property the string must be the name of the property,
    and \a var1, ... \a var8 are ignored.

    The returned QAxObject is a child of this object (which is either of
    type QAxObject or QAxWidget), and is deleted when this object is
    deleted. It is however safe to delete the returned object yourself,
    and you should do so when you iterate over lists of subobjects.

    COM enabled applications usually have an object model publishing
    certain elements of the application as dispatch interfaces. Use
    this method to navigate the hierarchy of the object model, e.g.

    \code
    QAxWidget outlook("Outlook.Application");
    QAxObject *session = outlook.querySubObject("Session");
    if (session) {
	QAxObject *defFolder = session->querySubObject(
				"GetDefaultFolder(OlDefaultFolders)",
				"olFolderContacts");
	//...
    }
    \endcode
*/
QAxObject *QAxBase::querySubObject(const QString &name, 
                                   const QVariant &var1,
                                   const QVariant &var2,
                                   const QVariant &var3,
                                   const QVariant &var4,
                                   const QVariant &var5,
                                   const QVariant &var6,
                                   const QVariant &var7,
                                   const QVariant &var8)
{
    QList<QVariant> vars;
    QVariant var = var1;
    int argc = 1;
    while(var.isValid()) {
        vars << var;
        switch(++argc) {
        case 2: var = var2; break;
        case 3: var = var3; break;
        case 4: var = var4; break;
        case 5: var = var5; break;
        case 6: var = var6; break;
        case 7: var = var7; break;
        case 8: var = var8; break;
        default:var = QVariant(); break;
        }
    }
    
    return querySubObject(name, vars);
}

/*!
    \overload

    The QVariant objects in \a vars are updated when the method has
    out-parameters.
*/
QAxObject *QAxBase::querySubObject(const QString &name, QList<QVariant> &vars)
{
    QAxObject *object = 0;
    VARIANTARG res;
    
    QByteArray rettype;
    if (!dynamicCallHelper(QMetaObject::normalizedSignature(name.latin1()), &res, vars, rettype))
        return 0;
    
    switch (res.vt) {
    case VT_DISPATCH:
        if (res.pdispVal) {
            object = new QAxObject(res.pdispVal, qObject(), (QString(qObject()->objectName()) + "/" + name).latin1());
            ((QAxBase*)object)->d->tryCache = true;
        }
        break;
    case VT_UNKNOWN:
        if (res.punkVal) {
            object = new QAxObject(res.punkVal, qObject(), (QString(qObject()->objectName()) + "/" + name).latin1());
            ((QAxBase*)object)->d->tryCache = true;
        }
        break;
    case VT_EMPTY:
#ifdef QT_CHECK_STATE
        {
            const char *coclass = metaObject()->classInfo(metaObject()->indexOfClassInfo("CoClass")).value();
            qWarning("QAxBase::querySubObject: %s: error calling function or property in %s (%s)"
                , name.latin1(), control().latin1(), coclass ? coclass: "unknown");
        }
#endif
        break;
    default:
#ifdef QT_CHECK_STATE
        {
            const char *coclass = metaObject()->classInfo(metaObject()->indexOfClassInfo("CoClass")).value();
            qWarning("QAxBase::querySubObject: %s: method or property is not of interface type in %s (%s)"
                , name.latin1(), control().latin1(), coclass ? coclass: "unknown");
        }
#endif
        break;
    }
    
    clearVARIANT(&res);
    return object;
}

class QtPropertyBag : public IPropertyBag
{
public:
    QtPropertyBag() :ref(0) {}
    virtual ~QtPropertyBag() {}
    
    HRESULT __stdcall QueryInterface(REFIID iid, LPVOID *iface)
    {
        *iface = 0;
        if (iid == IID_IUnknown)
            *iface = this;
        else if (iid == IID_IPropertyBag)
            *iface = this;
        else
            return E_NOINTERFACE;
        
        AddRef();
        return S_OK;
    }
    unsigned long __stdcall AddRef() { return ++ref; }
    unsigned long __stdcall Release()
    {
        if (!--ref) {
            delete this;
            return 0;
        }
        return ref;
    }
    
    HRESULT __stdcall Read(LPCOLESTR name, VARIANT *var, IErrorLog *)
    {
        if (!var)
            return E_POINTER;
        
        QString property = BSTRToQString((TCHAR*)name).local8Bit();
        QVariant qvar = map.value(property);
        QVariantToVARIANT(qvar, *var, qvar.typeName());
        return S_OK;
    }
    HRESULT __stdcall Write(LPCOLESTR name, VARIANT *var)
    {
        if (!var)
            return E_POINTER;
        QString property = BSTRToQString((TCHAR*)name).local8Bit();
        QVariant qvar = VARIANTToQVariant(*var, 0);
        map[property] = qvar;
        
        return S_OK;
    }
    
    QAxBase::PropertyBag map;
    
private:
    unsigned long ref;
};

/*!
    Returns a name:value map of all the properties exposed by the COM
    object.

    This is more efficient than getting multiple properties
    individually if the COM object supports property bags.

    \warning It is not guaranteed that the property bag implementation
    of the COM object returns all properties, or that the properties
    returned are the same as those available through the IDispatch
    interface.
*/
QAxBase::PropertyBag QAxBase::propertyBag() const
{
    PropertyBag result;
    
    if (!d->ptr && !d->initialized) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }
    
    if (isNull())
        return result;
    IPersistPropertyBag *persist = 0;
    d->ptr->QueryInterface(IID_IPersistPropertyBag, (void**)&persist);
    if (persist) {
        QtPropertyBag *pbag = new QtPropertyBag();
        pbag->AddRef();
        persist->Save(pbag, false, true);
        result = pbag->map;
        pbag->Release();
        persist->Release();
        return result;
    } else {
        const QMetaObject *mo = metaObject();
        for (int p = mo->propertyOffset(); p < mo->propertyCount(); ++p) {
            const QMetaProperty property = mo->property(p);
            QVariant var = qObject()->property(property.name());
            result.insert(property.name(), var);
        }
    }
    return result;
}

/*!
    Sets the properties of the COM object to the corresponding values
    in \a bag.

    \warning
    You should only set property bags that have been returned by the
    propertyBag function, as it cannot be guaranteed that the property
    bag implementation of the COM object supports the same properties
    that are available through the IDispatch interface.

    \sa propertyBag()
*/
void QAxBase::setPropertyBag(const PropertyBag &bag)
{
    if (!d->ptr && !d->initialized) {
        initialize(&d->ptr);
        d->initialized = true;
    }
    
    if (isNull())
        return;
    IPersistPropertyBag *persist = 0;
    d->ptr->QueryInterface(IID_IPersistPropertyBag, (void**)&persist);
    if (persist) {
        QtPropertyBag *pbag = new QtPropertyBag();
        pbag->map = bag;
        pbag->AddRef();
        persist->Load(pbag, 0);
        pbag->Release();
        persist->Release();
    } else {
        const QMetaObject *mo = metaObject();
        for (int p = mo->propertyOffset(); p < mo->propertyCount(); ++p) {
            const QMetaProperty property = mo->property(p);
            QVariant var = bag.value(property.name());
            qObject()->setProperty(property.name(), var);
        }
    }
}

/*!
    Returns true if the property \a prop is writable; otherwise
    returns false. By default, all properties are writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa setPropertyWritable(), propertyChanged()
*/
bool QAxBase::propertyWritable(const char *prop) const
{
    return d->propWritable.value(prop, true);
}

/*!
    Sets the property \a prop to writable if \a ok is true, otherwise
    sets \a prop to be read-only. By default, all properties are
    writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa propertyWritable(), propertyChanged()
*/
void QAxBase::setPropertyWritable(const char *prop, bool ok)
{
    d->propWritable[prop] = ok;
}

/*!
    Returns true if there is no COM object loaded by this wrapper;
    otherwise return false.

    \sa control
*/
bool QAxBase::isNull() const
{
    return !d->ptr;
}

/*!
    Returns a QVariant that wraps the COM object. The variant can
    then be used as a parameter in e.g. dynamicCall().
*/
QVariant QAxBase::asVariant() const
{
    if (!d->ptr && !d->initialized) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }
    
    QVariant qvar;
    if (d->dispatch())
        qVariantSet(qvar, d->dispatch(), "IDispatch*");
    else if (d->ptr)
        qVariantSet(qvar, d->ptr, "IUnknown*");
    
    return qvar;
}

/*!
    \fn QObject *QAxBase::qObject()
    \internal
*/

/*!
    \fn void QAxBase::signal(const QString &name, int argc, void *argv)

    This generic signal gets emitted when the COM object issues the
    event \a name. \a argc is the number of parameters provided by the
    event (DISPPARAMS.cArgs), and \a argv is the pointer to the
    parameter values (DISPPARAMS.rgvarg). Note that the order of parameter
    values is turned around, ie. the last element of the array is the first
    parameter in the function.

    \code
    void Receiver::slot(const QString &name, int argc, void *argv)
    {
	VARIANTARG *params = (VARIANTARG*)argv;
	if (name.startsWith("BeforeNavigate2(")) {
	    IDispatch *pDisp = params[argc-1].pdispVal;
	    VARIANTARG URL = *params[argc-2].pvarVal;
	    VARIANTARG Flags = *params[argc-3].pvarVal;
	    VARIANTARG TargetFrameName = *params[argc-4].pvarVal;
	    VARIANTARG PostData = *params[argc-5].pvarVal;
	    VARIANTARG Headers = *params[argc-6].pvarVal;
	    bool *Cancel = params[argc-7].pboolVal;
	}
    }
    \endcode

    Use this signal if the event has parameters of unsupported data
    types. Otherwise, connect directly to the signal \a name.
*/

/*!
    \fn void QAxBase::propertyChanged(const QString &name)

    If the COM object supports property notification, this signal gets
    emitted when the property called \a name is changed.
*/

/*!
    \fn void QAxBase::exception(int code, const QString &source, const QString &desc, const QString &help)

    This signal is emitted when the COM object throws an exception while called using the OLE automation
    interface IDispatch. \a code, \a source, \a desc and \a help provide information about the exception as
    provided by the COM server and can be used to provide useful feedback to the end user. \a help includes
    the help file, and the help context ID in brackets, e.g. "filename [id]".
*/

#endif //QAXPRIVATE_DECL

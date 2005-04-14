/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaxbindable.h"
#include "qaxfactory.h"

#include <qapplication.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qvariant.h>
#include <qtextstream.h>

#include <qt_windows.h>
#include <olectl.h>

#define Q_REQUIRED_RPCNDR_H_VERSION 475

// Some global variables to store module information
bool qAxIsServer = false;
HANDLE qAxInstance = 0;
ITypeLib *qAxTypeLibrary = 0;
char qAxModuleFilename[MAX_PATH];
bool qAxOutProcServer = false;

// The QAxFactory instance
static QAxFactory* qax_factory = 0;
extern CLSID CLSID_QRect;
extern CLSID CLSID_QSize;
extern CLSID CLSID_QPoint;
extern void qax_shutDown();
extern bool qax_ownQApp;

QAxFactory *qAxFactory()
{
    if (!qax_factory) {
        extern QAxFactory *qax_instantiate();
        bool hadQApp = qApp != 0;
        qax_factory = qax_instantiate();
        // QAxFactory created a QApplication
        if (!hadQApp && qApp)
            qax_ownQApp = true;

        // register all types with metatype system as pointers
        QStringList keys(qax_factory->featureList());
        for (int i = 0; i < keys.count(); ++i) {
            QString key(keys.at(i));
            qRegisterMetaType((key + "*").toLatin1(), (void**)0);
        }
    }
    return qax_factory;
}

// Some local variables to handle module lifetime
static unsigned long qAxModuleRef = 0;
static CRITICAL_SECTION qAxModuleSection;


/////////////////////////////////////////////////////////////////////////////
// Server control
/////////////////////////////////////////////////////////////////////////////

static int initCount = 0;

QString qAxInit()
{
    static QString libFile;

    if (initCount++)
        return libFile;
    
    InitializeCriticalSection(&qAxModuleSection);
    
    libFile = qAxModuleFilename;
    libFile = libFile.toLower();
    if (LoadTypeLibEx((TCHAR*)libFile.utf16(), REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    int lastDot = libFile.lastIndexOf('.');
    libFile = libFile.left(lastDot) + ".tlb";
    if (LoadTypeLibEx((TCHAR*)libFile.utf16(), REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    lastDot = libFile.lastIndexOf('.');
    libFile = libFile.left(lastDot) + ".olb";
    if (LoadTypeLibEx((TCHAR*)libFile.utf16(), REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    libFile = QString();
    return libFile;
}

void qAxCleanup()
{
    if (!initCount)
        qWarning("qAxInit/Cleanup mismatch.");
    
    if (--initCount)
        return;

    delete qax_factory;
    qax_factory = 0;
    
    if (qAxTypeLibrary) {
        qAxTypeLibrary->Release();
        qAxTypeLibrary = 0;
    }
    
    DeleteCriticalSection(&qAxModuleSection);
}

unsigned long qAxLock()
{
    EnterCriticalSection(&qAxModuleSection);
    unsigned long ref = ++qAxModuleRef;
    LeaveCriticalSection(&qAxModuleSection);
    return ref;
}

unsigned long qAxUnlock()
{
    if (!initCount) // cleaned up already
        return 0;

    EnterCriticalSection(&qAxModuleSection);
    unsigned long ref = --qAxModuleRef;
    LeaveCriticalSection(&qAxModuleSection);
    
    if (!ref)
        qax_shutDown();
    return ref;
}

unsigned long qAxLockCount()
{
    return qAxModuleRef;
}

/////////////////////////////////////////////////////////////////////////////
// Registry
/////////////////////////////////////////////////////////////////////////////

extern bool qax_disable_inplaceframe;

// (Un)Register the ActiveX server in the registry.
// The QAxFactory implementation provides the information.
HRESULT UpdateRegistry(BOOL bRegister)
{
    QString file = QString::fromLocal8Bit(qAxModuleFilename);
    QString path = file.left(file.lastIndexOf("\\")+1);
    QString module = file.right(file.length() - path.length());
    module = module.left(module.lastIndexOf("."));
    
    const QString appId = qAxFactory()->appID().toString().toUpper();
    const QString libId = qAxFactory()->typeLibID().toString().toUpper();
    
    QString libFile = qAxInit();
    QString typeLibVersion;

    TLIBATTR *libAttr = 0;
    if (qAxTypeLibrary)
        qAxTypeLibrary->GetLibAttr(&libAttr);
    if (!libAttr)
        return SELFREG_E_TYPELIB;

    DWORD major = libAttr->wMajorVerNum;
    DWORD minor = libAttr->wMinorVerNum;
    typeLibVersion = QString::number((uint)major) + "." + QString::number((uint)minor);

    if (bRegister)
        RegisterTypeLib(qAxTypeLibrary, (TCHAR*)libFile.utf16(), 0);
    else
        UnRegisterTypeLib(libAttr->guid, libAttr->wMajorVerNum, libAttr->wMinorVerNum, libAttr->lcid, libAttr->syskind);

    qAxTypeLibrary->ReleaseTLibAttr(libAttr);

    if (typeLibVersion.isEmpty())
        typeLibVersion = "1.0";
    
    QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Classes", QSettings::NativeFormat);
    
    // we try to create the ActiveX widgets later on...
    bool delete_qApp = false;
    if (!qApp) {
        int argc = 0;
        (void)new QApplication(argc, 0);
        delete_qApp = true;
    }
    
    if (bRegister) {
        if (qAxOutProcServer) {
            settings.setValue("/AppID/" + appId + "/.", module);
            settings.setValue("/AppID/" + module + ".EXE/AppID", appId);
        }

        QStringList keys = qAxFactory()->featureList();
        for (QStringList::Iterator key = keys.begin(); key != keys.end(); ++key) {
            const QString className = *key;
            QObject *object = qAxFactory()->createObject(className);

            const QMetaObject *mo = qAxFactory()->metaObject(className);
            const QString classId = qAxFactory()->classID(className).toString().toUpper();

            if (object) { // don't register subobject classes
                QString classVersion = mo ? QString(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString::null;
                if (classVersion.isNull())
                    classVersion = "1.0";
                bool insertable = mo && QString(mo->classInfo(mo->indexOfClassInfo("Insertable")).value()) == "yes";
                bool control = object->isWidgetType();
                const QString classMajorVersion = classVersion.left(classVersion.indexOf("."));
                uint olemisc = OLEMISC_SETCLIENTSITEFIRST
                    |OLEMISC_ACTIVATEWHENVISIBLE
                    |OLEMISC_INSIDEOUT
                    |OLEMISC_CANTLINKINSIDE
                    |OLEMISC_RECOMPOSEONRESIZE;
                if (!control)
                    olemisc |= OLEMISC_INVISIBLEATRUNTIME;
                else if (qFindChild<QMenuBar*>(object) && !qax_disable_inplaceframe)
                    olemisc |= OLEMISC_WANTSTOMENUMERGE;
                
                settings.setValue("/" + module + "." + className + "." + classMajorVersion + "/.", className + " Class");
                settings.setValue("/" + module + "." + className + "." + classMajorVersion + "/CLSID/.", classId);
                if (insertable)
                    settings.setValue("/" + module + "." + className + "." + classMajorVersion + "/Insertable/.", QVariant());
                
                settings.setValue("/" + module + "." + className + "/.", className + " Class");
                settings.setValue("/" + module + "." + className + "/CLSID/.", classId);
                settings.setValue("/" + module + "." + className + "/CurVer/.", module + "." + className + "." + classMajorVersion);
                
                settings.setValue("/CLSID/" + classId + "/.", className + " Class");
                if (file.right(3).toLower() == "exe")
                    settings.setValue("/CLSID/" + classId + "/AppID", appId);
                if (control)
                    settings.setValue("/CLSID/" + classId + "/Control/.", QVariant());
                if (insertable)
                    settings.setValue("/CLSID/" + classId + "/Insertable/.", QVariant());
                if (file.right(3).toLower() == "dll")
                    settings.setValue("/CLSID/" + classId + "/InProcServer32/.", file);
                else
                    settings.setValue("/CLSID/" + classId + "/LocalServer32/.", file + " -activex");
                settings.setValue("/CLSID/" + classId + "/MiscStatus/.", control ? "1" : "0");
                settings.setValue("/CLSID/" + classId + "/MiscStatus/1/.", QString::number(olemisc));
                settings.setValue("/CLSID/" + classId + "/Programmable/.", QVariant());
                settings.setValue("/CLSID/" + classId + "/ToolboxBitmap32/.", file + ", 101");
                settings.setValue("/CLSID/" + classId + "/TypeLib/.", libId);
                settings.setValue("/CLSID/" + classId + "/Version/.", classVersion);
                settings.setValue("/CLSID/" + classId + "/VersionIndependentProgID/.", module + "." + className);
                settings.setValue("/CLSID/" + classId + "/ProgID/.", module + "." + className + "." + classVersion.left(classVersion.indexOf('.')));

                delete object;
            }

            qAxFactory()->registerClass(className, &settings);
        }
    } else {
        QStringList keys = qAxFactory()->featureList();
        for (QStringList::Iterator key = keys.begin(); key != keys.end(); ++key) {
            const QString className = *key;
            const QMetaObject *mo = qAxFactory()->metaObject(className);
            const QString classId = qAxFactory()->classID(className).toString().toUpper();
            
            QString classVersion = mo ? QString(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString::null;
            if (classVersion.isNull())
                classVersion = "1.0";
            const QString classMajorVersion = classVersion.left(classVersion.indexOf("."));
            
            qAxFactory()->unregisterClass(className, &settings);
            
            settings.remove("/" + module + "." + className + "." + classMajorVersion + "/CLSID/.");
            settings.remove("/" + module + "." + className + "." + classMajorVersion + "/Insertable/.");
            settings.remove("/" + module + "." + className + "." + classMajorVersion + "/.");
            settings.remove("/" + module + "." + className + "." + classMajorVersion);
            
            settings.remove("/" + module + "." + className + "/CLSID/.");
            settings.remove("/" + module + "." + className + "/CurVer/.");
            settings.remove("/" + module + "." + className + "/.");
            settings.remove("/" + module + "." + className);
            
            settings.remove("/CLSID/" + classId + "/AppID");
            settings.remove("/CLSID/" + classId + "/Control/.");
            settings.remove("/CLSID/" + classId + "/Insertable/.");
            settings.remove("/CLSID/" + classId + "/InProcServer32/.");
            settings.remove("/CLSID/" + classId + "/LocalServer32/.");
            settings.remove("/CLSID/" + classId + "/MiscStatus/1/.");
            settings.remove("/CLSID/" + classId + "/MiscStatus/.");	    
            settings.remove("/CLSID/" + classId + "/Programmable/.");
            settings.remove("/CLSID/" + classId + "/ToolboxBitmap32/.");
            settings.remove("/CLSID/" + classId + "/TypeLib/.");
            settings.remove("/CLSID/" + classId + "/Version/.");
            settings.remove("/CLSID/" + classId + "/VersionIndependentProgID/.");
            settings.remove("/CLSID/" + classId + "/ProgID/.");
            settings.remove("/CLSID/" + classId + "/.");
            settings.remove("/CLSID/" + classId);
        }
    }
    
    if (delete_qApp)
        delete qApp;

    qAxCleanup();
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// IDL generator
/////////////////////////////////////////////////////////////////////////////

static QList<QByteArray> enums;
static QList<QByteArray> enumValues;
static QList<QByteArray> subtypes;

static const char* const type_map[][2] =
{
    // QVariant/Qt Value data types
    { "QString",	"BSTR" },
    { "QCString",	"BSTR" },
    { "bool",		"VARIANT_BOOL" },
    { "int",		"int" },
    { "uint",		"unsigned int" },
    { "double",		"double" }, 
    { "QColor",		"OLE_COLOR" },
    { "QDate",		"DATE" },
    { "QTime",		"DATE" },
    { "QDateTime",	"DATE" },
    { "QFont",		"IFontDisp*" },
    { "QPixmap",	"IPictureDisp*" },
    { "QVariant",	"VARIANT" },
    { "QVariantList",	"SAFEARRAY(VARIANT)" },
    { "quint64",	"CY" },
    { "qint64",	        "CY" },
    { "qulonglong",	"CY" },
    { "qlonglong",	"CY" },
    { "QByteArray",	"SAFEARRAY(BYTE)" },
    { "QStringList",	"SAFEARRAY(BSTR)" },
    // Userdefined Qt datatypes - some not on Borland though
    { "QCursor",        "enum MousePointer" },
    { "Qt::FocusPolicy","enum FocusPolicy" },
#ifndef Q_CC_BOR
# if __REQUIRED_RPCNDR_H_VERSION__ >= Q_REQUIRED_RPCNDR_H_VERSION
    { "QRect",		"struct QRect" },
    { "QSize",		"struct QSize" },
    { "QPoint",		"struct QPoint" },
# endif
#endif
    // And we support COM data types
    { "BOOL",		"BOOL" },
    { "BSTR",		"BSTR" },
    { "OLE_COLOR",	"OLE_COLOR" },
    { "DATE",		"DATE" },
    { "VARIANT",	"VARIANT" },
    { "IDispatch",	"IDispatch*" },
    { "IUnknown",	"IUnknown*" },
    { "IDispatch*",	"IDispatch*" },
    { "IUnknown*",	"IUnknown*" },
    { 0,		0 }
};

static QByteArray convertTypes(const QByteArray &qtype, bool *ok)
{
    qRegisterMetaType("IDispatch*", (void**)0);
    qRegisterMetaType("IUnknown*", (void**)0);

    *ok = false;
    
    int i = 0;
    while (type_map[i][0]) {
        if (qtype == type_map[i][0] && type_map[i][1]) {
            *ok = true;
            return type_map[i][1];	    
        }
        ++i;
    }
    if (enums.contains(qtype)) {
        *ok = true;
        return "enum " + qtype;
    }
    if (subtypes.contains(qtype)) {
        *ok = true;
    }
    return qtype;
}

static const char* const keyword_map[][2] =
{
    { "aggregatable",	"aggregating"	    },
    { "allocate",	"alloc"		    },
    { "appobject",	"appObject"	    },
    { "arrays",		"array"		    },
    { "async",		"asynchronous"	    },
    { "bindable",	"binding"	    },
    { "Boolean",	"boolval"	    },
    { "boolean",	"boolval"	    },
    { "broadcast",	"broadCast"	    },
    { "callback",	"callBack"	    },
    { "decode",		"deCode"	    },
    { "default",	"defaulted"	    },
    { "defaultbind",	"defaultBind"	    },
    { "defaultvalue",	"defaultValue"	    },
    { "encode"		"enCode"	    },
    { "endpoint",	"endPoint"	    },
    { "hidden",		"isHidden"	    },
    { "ignore",		"ignore_"	    },
    { "local",		"local_"	    },
    { "notify",		"notify_"	    },
    { "object",		"object_"	    },
    { "optimize",	"optimize_"	    },
    { "optional",	"optional_"	    },
    { "out",		"out_"		    },
    { "pipe",		"pipe_"		    },
    { "proxy",		"proxy_"	    },
    { "ptr",		"pointer"	    },
    { "readonly",	"readOnly"	    },
    { "small",		"small_"	    },
    { "source",		"source_"	    },
    { "string",		"string_"	    },
    { "uuid",		"uuid_"		    },
    { 0,		0		    }
};

static QByteArray replaceKeyword(const QByteArray &name)
{
    int i = 0;
    while (keyword_map[i][0]) {
        if (name == keyword_map[i][0] && keyword_map[i][1])
            return keyword_map[i][1];
        ++i;
    }
    return name;
}

static QMap<QByteArray, int> mapping;

static QByteArray renameOverloads(const QByteArray &name)
{
    QByteArray newName = name;
    
    int n = mapping.value(name);
    if (mapping.contains(name)) {
        int n = mapping.value(name);
        newName = name + "_" + QByteArray::number(n);
        mapping.insert(name, n+1);
    } else {
        mapping.insert(name, 1);
    }
    
    return newName;
}

// filter out some properties
static const char* const ignore_props[] =
{
    "name",
    "objectName",
    "isTopLevel",
    "isDialog",
    "isModal",
    "isPopup",
    "isDesktop",
    "geometry",
    "pos",
    "frameSize",
    "frameGeometry",
    "size",
    "sizeHint",
    "minimumSizeHint",
    "microFocusHint",
    "rect",
    "childrenRect",
    "childrenRegion",
    "minimumSize",
    "maximumSize",
    "sizeIncrement",
    "baseSize",
    "ownPalette",
    "ownFont",
    "ownCursor",
    "visibleRect",
    "isActiveWindow",
    "underMouse",
    "visible",
    "hidden",
    "minimized",
    "focus",
    "focusEnabled",
    "customWhatsThis",
    "shown",
    "windowOpacity",
    0
};

// filter out some slots
static const char* const ignore_slots[] =
{
    "deleteLater",
    "setMouseTracking",
    "update",
    "repaint",
    "iconify",
    "showMinimized",
    "showMaximized",
    "showFullScreen",
    "showNormal",
    "polish",
    "constPolish",
    "stackUnder",
    "setShown",
    "setHidden",
    "move_1",
    "resize_1",
    "setGeometry_1",
    0
};

static bool ignore(const char *test, const char *const *table)
{
    if (!test)
        return true;
    int i = 0;
    while (table[i]) {
        if (!strcmp(test, table[i]))
            return true;
        ++i;
    }
    return false;
}

bool ignoreSlots(const char *test)
{
    return ignore(test, ignore_slots);
}

bool ignoreProps(const char *test)
{
    return ignore(test, ignore_props);
}

#define STRIPCB(x) x = x.mid(1, x.length()-2)

static QByteArray prototype(const QList<QByteArray> &parameterTypes, const QList<QByteArray> &parameterNames, bool *ok)
{
    QByteArray prototype;
    
    for (int p = 0; p < parameterTypes.count() && *ok; ++p) {
        bool out = false;
        QByteArray type(parameterTypes.at(p));
        QByteArray name(parameterNames.at(p));
        
        if (type.endsWith("&")) {
            out = true;
            type.truncate(type.length() - 1);
        } else if (type.endsWith("**")) {
            out = true;
            type.truncate(type.length() - 1);
        } else if (type.endsWith("*") && !subtypes.contains(type)) {
            type.truncate(type.length() - 1);
        }
        if (type.isEmpty()) {
            ok = false;
            break;
        }
        type = convertTypes(type, ok);
        if (!out)
            prototype += "[in] " + type + " ";
        else
            prototype += "[in,out] " + type + " ";
        
        if (out)
            prototype += "*";
        if (name.isEmpty())
            prototype += "p" + QByteArray::number(p);
        else
            prototype += "p_" + replaceKeyword(name);
        
        if (p < parameterTypes.count() - 1)
            prototype += ", ";
    }
    
    return prototype;
}

static QByteArray addDefaultArguments(const QByteArray &prototype, int numDefArgs)
{
    // nothing to do, or unsupported anyway
    if (!numDefArgs || prototype.contains("/**"))
        return prototype;

    QByteArray ptype(prototype);

    int in = -1;
    while (numDefArgs) {
        in = ptype.lastIndexOf("]", in);
        ptype.replace(in, 1, ",optional]");
        in = ptype.indexOf(' ', in) + 1;
        QByteArray type = ptype.mid(in, ptype.indexOf(' ', in) - in);
        ptype.replace(in, type.length(), "VARIANT");
        --numDefArgs;
    }

    return ptype;
}

static HRESULT classIDL(QObject *o, const QMetaObject *mo, const QString &className, bool isBindable, QTextStream &out)
{
    int id = 1;
    int i = 0;
    if (!mo)
        return 3;
    
    QString topclass = qAxFactory()->exposeToSuperClass(className);
    if (topclass.isEmpty())
        topclass = "Qt";
    bool hasStockEvents = qAxFactory()->hasStockEvents(className);
    
    const QMetaObject *pmo = mo;
    do {
        pmo = pmo->superClass();
    } while (pmo && topclass != pmo->className());
    
    int enumoff = pmo ? pmo->enumeratorOffset() : mo->enumeratorOffset();
    int memberoff = pmo ? pmo->memberOffset() : mo->memberOffset();
    int propoff = pmo ? pmo->propertyOffset() : mo->propertyOffset();
    
    int qtProps = 0;
    int qtSlots = 0;

    bool control = false;
    
    if (o && o->isWidgetType()) {
        qtProps = QWidget::staticMetaObject.propertyCount();
        qtSlots = QWidget::staticMetaObject.propertyCount();
        control = true;
    }
    
    QString classID = qAxFactory()->classID(className).toString().toUpper();
    if (QUuid(classID).isNull())
        return 4;
    STRIPCB(classID);
    QString interfaceID = qAxFactory()->interfaceID(className).toString().toUpper();
    if (QUuid(interfaceID).isNull())
        return 5;
    STRIPCB(interfaceID);
    QString eventsID = qAxFactory()->eventsID(className).toString().toUpper();
    bool hasEvents = !QUuid(eventsID).isNull();
    STRIPCB(eventsID);
    
    QString defProp(mo->classInfo(mo->indexOfClassInfo("DefaultProperty")).value());
    QString defSignal(mo->classInfo(mo->indexOfClassInfo("DefaultSignal")).value());
    
    for (i = enumoff; i < mo->enumeratorCount(); ++i) {
        const QMetaEnum enumerator = mo->enumerator(i);
        if (enums.contains(enumerator.name()))
            continue;
        
        enums.append(enumerator.name());
        
        out << "\tenum " << enumerator.name() << " {" << endl;
        
        for (int j = 0; j < enumerator.keyCount(); ++j) {
            QByteArray key(enumerator.key(j));
            while (enumValues.contains(key)) {
                key += "_";
            }
            enumValues.append(key);
            uint value = (uint)enumerator.value(j);
            key = key.leftJustified(20);
            out << "\t\t" << key << "\t= ";
            if (enumerator.isFlag())
                out << "0x" << QByteArray::number(value, 16).rightJustified(8, '0');
            else
                out << value;
            if (j < enumerator.keyCount()-1)
                out << ", ";
            out << endl;
        }
        out << "\t};" << endl << endl;
    }

    // mouse cursor enum for QCursor support
    if (!enums.contains("MousePointer")) {
        enums.append("MousePointer");
        out << "\tenum MousePointer {" << endl;
        out << "\t\tArrowCursor             = " << Qt::ArrowCursor << "," << endl;
        out << "\t\tUpArrowCursor           = " << Qt::UpArrowCursor << "," << endl;
        out << "\t\tCrossCursor             = " << Qt::CrossCursor << "," << endl;
        out << "\t\tWaitCursor              = " << Qt::WaitCursor << "," << endl;
        out << "\t\tIBeamCursor             = " << Qt::IBeamCursor << "," << endl;
        out << "\t\tSizeVerCursor           = " << Qt::SizeVerCursor << "," << endl;
        out << "\t\tSizeHorCursor           = " << Qt::SizeHorCursor << "," << endl;
        out << "\t\tSizeBDiagCursor         = " << Qt::SizeBDiagCursor << "," << endl;
        out << "\t\tSizeFDiagCursor         = " << Qt::SizeFDiagCursor << "," << endl;
        out << "\t\tSizeAllCursor           = " << Qt::SizeAllCursor << "," << endl;
        out << "\t\tBlankCursor             = " << Qt::BlankCursor << "," << endl;
        out << "\t\tSplitVCursor            = " << Qt::SplitVCursor << "," << endl;
        out << "\t\tSplitHCursor            = " << Qt::SplitHCursor << "," << endl;
        out << "\t\tPointingHandCursor      = " << Qt::PointingHandCursor << "," << endl;
        out << "\t\tForbiddenCursor         = " << Qt::ForbiddenCursor << "," << endl;
        out << "\t\tWhatsThisCursor         = " << Qt::WhatsThisCursor << "," << endl;
        out << "\t\tBusyCursor\t= " << Qt::BusyCursor << endl;
        out << "\t};" << endl << endl;
    }
    
    out << endl;
    out << "\t[" << endl;
    out << "\t\tuuid(" << interfaceID << ")," << endl;
    out << "\t\thelpstring(\"" << className << " Interface\")" << endl;
    out << "\t]" << endl;
    out << "\tdispinterface I" << className  << endl;
    out << "\t{" << endl;
    
    out << "\tproperties:" << endl;
    for (i = propoff; i < mo->propertyCount(); ++i) {
        const QMetaProperty property = mo->property(i);
        /*	if (property.testFlags(QMetaProperty::Override))
        continue;*/
        if (i <= qtProps && ignoreProps(property.name()))
            continue;
        if (!property.name() || mo->indexOfProperty(property.name()) > i)
            continue;
        
        bool ok = true;
        QByteArray type(convertTypes(property.typeName(), &ok));
        QByteArray name(replaceKeyword(property.name()));
        
        if (!ok)
            out << "\t/****** Property is of unsupported datatype" << endl;
        
        out << "\t\t[id(" << id << ")";
        if (!property.isWritable())
            out << ", readonly";
        if (isBindable && property.isScriptable(o))
            out << ", bindable";
        if (!property.isDesignable(o))
            out << ", nonbrowsable";
        if (isBindable)
            out << ", requestedit";
        if (defProp == name)
            out << ", uidefault";
        out << "] " << type << " " << name << ";" << endl;
        
        if (!ok)
            out << "\t******/" << endl;
        ++id;
    }
    out << endl;
    out << "\tmethods:" << endl;
    int numDefArgs = 0;
    QByteArray outBuffer;
    for (i = memberoff; i < mo->memberCount(); ++i) {
        const QMetaMember slot = mo->member(i);
        if (slot.access() != QMetaMember::Public || slot.memberType() == QMetaMember::Signal)
            continue;

        if (slot.attributes() & QMetaMember::Cloned) {
            ++numDefArgs;
            continue;
        }
        if (!outBuffer.isEmpty()) {
            outBuffer = addDefaultArguments(outBuffer, numDefArgs);
            numDefArgs = 0;
            out << outBuffer;
            outBuffer = QByteArray();
        }
        
        QByteArray signature(slot.signature());
        QByteArray name(signature.left(signature.indexOf('(')));
        if (i <= qtSlots && ignoreSlots(name))
            continue;

        signature = signature.mid(name.length() + 1);
        signature.truncate(signature.length() - 1);
        name = renameOverloads(replaceKeyword(name));
        if (ignoreSlots(name))
            continue;

        QList<QByteArray> parameterTypes(slot.parameterTypes());
        QList<QByteArray> parameterNames(slot.parameterNames());
        
        bool ok = true;
        QByteArray type = slot.typeName();
        if (type.isEmpty())
            type = "void";
        else
            type = convertTypes(type, &ok);
        
        QByteArray ptype(prototype(parameterTypes, parameterNames, &ok));
        if (!ok)
            outBuffer += "\t/****** Slot parameter uses unsupported datatype\n";
        
        outBuffer += "\t\t[id(" + QString::number(id) + ")] " + type + " " + name + "(" + ptype + ");\n";
        
        if (!ok)
            outBuffer += "\t******/\n";
        ++id;
    }
    if (!outBuffer.isEmpty()) {
        outBuffer = addDefaultArguments(outBuffer, numDefArgs);
        numDefArgs = 0;
        out << outBuffer;
        outBuffer = QByteArray();
    }
    out << "\t};" << endl << endl;

    mapping.clear();
    id = 1;
    
    if (hasEvents) {
        out << "\t[" << endl;
        out << "\t\tuuid(" << eventsID << ")," << endl;
        out << "\t\thelpstring(\"" << className << " Events Interface\")" << endl;
        out << "\t]" << endl;
        out << "\tdispinterface I" << className << "Events" << endl;
        out << "\t{" << endl;
        out << "\tproperties:" << endl;
        out << "\tmethods:" << endl;
        
        if (hasStockEvents) {
            out << "\t/****** Stock events ******/" << endl;
            out << "\t\t[id(DISPID_CLICK)] void Click();" << endl;
            out << "\t\t[id(DISPID_DBLCLICK)] void DblClick();" << endl;
            out << "\t\t[id(DISPID_KEYDOWN)] void KeyDown(short* KeyCode, short Shift);" << endl;
            out << "\t\t[id(DISPID_KEYPRESS)] void KeyPress(short* KeyAscii);" << endl;
            out << "\t\t[id(DISPID_KEYUP)] void KeyUp(short* KeyCode, short Shift);" << endl;
            out << "\t\t[id(DISPID_MOUSEDOWN)] void MouseDown(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl;
            out << "\t\t[id(DISPID_MOUSEMOVE)] void MouseMove(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl;
            out << "\t\t[id(DISPID_MOUSEUP)] void MouseUp(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl << endl;
        }
        
        for (i = memberoff; i < mo->memberCount(); ++i) {
            const QMetaMember signal = mo->member(i);
            if (signal.memberType() != QMetaMember::Signal)
                continue;
            
            QByteArray signature(signal.signature());
            QByteArray name(signature.left(signature.indexOf('(')));
            signature = signature.mid(name.length() + 1);
            signature.truncate(signature.length() - 1);

            QList<QByteArray> parameterTypes(signal.parameterTypes());
            QList<QByteArray> parameterNames(signal.parameterNames());
            
            bool isDefault = defSignal == name;
            name = renameOverloads(replaceKeyword(name));
            bool ok = true;
            
            QByteArray type = signal.typeName();
            if (!type.isEmpty()) // signals with return value not supported
                continue;
            
            QByteArray ptype(prototype(parameterTypes, parameterNames, &ok));
            if (!ok)
                out << "\t/****** Signal parameter uses unsupported datatype" << endl;
            
            out << "\t\t[id(" << id << ")";
            if (isDefault)
                out << ", uidefault";
            out << "] void " << name << "(" << ptype << ");" << endl;
            
            if (!ok)
                out << "\t******/" << endl;
            ++id;
        }
        out << "\t};" << endl << endl;
    }
    
    out << "\t[" << endl;
    
    if (qstricmp(mo->classInfo(mo->indexOfClassInfo("Aggregatable")).value(), "no"))
        out << "\t\taggregatable," << endl;
    if (!qstricmp(mo->classInfo(mo->indexOfClassInfo("RegisterObject")).value(), "yes"))
        out << "\t\tappobject," << endl;
    if (mo->classInfo(mo->indexOfClassInfo("LicenseKey")).value())
        out << "\t\tlicensed," << endl;
    const char *helpString = mo->classInfo(mo->indexOfClassInfo("Description")).value();
    if (helpString)
        out << "\t\thelpstring(\"" << helpString << "\")," << endl;
    else
        out << "\t\thelpstring(\"" << className << " Class\")," << endl;
    const char *classVersion = mo->classInfo(mo->indexOfClassInfo("Version")).value();
    if (classVersion)
        out << "\t\tversion(" << classVersion << ")," << endl;
    out << "\t\tuuid(" << classID << ")";
    if (control) {
        out << ", " << endl;
        out << "\t\tcontrol";
    } else if (!o) {
        out << ", " << endl;
        out << "\t\tnoncreatable";
    }
    out << endl;
    out << "\t]" << endl;
    out << "\tcoclass " << className << endl;
    out << "\t{" << endl;
    out << "\t\t[default] dispinterface I" << className << ";" << endl;
    if (hasEvents)
        out << "\t\t[default, source] dispinterface I" << className << "Events;" << endl;
    out << "\t};" << endl;
    
    return S_OK;
}

#if defined(Q_CC_BOR)
extern "C" __stdcall HRESULT DumpIDL(const QString &outfile, const QString &ver)
#else
extern "C" HRESULT __stdcall DumpIDL(const QString &outfile, const QString &ver)
#endif
{
    qAxIsServer = false;
    QTextStream out;
    if (outfile.contains("\\")) {
        QString outpath = outfile.left(outfile.lastIndexOf("\\"));
        QDir dir;
        dir.mkpath(outpath);
    }
    QFile file(outfile);
    file.remove();
    
    QString filebase = qAxModuleFilename;
    filebase = filebase.left(filebase.lastIndexOf("."));
    
    QString appID = qAxFactory()->appID().toString().toUpper();
    if (QUuid(appID).isNull())
        return 1;
    STRIPCB(appID);
    QString typeLibID = qAxFactory()->typeLibID().toString().toUpper();
    if (QUuid(typeLibID).isNull())
        return 2;
    STRIPCB(typeLibID);
    QString typelib = filebase.right(filebase.length() - filebase.lastIndexOf("\\")-1);
    
    if (!file.open(QIODevice::WriteOnly))
        return -1;
    
    out.setDevice(&file);
    
    QString version(ver.unicode(), ver.length());
    while (version.count('.') > 1) {
        int lastdot = version.lastIndexOf('.');
        version = version.left(lastdot) + version.right(version.length() - lastdot - 1);
    }
    if (version.isEmpty())
        version = "1.0";
    
    QString idQRect(QUuid(CLSID_QRect).toString());
    STRIPCB(idQRect);
    QString idQSize(QUuid(CLSID_QSize).toString());
    STRIPCB(idQSize);
    QString idQPoint(QUuid(CLSID_QPoint).toString());
    STRIPCB(idQPoint);
    
    out << "/****************************************************************************" << endl;
    out << "** Interface definition generated for ActiveQt project" << endl;
    out << "**" << endl;
    out << "**     '" << qAxModuleFilename << "'" << endl;
    out << "**" << endl;
    out << "** Created:  " << QDateTime::currentDateTime().toString() << endl;
    out << "**" << endl;
    out << "** WARNING! All changes made in this file will be lost!" << endl;
    out << "****************************************************************************/" << endl << endl;
    
    out << "import \"ocidl.idl\";" << endl;
    out << "#include <olectl.h>" << endl << endl;
    
    // dummy application to create widgets
    bool delete_qApp = false;
    if (!qApp) {
        int argc;
        (void)new QApplication(argc, 0);
        delete_qApp = true;
    }
    
    out << "[" << endl;
    out << "\tuuid(" << typeLibID << ")," << endl;
    out << "\tversion(" << version << ")," << endl;
    out << "\thelpstring(\"" << typelib << " " << version << " Type Library\")" << endl;
    out << "]" << endl;
    out << "library " << typelib << "Lib" << endl;
    out << "{" << endl;
    out << "\timportlib(\"stdole32.tlb\");" << endl;
    out << "\timportlib(\"stdole2.tlb\");" << endl << endl;
    
    QStringList keys = qAxFactory()->featureList();
    QStringList::ConstIterator key;
    
    out << "\t/************************************************************************" << endl;
    out << "\t** If this causes a compile error in MIDL you need to upgrade the" << endl;
    out << "\t** Platform SDK you are using. Download the SDK from msdn.microsoft.com" << endl;
    out << "\t** and make sure that both the system and the Visual Studio environment" << endl;
    out << "\t** use the correct files." << endl;
    out << "\t**" << endl;
    
#ifndef Q_CC_BOR
#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t** Required version of MIDL could not be verified. QRect, QSize and QPoint" << endl;
    out << "\t** support needs an updated Platform SDK to be installed." << endl;
    out << "\t*************************************************************************" << endl;
#else
    out << "\t************************************************************************/" << endl;
#endif
    
    out << endl;
    out << "\t[uuid(" << idQRect << ")]" << endl;
    out << "\tstruct QRect {" << endl;
    out << "\t\tint left;" << endl;
    out << "\t\tint top;" << endl;
    out << "\t\tint right;" << endl;
    out << "\t\tint bottom;" << endl;
    out << "\t};" << endl << endl;
    
    out << "\t[uuid(" << idQSize << ")]" << endl;
    out << "\tstruct QSize {" << endl;
    out << "\t\tint width;" << endl;
    out << "\t\tint height;" << endl;
    out << "\t};" << endl << endl;
    
    out << "\t[uuid(" << idQPoint << ")]" << endl;
    out << "\tstruct QPoint {" << endl;
    out << "\t\tint x;" << endl;
    out << "\t\tint y;" << endl;
    out << "\t};" << endl;
#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t*/" << endl;
#endif
#else
    out << "\t** Custom data types not supported with Borland." << endl;
    out << "\t*************************************************************************" << endl;
#endif
    out << endl;
    
    out << "\t/* Forward declaration of classes that might be used as parameters */" << endl << endl;
    
    int res = S_OK;
    for (key = keys.begin(); key != keys.end(); ++key) {
        QByteArray className = (*key).toLatin1();
        const QMetaObject *mo = qAxFactory()->metaObject(className);
        // We have meta object information for this type. Forward declare it.
        if (mo) {
            out << "\tcoclass " << className << ";" << endl;
            QObject *o = qAxFactory()->createObject(className);
            subtypes.append(className);
            subtypes.append(className + "*");
            qRegisterMetaType(className, (void**)0);
            qRegisterMetaType(className + "*", (void**)0);
            delete o;
        }
    }
    out << endl;

    for (key = keys.begin(); key != keys.end(); ++key) {
        QByteArray className = (*key).toLatin1();
        const QMetaObject *mo = qAxFactory()->metaObject(className);
        // We have meta object information for this type. Define it.
        if (mo) {
            QObject *o = qAxFactory()->createObject(className);
            // It's not a control class, so it is actually a subtype. Define it.
            if (!o)
                res = classIDL(0, mo, className, false, out);
            delete o;
        }
    }

    out << endl;
    if (res != S_OK)
        goto ErrorInClass;
    
    for (key = keys.begin(); key != keys.end(); ++key) {
        QByteArray className = (*key).toLatin1();
        QObject *o = qAxFactory()->createObject(className);
        if (!o)
            continue;
        QAxBindable *bind = (QAxBindable*)o->qt_metacast("QAxBindable");
        bool isBindable =  bind != 0;
        
        subtypes.append(className);
        subtypes.append(className + "*");
        res = classIDL(o, o->metaObject(), className, isBindable, out);
        delete o;
        if (res != S_OK)
            break;
    }
    
    out << "};" << endl;
    out.flush();
    
ErrorInClass:
    if (delete_qApp)
        delete qApp;

    if (res != S_OK) {
        file.close();
        file.remove();
    }
    
    return res;
}

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

#include <qaxobject.h>
#include <qfile.h>
#include <qmetaobject.h>
#include <qtextstream.h>
#include <qsettings.h>
#include <quuid.h>

#include <qt_windows.h>
#include <ocidl.h>
#include <ctype.h>

#include "../../shared/types.h"

enum ObjectCategory
{
    DefaultObject   = 0x0,
    SubObject       = 0x01,
    ActiveX         = 0x02,
    NoMetaObject    = 0x04,
    NoImplementation = 0x08,
    NoDeclaration   = 0x10,
    NoInlines       = 0x20,
    OnlyInlines     = 0x40,
    DoNothing       = 0x80
};

// this comes from moc/qmetaobject.cpp
enum ProperyFlags  {
    Invalid = 0x00000000,
    Readable = 0x00000001,
    Writable = 0x00000002,
    Resetable = 0x00000004,
    EnumOrFlag = 0x00000008,
    StdCppSet = 0x00000100,
    Override = 0x00000200,
    Designable = 0x00001000,
    ResolveDesignable = 0x00002000,
    Scriptable = 0x00004000,
    ResolveScriptable = 0x00008000,
    Stored = 0x00010000,
    ResolveStored = 0x00020000,
    Editable = 0x00040000,
    ResolveEditable = 0x00080000
};

enum MemberFlags {
    AccessPrivate = 0x00,
    AccessProtected = 0x01,
    AccessPublic = 0x02,
    MemberMethod = 0x00,
    MemberSignal = 0x04,
    MemberSlot = 0x08,
    MemberCompatibility = 0x10,
    MemberCloned = 0x20,
    MemberScriptable = 0x40,
};

extern QMetaObject *qax_readEnumInfo(ITypeLib *typeLib, const QMetaObject *parentObject);
extern QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);
extern QMetaObject *qax_readInterfaceInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);
extern QList<QByteArray> qax_qualified_usertypes;

void writeEnums(QTextStream &out, const QMetaObject *mo)
{
    // enums
    for (int ienum = mo->enumeratorOffset(); ienum < mo->enumeratorCount(); ++ienum) {
        QMetaEnum metaEnum = mo->enumerator(ienum);
        out << "    enum " << metaEnum.name() << " {" << endl;
        for (int k = 0; k < metaEnum.keyCount(); ++k) {
            QByteArray key(metaEnum.key(k));
            out << "        " << key.leftJustified(24) << "= " << metaEnum.value(k);
            if (k < metaEnum.keyCount() - 1)
                out << ",";
            out << endl;
        }
        out << "    };" << endl;
        out << endl;
    }
}

void writeHeader(QTextStream &out, const QByteArray &nameSpace)
{
    out << "#ifndef QAX_DUMPCPP_" << nameSpace.toUpper() << "_H" << endl;
    out << "#define QAX_DUMPCPP_" << nameSpace.toUpper() << "_H" << endl;
    out << endl;
    out << "// Define this symbol to __declspec(dllexport) or __declspec(dllimport)" << endl;
    out << "#ifndef " << nameSpace.toUpper() << "_EXPORT" << endl;
    out << "#define " << nameSpace.toUpper() << "_EXPORT" << endl;
    out << "#endif" << endl;
    out << endl;
    out << "#include <qaxobject.h>" << endl;
    out << "#include <qaxwidget.h>" << endl;
    out << "#include <qdatetime.h>" << endl;
    out << "#include <qpixmap.h>" << endl;
    out << endl;
    out << "struct IDispatch;" << endl;
    out << endl;
}

void generateNameSpace(QTextStream &out, const QMetaObject *mo, const QByteArray &nameSpace)
{
    out << "namespace " << nameSpace << " {" << endl;
    out << endl;
    writeEnums(out, mo);

    // don't close on purpose
}

static QByteArray joinParameterNames(const QList<QByteArray> &parameterNames)
{
    QByteArray slotParameters;
    for (int p = 0; p < parameterNames.count(); ++p) {
        slotParameters += parameterNames.at(p);
        if (p < parameterNames.count() - 1)
            slotParameters += ',';
    }

    return slotParameters;
}

QByteArray constRefify(const QByteArray &type)
{
    QByteArray ctype(type);
    if (type == "QString" || type == "QPixmap" 
        || type == "QVariant" || type == "QDateTime"
        || type == "QColor" || type == "QFont"
        || type == "QByteArray" || type == "QValueList<QVariant>"
        || type == "QStringList")
        ctype = "const " + ctype + "&";

    return ctype;
}

void generateClassDecl(QTextStream &out, const QString &controlID, const QMetaObject *mo, const QByteArray &className, const QByteArray &nameSpace, ObjectCategory category)
{
    QList<QByteArray> functions;

    QByteArray indent;
    if (!(category & OnlyInlines))
        indent = "    ";

    if (!(category & OnlyInlines)) {
        // constructors
        out << "class " << nameSpace.toUpper() << "_EXPORT " << className << " : public ";
        if (category & ActiveX)
            out << "QAxWidget";
        else
            out << "QAxObject";
        out << endl;

        out << "{" << endl;
        out << "public:" << endl;
        out << "    " << className << "(";
        if (category & ActiveX)
            out << "QWidget *parent = 0, Qt::WFlags f";
        else if (category & SubObject)
            out << "IDispatch *subobject, QAxObject *parent";
        else
            out << "QObject *parent";
        out << " = 0)" << endl;
        out << "    : ";
        if (category & ActiveX)
            out << "QAxWidget(parent, f";
        else if (category & SubObject)
            out << "QAxObject((IUnknown*)subobject, parent";
        else
            out << "QAxObject(parent";
        out << ")" << endl;
        out << "    {" << endl;
        if (category & SubObject)
            out << "        internalRelease();" << endl;
        else
            out << "        setControl(\"" << controlID << "\");" << endl;
        out << "    }" << endl;
        out << endl;

        for (int ci = mo->classInfoOffset(); ci < mo->classInfoCount(); ++ci) {
            QMetaClassInfo info = mo->classInfo(ci);
            QByteArray iface_name = info.name();
            if (iface_name.startsWith("Event "))
                continue;

            QByteArray iface_class = info.value();

            out << "    " << className << "(" << iface_class << " *iface)" << endl;
            out << "    : QAxObject()" << endl;
            out << "    {" << endl;
            out << "        initializeFrom(iface);" << endl;
            out << "        delete iface;" << endl;
            out << "    }" << endl;
            out << endl;
        }
    }

    functions << className;

    // enums
    if (nameSpace.isEmpty() && !(category & OnlyInlines)) {
        for (int ienum = mo->enumeratorOffset(); ienum < mo->enumeratorCount(); ++ienum) {
            QMetaEnum metaEnum = mo->enumerator(ienum);
            out << "    enum " << metaEnum.name() << " {" << endl;
            for (int k = 0; k < metaEnum.keyCount(); ++k) {
                QByteArray key(metaEnum.key(k));
                out << "        " << key.leftJustified(24) << "= " << metaEnum.value(k);
                if (k < metaEnum.keyCount() - 1)
                    out << ",";
                out << endl;
            }
            out << "    };" << endl;
            out << endl;
        }
    }

    // properties
    for (int iprop = mo->propertyOffset(); iprop < mo->propertyCount(); ++iprop) {
        QMetaProperty property = mo->property(iprop);
        if (!property.isReadable())
            continue;

        QByteArray propertyName(property.name());
        if (propertyName == "control" || propertyName == className)
            continue;

        QByteArray propertyType(property.typeName());
        QByteArray castType(propertyType);

        QByteArray simplePropType = propertyType;
        simplePropType.replace('*', "");
        out << indent << "inline ";
        bool foreignNamespace = true;
        if (!propertyType.contains("::") && qax_qualified_usertypes.contains(simplePropType)) {
            propertyType = nameSpace + "::" + propertyType;
            foreignNamespace = false;
        }

        out << propertyType << " ";

        if (category & OnlyInlines)
            out << className << "::";
        out << propertyName << "() const";

        if (!(category & NoInlines)) {
            out << endl << indent << "{" << endl;
            if (qax_qualified_usertypes.contains(simplePropType)) {
                out << indent << "    IDispatch *disp_pointer = 0;" << endl;
                out << indent << "    qRegisterMetaType(\"" << property.typeName() << "\", &disp_pointer);" << endl;
            }
            out << indent << "    QVariant qax_result = property(\"" << propertyName << "\");" << endl;
            if (propertyType.at(propertyType.length()-1) == '*')
                out << indent << "    if (!qax_result.constData()) return 0;" << endl;
            out << indent << "    Q_ASSERT(qax_result.isValid());" << endl;
            if (qax_qualified_usertypes.contains(simplePropType)) {
                simplePropType = propertyType;
                simplePropType.replace('*', "");
                if (foreignNamespace) {
                    out << "#ifdef QAX_DUMPCPP_";
                    out << propertyType.left(propertyType.indexOf("::")).toUpper();
                    out << "_H" << endl;                    
                }

                out << indent << "    return new " << simplePropType << "(*(IDispatch**)qax_result.constData());" << endl;
                if (foreignNamespace) {
                    out << "#else" << endl;
                    out << indent << "    return 0; // foreign namespace not included" << endl;
                    out << "#endif" << endl;
                }

            } else {
                out << indent << "    return *(" << propertyType << "*)qax_result.constData();" << endl;
            }
            out << indent << "}" << endl;
        } else {
            out << ";" << endl;
        }

        functions << propertyName;
        
        if (property.isWritable()) {
            QByteArray setter(propertyName);
            QChar firstChar = setter.at(0);
            if (isupper(setter.at(0))) {
                setter = "Set" + setter;
            } else {
                setter[0] = toupper(setter[0]);
                setter = "set" + setter;
            }
            out << indent << "inline " << "void ";
            if (category & OnlyInlines)
                out << className << "::";
            out << setter << "(" << constRefify(propertyType) << " value)";
            
            if (!(category & NoInlines))
                out << "{ setProperty(\"" << propertyName << "\", QVariant(value)); }" << endl;
            else
                out << ";" << endl;

            functions << setter;
        }

        out << endl;
    }

    // slots - but not property setters
    int defaultArguments = 0;
    for (int islot = mo->memberOffset(); islot < mo->memberCount(); ++islot) {
        const QMetaMember slot(mo->member(islot));
        if (slot.memberType() != QMetaMember::Slot)
            continue;

#if 0
        // makes not sense really to respect default arguments...
        if (slot.attributes() & Cloned) {
            ++defaultArguments;
            continue;
        }
#endif

        QByteArray slotSignature(slot.signature());
        if (functions.contains(slotSignature.left(slotSignature.indexOf('('))))
            continue;

        QByteArray slotParameters(joinParameterNames(slot.parameterNames()));
        QByteArray slotTag(slot.tag());
        QByteArray slotType(slot.typeName());

        QByteArray simpleSlotType = slotType;
        simpleSlotType.replace('*', "");
        if (!slotType.contains("::") && qax_qualified_usertypes.contains(simpleSlotType))
            slotType = nameSpace + "::" + slotType;

        QByteArray slotNamedSignature;
        if (slotSignature.endsWith("()")) { // no parameters - no names
            slotNamedSignature = slotSignature;
        } else {
            slotNamedSignature = slotSignature.left(slotSignature.indexOf('(') + 1);
            QByteArray slotSignatureTruncated(slotSignature.mid(slotNamedSignature.length()));
            slotSignatureTruncated.truncate(slotSignatureTruncated.length() - 1);

            QList<QByteArray> signatureSplit = slotSignatureTruncated.split(',');
            QList<QByteArray> parameterSplit;
            if (slotParameters.isEmpty()) { // generate parameter names
                for (int i = 0; i < signatureSplit.count(); ++i)
                    parameterSplit << QByteArray("p") + QByteArray::number(i);
            } else {
                parameterSplit = slotParameters.split(',');
            }
            
            for (int i = 0; i < signatureSplit.count(); ++i) {
                slotNamedSignature += constRefify(signatureSplit.at(i));
                slotNamedSignature += " ";
                slotNamedSignature += parameterSplit.at(i);
                if (defaultArguments >= signatureSplit.count() - i) {
                    slotNamedSignature += " = ";
                    slotNamedSignature += signatureSplit.at(i) + "()";
                }
                if (i + 1 < signatureSplit.count())
                    slotNamedSignature += ", ";
            }
            slotNamedSignature += ')';
        }

        out << indent << "inline ";

        if (!slotTag.isEmpty())
            out << slotTag << " ";
        if (slotType.isEmpty())
            out << "void ";
        else
            out << slotType << " ";
        if (category & OnlyInlines)
            out << className << "::";

        out << slotNamedSignature;

        if (category & NoInlines) {
            out << ";" << endl;
        } else {
            out << endl;
            out << indent << "{" << endl;
            
            if (!slotType.isEmpty()) {
                out << indent << "    " << slotType << " qax_result;" << endl;
            }
            out << indent << "    void *_a[] = {";
            if (!slotType.isEmpty())
                out << "(void*)&qax_result";
            else
                out << "0";
            if (!slotParameters.isEmpty()) {
                out << ", (void*)&";
                out << slotParameters.replace(",", ", (void*)&");
            }
            out << "};" << endl;

            out << indent << "    qt_metacall(QMetaObject::InvokeMetaMember, " << islot << ", _a);" << endl;

            if (!slotType.isEmpty()) {
                if (qax_qualified_usertypes.contains(simpleSlotType)) {
                    simpleSlotType = slotType;
                    simpleSlotType.replace('*', "");
                    out << indent << "    return new " << simpleSlotType << "(qax_result);" << endl;
                } else {
                    out << indent << "    return qax_result;" << endl;
                }
            }
            out << indent << "}" << endl;
        }

        out << endl;
        defaultArguments = 0;
    }

    if (!(category & OnlyInlines)) {
        if (!(category & NoMetaObject)) {
            out << "// meta object functions" << endl;
            out << "    static const QMetaObject staticMetaObject;" << endl;
            out << "    virtual const QMetaObject *metaObject() const { return &staticMetaObject; }" << endl;
            out << "    virtual void *qt_metacast(const char *);" << endl;
        }

        out << "};" << endl;
    }
}

#define addString(string) \
    out << stringDataLength << ", "; \
    stringData += string; \
    stringDataLength += qstrlen(string); \
    stringData += "\\0"; \
    lineLength += qstrlen(string) + 1; \
    if (lineLength > 200) { stringData += "\"\n    \""; lineLength = 0; } \
    ++stringDataLength;

void generateClassImpl(QTextStream &out, const QMetaObject *mo, const QByteArray &className, const QByteArray &nameSpace, ObjectCategory category)
{
    QByteArray qualifiedClassName;
    if (!nameSpace.isEmpty())
        qualifiedClassName = nameSpace + "::";
    qualifiedClassName += className;

    QByteArray stringData(qualifiedClassName);
    int stringDataLength = stringData.length();
    stringData += "\\0\"\n";
    ++stringDataLength;
    int lineLength = 0;

    int classInfoCount = mo->classInfoCount() - mo->classInfoOffset();
    int enumCount = mo->enumeratorCount() - mo->enumeratorOffset();
    int memberCount = mo->memberCount() - mo->memberOffset();
    int propertyCount = mo->propertyCount() - mo->propertyOffset();
    int enumStart = 10;

    out << "static const uint qt_meta_data_" << qualifiedClassName.replace(':', '_') << "[] = {" << endl;
    out << endl;
    out << " // content:" << endl;
    out << "       1,       // revision" << endl;
    out << "       0,       // classname" << endl;
    out << "       " << classInfoCount << ",    " << (classInfoCount ? enumStart : 0) << ", // classinfo" << endl;
    enumStart += classInfoCount * 2;
    out << "       " << memberCount << ",    " << (memberCount ? enumStart : 0) << ", // members" << endl;
    enumStart += memberCount * 5;
    out << "       " << propertyCount << ",    " << (propertyCount ? enumStart : 0) << ", // properties" << endl;
    enumStart += propertyCount * 3;
    out << "       " << enumCount << ",    " << (enumCount ? enumStart : 0)
        << ", // enums/sets" << endl;
    out << endl;

    if (classInfoCount) {
        out << " // classinfo: key, value" << endl;
        stringData += "    \"";
        for (int i = 0; i < classInfoCount; ++i) {
            QMetaClassInfo classInfo = mo->classInfo(i + mo->classInfoOffset());
            out << "       ";
            addString(classInfo.name());
            addString(classInfo.value());
            out << endl;
        }
        stringData += "\"\n";
        out << endl;
    }
    if (memberCount) {
        out << " // signals: signature, parameters, type, tag, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < memberCount; ++i) {
            const QMetaMember signal(mo->member(i + mo->memberOffset()));
            if (signal.memberType() != QMetaMember::Signal)
                continue;
            out << "       ";
            addString(signal.signature());
            addString(joinParameterNames(signal.parameterNames()));
            addString(signal.typeName());
            addString(signal.tag());
            out << (AccessProtected | signal.attributes() | MemberSignal) << "," << endl;
        }
        stringData += "\"\n";
        out << endl;

        out << " // slots: signature, parameters, type, tag, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < memberCount; ++i) {
            const QMetaMember slot(mo->member(i + mo->memberOffset()));
            if (slot.memberType() != QMetaMember::Slot)
                continue;
            out << "       ";
            addString(slot.signature());
            addString(joinParameterNames(slot.parameterNames()));
            addString(slot.typeName());
            addString(slot.tag());
            out << (0x01 | slot.attributes() | MemberSlot) << "," << endl;
        }
        stringData += "\"\n";
        out << endl;
    }
    if (propertyCount) {
        out << " // properties: name, type, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < propertyCount; ++i) {
            QMetaProperty property = mo->property(i + mo->propertyOffset());
            out << "       ";
            addString(property.name());
            addString(property.typeName());

            uint flags = 0;
            uint vartype = property.type();
            if (vartype != QVariant::Invalid && vartype != QVariant::UserType)
                flags = vartype << 24;
            else if (QByteArray(property.typeName()) == "QVariant")
                flags |= 0xff << 24;

            if (property.isReadable())
                flags |= Readable;
            if (property.isWritable())
                flags |= Writable;
            if (property.isEnumType())
                flags |= EnumOrFlag;
            if (property.isDesignable())
                flags |= Designable;
            if (property.isScriptable())
                flags |= Scriptable;
            if (property.isStored())
                flags |= Stored;
            if (property.isEditable())
                flags |= Editable;

            out << flags << "," << endl;           
        }
        stringData += "\"\n";
        out << endl;
    }
    if (enumCount) {
        out << " // enums: name, flags, count, data" << endl;
        stringData += "    \"";
        enumStart += enumCount * 4;
        for (int i = 0; i < enumCount; ++i) {
            QMetaEnum enumerator = mo->enumerator(i + mo->enumeratorOffset());
            out << "       ";
            addString(enumerator.name());
            out << (enumerator.isFlag() ? "0x1" : "0x0") << ", " << enumerator.keyCount() << ", " << enumStart << ", " << endl;
            enumStart += enumerator.keyCount() * 2;
        }
        stringData += "\"\n";
        out << endl;

        out << " // enum data: key, value" << endl;
        for (int i = 0; i < enumCount; ++i) {
            stringData += "    \"";
            QMetaEnum enumerator = mo->enumerator(i + mo->enumeratorOffset());
            for (int j = 0; j < enumerator.keyCount(); ++j) {
                out << "       ";
                addString(enumerator.key(j));
                if (nameSpace.isEmpty())
                    out << className << "::";
                else
                    out << nameSpace << "::";
                out << enumerator.key(j) << "," << endl;
            }
            stringData += "\"\n";
        }        
        out << endl;
    }
    out << "        0        // eod" << endl;
    out << "};" << endl;
    out << endl;

    out << "static const char qt_meta_stringdata_" << qualifiedClassName.replace(':','_') << "[] = {" << endl;
    out << "    \"" << stringData << endl;
    out << "};" << endl;
    out << endl;

    out << "const QMetaObject " << className << "::staticMetaObject = {" << endl;
    if (category & ActiveX)
        out << "{ &QWidget::staticMetaObject," << endl;
    else
        out << "{ &QObject::staticMetaObject," << endl;
    out << "qt_meta_stringdata_" << qualifiedClassName.replace(':','_') << "," << endl;
    out << "qt_meta_data_" << qualifiedClassName.replace(':','_') << " }" << endl;
    out << "};" << endl;
    out << endl;

    out << "void *" << className << "::qt_metacast(const char *_clname)" << endl;
    out << "{" << endl;
    out << "    if (!_clname) return 0;" << endl;
    out << "    if (!strcmp(_clname, qt_meta_stringdata_" << qualifiedClassName.replace(':','_') << "))" << endl;
    out << "        return static_cast<void*>(const_cast<" << className << "*>(this));" << endl;
    if (category & ActiveX)
        out << "    return QAxWidget::qt_metacast(_clname);" << endl;
    else
        out << "    return QAxObject::qt_metacast(_clname);" << endl;
    out << "}" << endl;

}

bool generateClass(QAxObject *object, const QByteArray &className, const QByteArray &nameSpace, const QByteArray &outname, ObjectCategory category)
{
    IOleControl *control = 0;
    object->queryInterface(IID_IOleControl, (void**)&control);
    if (control) {
        category = ActiveX;
        control->Release();
    }

    const QMetaObject *mo = object->metaObject();

    if (!nameSpace.isEmpty() && !(category & NoDeclaration)) {
        QFile outfile(nameSpace.toLower() + ".h");
		if (!outfile.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            qWarning("dumpcpp: Could not open output file '%s'", outfile.fileName().latin1());
            return false;
        }
        QTextStream out(&outfile);

        out << "/****************************************************************************" << endl;
        out << "**" << endl;
        out << "** Namespace " << nameSpace << " generated by dumpcpp" << endl;
        out << "**" << endl;
        out << "****************************************************************************/" << endl;
        out << endl;

        writeHeader(out, nameSpace);
        generateNameSpace(out, mo, nameSpace);

        // close namespace file
        out << "};" << endl;
        out << endl;

        out << "#endif" << endl;
        out << endl;
    }

    if (!(category & NoDeclaration)) {
        QFile outfile(outname + ".h");
        if (!outfile.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            qWarning("dumpcpp: Could not open output file '%s'", outfile.fileName().latin1());
            return false;
        }
        QTextStream out(&outfile);

        out << "/****************************************************************************" << endl;
        out << "**" << endl;
        out << "** Class declaration generated by dumpcpp" << endl;
        out << "**" << endl;
        out << "****************************************************************************/" << endl;
        out << endl;

        out << "#include <qdatetime.h>" << endl;
        if (category & ActiveX)
            out << "#include <qaxwidget.h>" << endl;
        else
            out << "#include <qaxobject.h>" << endl;
        out << endl;

        out << "struct IDispatch;" << endl,
        out << endl;

        if (!nameSpace.isEmpty()) {
            out << "#include \"" << nameSpace.toLower() << ".h\"" << endl;
            out << endl;
            out << "namespace " << nameSpace << " {" << endl;
        }

        generateClassDecl(out, object->control(), mo, className, nameSpace, category);

        if (!nameSpace.isEmpty()) {
            out << endl;
            out << "};" << endl;
        }
    }

    if (!(category & (NoMetaObject|NoImplementation))) {
        QFile outfile(outname + ".cpp");
        if (!outfile.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            qWarning("dumpcpp: Could not open output file '%s'", outfile.fileName().latin1());
            return false;
        }
        QTextStream out(&outfile);

        out << "#include <qmetaobject.h>" << endl;
        out << "#include \"" << outname << ".h\"" << endl;
        out << endl;

        if (!nameSpace.isEmpty()) {
            out << "using namespace " << nameSpace << ";" << endl;
            out << endl;
        }

        generateClassImpl(out, mo, className, nameSpace, category);
    }

    return true;
}

bool generateTypeLibrary(const QByteArray &typeLib, const QByteArray &outname, ObjectCategory category)
{
    QString typeLibFile(typeLib);
    typeLibFile = typeLibFile.replace('/', '\\');
    QString cppFile(outname);

    ITypeLib *typelib;
    LoadTypeLibEx(reinterpret_cast<const wchar_t *>(typeLibFile.utf16()), REGKIND_NONE, &typelib);
    if (!typelib) {
        qWarning("dumpcpp: loading '%s' as a type library failed", typeLibFile.latin1());
        return false;
    }

    QString libName;
    BSTR nameString;
    HRESULT hasDocu = typelib->GetDocumentation(-1, &nameString, 0, 0, 0);
    libName = BSTRToQString(nameString);
    SysFreeString(nameString);

    QString libVersion("1.0");

    TLIBATTR *tlibattr = 0;
    typelib->GetLibAttr(&tlibattr);
    if (tlibattr) {
        libVersion = QString("%1.%2").arg(tlibattr->wMajorVerNum).arg(tlibattr->wMinorVerNum);
        typelib->ReleaseTLibAttr(tlibattr);
    }

    if (cppFile.isEmpty())
        cppFile = libName.toLower();

    if (cppFile.isEmpty()) {
        qWarning("dumpcpp: no output filename provided, and cannot deduce output filename");
        return false;
    }

    QMetaObject *namespaceObject = qax_readEnumInfo(typelib, 0);

    QFile implFile(cppFile + ".cpp");
    QTextStream implOut(&implFile);
    if (!(category & (NoMetaObject|NoImplementation))) {
        if (!implFile.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            qWarning("dumpcpp: Could not open output file '%s'", implFile.fileName().latin1());
            return false;
        }

        implOut << "/****************************************************************************" << endl;
        implOut << "**" << endl;
        implOut << "** Metadata for " << libName << " generated by dumpcpp from type library" << endl;
        implOut << "** " << typeLibFile << endl;
        implOut << "**" << endl;
        implOut << "****************************************************************************/" << endl;
        implOut << endl;

        implOut << "#define QAX_DUMPCPP_" << libName.toUpper() << "_NOINLINES" << endl;

        implOut << "#include \"" << cppFile << ".h\"" << endl;
        implOut << endl;
        implOut << "using namespace " << libName << ";" << endl;
        implOut << endl;
    }

    QFile declFile(cppFile + ".h");
    QTextStream declOut(&declFile);
    QByteArray classes;
    QTextStream classesOut(&classes, QIODevice::WriteOnly);
    QByteArray inlines;
    QTextStream inlinesOut(&inlines, QIODevice::WriteOnly);

    QMap<QByteArray, QList<QByteArray> > namespaces;

    if(!(category & NoDeclaration)) {
        if (!declFile.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            qWarning("dumpcpp: Could not open output file '%s'", declFile.fileName().latin1());
            return false;
        }

        declOut << "/****************************************************************************" << endl;
        declOut << "**" << endl;
        declOut << "** Namespace " << libName << " generated by dumpcpp from type library" << endl;
        declOut << "** " << typeLibFile << endl;
        declOut << "**" << endl;
        declOut << "****************************************************************************/" << endl;
        declOut << endl;

        writeHeader(declOut, libName.latin1());

        UINT typeCount = typelib->GetTypeInfoCount();
        if (declFile.isOpen()) {
            declOut << endl;
            declOut << "// Referenced namespace" << endl;
            for (UINT index = 0; index < typeCount; ++index) {
                ITypeInfo *typeinfo = 0;
                typelib->GetTypeInfo(index, &typeinfo);
                if (!typeinfo)
                    continue;

                TYPEATTR *typeattr;
                typeinfo->GetTypeAttr(&typeattr);
                if (!typeattr || typeattr->wTypeFlags & TYPEFLAG_FHIDDEN) {
                    typeinfo->Release();
                    continue;
                }

                TYPEKIND typekind;
                typelib->GetTypeInfoType(index, &typekind);

                QMetaObject *metaObject = 0;

                // trigger meta object to collect references to other type libraries
                switch (typekind) {
                case TKIND_COCLASS:
                    metaObject = qax_readClassInfo(typelib, typeinfo, &QObject::staticMetaObject);
                    break;
                case TKIND_DISPATCH:
                    metaObject = qax_readInterfaceInfo(typelib, typeinfo, &QObject::staticMetaObject);
                    break;
                default:
                    break;
                }

                delete metaObject;
                typeinfo->ReleaseTypeAttr(typeattr);
                typeinfo->Release();
            }

            for (int i = 0; i < qax_qualified_usertypes.count(); ++i) {
                QByteArray refType = qax_qualified_usertypes.at(i);
                QByteArray refTypeLib;
                if (refType.contains("::")) {
                    refTypeLib = refType;
                    refType = refType.mid(refType.lastIndexOf("::") + 2);
                    refTypeLib = refTypeLib.left(refTypeLib.indexOf("::"));
                    namespaces[refTypeLib].append(refType);
                } else {
                    namespaces[libName.latin1()].append(refType);
                }                
            }

            QList<QByteArray> keys = namespaces.keys();
            for (int n = 0; n < keys.count(); ++n) {
                QByteArray nspace = keys.at(n);
                if (nspace != libName) {
                    declOut << "namespace " << nspace << " {" << endl;
                    QList<QByteArray> classes = namespaces.value(nspace);
                    for (int c = 0; c < classes.count(); ++c) {
                        declOut << "    class " << classes.at(c) << ";" << endl;
                    }
                    declOut << "}" << endl << endl;
                }
            }

            declOut << endl;
        }
        generateNameSpace(declOut, namespaceObject, libName.latin1());

        QList<QByteArray> classes = namespaces.value(libName.latin1());
        if (classes.count())
            declOut << "// forward declarations" << endl;
        for (int c = 0; c < classes.count(); ++c)
            declOut << "    class " << classes.at(c) << ";" << endl;

        declOut << endl;
    }

    UINT typeCount = typelib->GetTypeInfoCount();
    for (UINT index = 0; index < typeCount; ++index) {
        ITypeInfo *typeinfo = 0;
        typelib->GetTypeInfo(index, &typeinfo);
        if (!typeinfo)
            continue;

        TYPEATTR *typeattr;
        typeinfo->GetTypeAttr(&typeattr);
        if (!typeattr || typeattr->wTypeFlags & TYPEFLAG_FHIDDEN) {
            typeinfo->Release();
            continue;
        }

        TYPEKIND typekind;
        typelib->GetTypeInfoType(index, &typekind);

        uint object_category = category;
        if (!(typeattr->wTypeFlags & TYPEFLAG_FCANCREATE))
            object_category |= SubObject;
        else if (typeattr->wTypeFlags & TYPEFLAG_FCONTROL)
            object_category |= ActiveX;

        QMetaObject *metaObject = 0;
        QUuid guid(typeattr->guid);

        switch (typekind) {
        case TKIND_COCLASS:
            metaObject = qax_readClassInfo(typelib, typeinfo, &QObject::staticMetaObject);
            break;
        case TKIND_DISPATCH:
            metaObject = qax_readInterfaceInfo(typelib, typeinfo, &QObject::staticMetaObject);
            break;
        default:
            break;
        }

        if (metaObject) {
            QByteArray className(metaObject->className());
            if (declFile.isOpen()) {
                if (typekind == TKIND_COCLASS) { // write those later...
                    generateClassDecl(classesOut, guid.toString(), metaObject, className, libName.latin1(), (ObjectCategory)(object_category|NoInlines));
                    classesOut << endl;
                } else {
                    generateClassDecl(declOut, guid.toString(), metaObject, className, libName.latin1(), (ObjectCategory)(object_category|NoInlines));
                    declOut << endl;
                }
                generateClassDecl(inlinesOut, guid.toString(), metaObject, className, libName.latin1(), (ObjectCategory)(object_category|OnlyInlines));
                inlinesOut << endl;
            }
            if (implFile.isOpen()) {
                generateClassImpl(implOut, metaObject, className, libName.latin1(), (ObjectCategory)object_category);
                implOut  << endl;
            }
        }

        delete metaObject;

        typeinfo->ReleaseTypeAttr(typeattr);
        typeinfo->Release();
    }

    delete namespaceObject;

    if (declFile.isOpen()) {
        if (classes.size()) {
            declOut << "// Actual coclasses" << endl;
            declOut << classes;
        }
        if (inlines.size()) {
            declOut << "// member function implementation" << endl;
            declOut << "#ifndef QAX_DUMPCPP_" << libName.toUpper() << "_NOINLINES" << endl;
            declOut << inlines << endl;
            declOut << "#endif" << endl << endl;
        }
        // close namespace
        declOut << "}" << endl;
        declOut << endl;

        declOut << "#endif" << endl;
        declOut << endl;
    }

    typelib->Release();
    return true;
}

int main(int argc, char **argv)
{
    extern bool qax_dispatchEqualsIDispatch;

    CoInitialize(0);

    uint category = DefaultObject;

    enum State {
        Default = 0,
        Output,
        NameSpace
    } state;
    state = Default;
    
    QByteArray outname;
    QByteArray typeLib;
    QByteArray nameSpace;
    
    for (int a = 1; a < argc; ++a) {
        QByteArray arg(argv[a]);
        const char first = arg[0];
        switch(state) {
        case Default:
            if (first == '-' || first == '/') {
                arg = arg.mid(1);
                arg.toLower();

                if (arg == "o") {
                    state = Output;
                } else if (arg == "n") {
                    state = NameSpace;
                } else if (arg == "v") {
                    qWarning("dumpcpp: Version 1.0");
                    return 0;
                } else if (arg == "nometaobject") {
                    category |= NoMetaObject;
                } else if (arg == "impl") {
                    category |= NoDeclaration;
                } else if (arg == "decl") {
                    category |= NoImplementation;
                } else if (arg == "donothing") {
                    category = DoNothing;
                    break;
                } else if (arg == "no_compat") {
                    qax_dispatchEqualsIDispatch = false;
                    break;
                } else if (arg == "h") {
                    qWarning("dumpcpp Version1.0\n\n"
                        "Generate a C++ namespace from a type library.\n\n"
                        "Usage:\n"
                        "dumpcpp input [-[-n <namespace>] [-o <filename>]\n\n"
                        "   input:     A type library file, type library ID, ProgID or CLSID\n\n"
                        "Optional parameters:\n"
                        "   namespace: The name of the generated C++ namespace\n"
                        "   filename:  The file name (without extension) of the generated files\n"
                        "\n"
                        "Other parameters:\n"
                        "   -nometaobject Don't generate meta object information (no .cpp file)\n"
                        "   -impl Only generate the .cpp file\n"
                        "   -decl Only generate the .h file\n"
                        "\n"
                        "Examples:\n"
                        "   dumpcpp Outlook.Application -o outlook\n"
                        "   dumpcpp {3B756301-0075-4E40-8BE8-5A81DE2426B7}\n"
                        "\n");
                    return 0;
                }
            } else {
                typeLib = arg;
            }
            break;

        case Output:
            outname = arg;
            state = Default;
            break;

        case NameSpace:
            nameSpace = arg;
            state = Default;
            break;

        default:
            break;
        }
    }

    if (category == DoNothing)
        return 0;
    
    if (typeLib.isEmpty()) {
        qWarning("dumpcpp: No object class or type library name provided.\n"
            "         Use -h for help.");
        return -1;
    }

    // not a file - search registry
    if (!QFile::exists(typeLib)) {
        bool isObject = false;
        QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Classes", Qt::NativeFormat);

        // regular string and not a file - must be ProgID
        if (typeLib.at(0) != '{') {
            CLSID clsid;
            if (CLSIDFromProgID(reinterpret_cast<const wchar_t *>(QString(QLatin1String(typeLib)).utf16()), &clsid) != S_OK) {
                qWarning("dumpcpp: '%s' is not a type library and not a registered ProgID", typeLib.constData());
                return -2;
            }
            QUuid uuid(clsid);
            typeLib = uuid.toString().latin1();
            isObject = true;
        }

        // check if CLSID
        if (!isObject) {
            QVariant test = settings.value("/CLSID/" + typeLib + "/.");
            isObject = test.isValid();
        }

        // search typelib ID for CLSID
        if (isObject)
            typeLib = settings.value("/CLSID/" + typeLib + "/Typelib/.").toByteArray();

        // interpret input as type library ID
        QString key = QString("/TypeLib/") + QLatin1String(typeLib);
        settings.beginGroup(key);
        QStringList versions = settings.childGroups();
        QStringList codes;
        if (versions.count()) {
            settings.beginGroup("/" + versions.last());
            codes = settings.childGroups();
            key += "/" + versions.last();
            settings.endGroup();
        }
        settings.endGroup();

        for (int c = 0; c < codes.count(); ++c) {
            typeLib = settings.value(key + "/" + codes.at(c) + "/win32/.").toByteArray();
            if (QFile::exists(typeLib)) {
                break;
            }
        }
    }

    if (!QFile::exists(typeLib)) {
        qWarning("dumpcpp: type library '%s' not found", typeLib.constData());
        return -2;
    }

    if (!generateTypeLibrary(typeLib, outname, (ObjectCategory)category)) {
        qWarning("dumpcpp: error processing type library '%s'", typeLib.constData());
        return -1;
    }

    return 0;
}

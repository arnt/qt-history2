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
#include <qtextstream.h>
#include <quuid.h>

#include <qt_windows.h>
#include <ocidl.h>
#include <ctype.h>

#include "../../shared/types.h"

void *qMalloc(size_t size) { return ::malloc(size); }

enum ObjectCategory
{
    DefaultObject   = 0x0,
    SubObject       = 0x01,
    ActiveX         = 0x02,
    NoMetaObject    = 0x04,
    NoImplementation = 0x08,
    NoDeclaration   = 0x10,
    DoNothing       = 0x11
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

enum FunctionFlags  {
    AccessPrivate = 0x01,
    AccessPublic = 0x02,
    AccessProtected = 0x04,
    AccessMask = 0x07, //mask
    Compatability = QMetaMember::Compatability,
    Cloned = QMetaMember::Cloned
};

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

void generateNameSpace(QTextStream &out, const QMetaObject *mo, const QByteArray &nameSpace)
{
    out << "#ifndef QAX_DUMPCPP_" << nameSpace.toUpper() << "_H" << endl;
    out << "#define QAX_DUMPCPP_" << nameSpace.toUpper() << "_H" << endl;
    out << endl;
    out << "#include <qaxobject.h>" << endl;
    out << "#include <qaxwidget.h>" << endl;
    out << "#include <qdatetime.h>" << endl;
    out << endl;
    out << "struct IDispatch;" << endl;

    out << "namespace " << nameSpace << " {" << endl;
    out << endl;
    writeEnums(out, mo);

    // don't close on purpose
}

void generateClassDecl(QTextStream &out, const QString &controlID, const QMetaObject *mo, const QByteArray &className, const QByteArray &nameSpace, ObjectCategory category)
{
    QList<QByteArray> functions;

    // constructor
    out << "class " << className << " : public ";
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

    functions << className;

    // enums
    if (nameSpace.isEmpty()) {
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
 
        out << "    inline " << propertyType << " " << propertyName << "() const {" << endl;
        out << "        QVariant qax_result = property(\"" << propertyName << "\");" << endl;
        if (propertyType.at(propertyType.length()-1) == '*')
            out << "        if (!qax_result.constData()) return 0;" << endl;
        out << "        return *(" << propertyType << "*)qax_result.constData();" << endl;
        out << "    }" << endl;

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
            out << "    inline void " << setter << "(" << propertyType << " value) { setProperty(\"" << propertyName << "\", QVariant(value)); }" << endl;
            functions << setter;
        }

        out << endl;
    }

    // slots - but not property setters
    for (int islot = mo->slotOffset(); islot < mo->slotCount(); ++islot) {
        QMetaMember slot = mo->slot(islot);
        
        QByteArray slotSignature(slot.signature());
        if (functions.contains(slotSignature.left(slotSignature.indexOf('('))))
            continue;
        QByteArray slotParameters(slot.parameters());
        QByteArray slotTag(slot.tag());
        QByteArray slotType(slot.typeName());

        QByteArray slotNamedSignature;
        if (!slotParameters.isEmpty()) {
            QByteArray slotSignatureTruncated(slotSignature.left(slotSignature.indexOf(')')));
            QList<QByteArray> signatureSplit = slotSignatureTruncated.split(',');
            QList<QByteArray> parameterSplit = slotParameters.split(',');
            for (int i = 0; i < signatureSplit.count(); ++i) {
                slotNamedSignature += signatureSplit.at(i);
                slotNamedSignature += " ";
                slotNamedSignature += parameterSplit.at(i);
                if (i + 1 < signatureSplit.count())
                    slotNamedSignature += ", ";
            }
            slotNamedSignature += ')';
        } else if (!slotSignature.contains("()")) {
            out << "    // ignored slot - no parameters: " << slotSignature;
            continue;
        } else {
            slotNamedSignature = slotSignature;
        }

        out << "    inline ";
        if (!slotTag.isEmpty())
            out << slotTag << " ";
        if (slotType.isEmpty())
            out << "void ";
        else
            out << slotType << " ";
        out << slotNamedSignature << " {" << endl;
        if (!slotParameters.isEmpty()) {
            out << "        QVariantList qax_parameters;" << endl;
            out << "        qax_parameters << " << slotParameters.replace(',', " << ") << ";" << endl;
        }
        out << "        ";
        if (!slotType.isEmpty())
            out << "QVariant qax_result = ";

        out << "dynamicCall(\"" << slotSignature << "\"";
        if (!slotParameters.isEmpty())
            out << ", qax_parameters";
        out << ");" << endl;
        if (!slotType.isEmpty()) {
            if (slotType.at(slotType.length() - 1) == '*')
                out << "        if (!qax_result.constData()) return 0;" << endl;
            out << "        return *(" << slotType << "*)qax_result.constData();" << endl;
        }
        out << "    }" << endl;

        out << endl;
    }

    if (!(category & NoMetaObject)) {
        out << "// meta object functions" << endl;
        out << "    static const QMetaObject staticMetaObject;" << endl;
        out << "    virtual const QMetaObject *metaObject() const { return &staticMetaObject; }" << endl;
        out << "    virtual void *qt_metacast(const char *);" << endl;
    }

    out << "};" << endl;
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
    int signalCount = mo->signalCount() - mo->signalOffset();
    int slotCount = mo->slotCount() - mo->slotOffset();
    int propertyCount = mo->propertyCount() - mo->propertyOffset();
    int enumStart = 12 + classInfoCount * 2 + signalCount * 5 + slotCount * 5 + propertyCount * 3;

    out << "static const uint qt_meta_data_" << qualifiedClassName.replace(':', '_') << "[] = {" << endl;
    out << endl;
    out << " // content:" << endl;
    out << "       1,       // revision" << endl;
    out << "       0,       // classname" << endl;
    out << "       " << classInfoCount << ",    " 
        << (classInfoCount ? 12 : 0) << ", // classinfo" << endl;
    out << "       " << signalCount << ",    "
        << (signalCount ? 12 + classInfoCount * 2 : 0)
        << ", // signals" << endl;
    out << "       " << slotCount << ",    "
        << (slotCount ? 12 + classInfoCount * 2 + signalCount * 5 : 0)
        << ", // slots" << endl;
    out << "       " << propertyCount << ",    "
        << (propertyCount ? 12 + classInfoCount * 2 + signalCount * 5 + slotCount * 5 : 0)
        << ", // properties" << endl;
    out << "       " << enumCount << ",    "
        << (enumCount ? enumStart : 0)
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
    if (signalCount) {
        out << " // signals: signature, parameters, type, tag, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < signalCount; ++i) {
            QMetaMember signal = mo->signal(i + mo->signalOffset());
            out << "       ";
            addString(signal.signature());
            addString(signal.parameters());
            addString(signal.typeName());
            addString(signal.tag());
            out << (AccessProtected | signal.attributes()) << "," << endl;
        }
        stringData += "\"\n";
        out << endl;
    }
    if (slotCount) {
        out << " // slots: signature, parameters, type, tag, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < slotCount; ++i) {
            QMetaMember slot = mo->slot(i + mo->slotOffset());
            out << "       ";
            addString(slot.signature());
            addString(slot.parameters());
            addString(slot.typeName());
            addString(slot.tag());
            out << (0x01 | slot.attributes()) << "," << endl;
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
            if (vartype != QVariant::Invalid)
                flags = vartype << 24;
            else if (QByteArray(property.typeName()) == "QVariant")
                flags |= 0xff << 24;
            else if (QByteArray(property.typeName()).endsWith('*'))
                flags |= QVariant::UserType << 24;

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
        if (!outfile.open(IO_WriteOnly | IO_Translate)) {
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

        generateNameSpace(out, mo, nameSpace);

        // close namespace file
        out << "};" << endl;
        out << endl;

        out << "#endif" << endl;
        out << endl;
    }

    if (!(category & NoDeclaration)) {
        QFile outfile(outname + ".h");
        if (!outfile.open(IO_WriteOnly | IO_Translate)) {
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
        if (!outfile.open(IO_WriteOnly | IO_Translate)) {
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

extern QMetaObject *qax_readEnumInfo(ITypeLib *typeLib, const QMetaObject *parentObject);
extern QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);
extern QMetaObject *qax_readInterfaceInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);

bool generateTypeLibrary(const QByteArray &typeLib, const QByteArray &outname, ObjectCategory category)
{
    QString typeLibFile(typeLib);
    typeLibFile = typeLibFile.replace('/', '\\');
    QString cppFile(outname);

    ITypeLib *typelib;
    LoadTypeLibEx(typeLibFile.utf16(), REGKIND_NONE, &typelib);
    if (!typelib) {
        qWarning("dumpcpp: loading '%s' as a type library failed", typeLibFile.latin1());
        return false;
    }

    QString libName, libDoc, libHelp;

    BSTR nameString;
    BSTR docString;
    DWORD helpContext;
    BSTR helpFile;
    if (S_OK == typelib->GetDocumentation(-1, &nameString, &docString, &helpContext, &helpFile)) {
        libName = BSTRToQString(nameString);
        libDoc = BSTRToQString(docString);
        libHelp = BSTRToQString(helpFile);
    }

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
        qWarning("dumpcpp: no output filename provided, and cannot deduce output filename", typeLibFile.latin1());
        return false;
    }

    QMetaObject *namespaceObject = qax_readEnumInfo(typelib, 0);

    QFile implFile(cppFile + ".cpp");
    QTextStream implOut(&implFile);
    if (!(category & (NoMetaObject|NoImplementation))) {
        if (!implFile.open(IO_WriteOnly | IO_Translate)) {
            qWarning("dumpcpp: Could not open output file '%s'", implFile.fileName().latin1());
            return false;
        }

        implOut << "#include \"" << cppFile << ".h\"" << endl;
        implOut << endl;
        implOut << "using namespace " << libName << ";" << endl;
        implOut << endl;
    }

    QFile declFile(cppFile + ".h");
    QTextStream declOut(&declFile);
    if(!(category & NoDeclaration)) {
        if (!declFile.open(IO_WriteOnly | IO_Translate)) {
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

        generateNameSpace(declOut, namespaceObject, libName.latin1());
    }

    UINT typeCount = typelib->GetTypeInfoCount();
    for (UINT index = 0; index < typeCount; ++index) {
        TYPEKIND typekind;
        typelib->GetTypeInfoType(index, &typekind);

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

        uint object_category = category;
        if (!(typeattr->wTypeFlags & TYPEFLAG_FCANCREATE))
            object_category |= SubObject;
        else if (typeattr->wTypeFlags & TYPEFLAG_FCONTROL)
            object_category |= ActiveX;

        QUuid guid(typeattr->guid);
        QMetaObject *metaObject = 0;

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
            if (className.at(0) != '_') {
                if (declFile.isOpen()) {
                    generateClassDecl(declOut, guid.toString(), metaObject, className, libName.latin1(), (ObjectCategory)object_category);
                    declOut << endl;
                }
                if (implFile.isOpen()) {
                    generateClassImpl(implOut, metaObject, className, libName.latin1(), (ObjectCategory)object_category);
                    implOut  << endl;
                }
            }
        }

        delete metaObject;

        typeinfo->ReleaseTypeAttr(typeattr);
        typeinfo->Release();
    }

    delete namespaceObject;

    if (declFile.isOpen()) {
        // close namespace
        declOut << "};" << endl;
        declOut << endl;

        declOut << "#endif" << endl;
        declOut << endl;
    }

    typelib->Release();
    return true;
}

int main(int argc, char **argv)
{
    CoInitialize(0);

    uint category = DefaultObject;

    enum State {
        Default = 0,
        Output,
        NameSpace,
        ClassName
    } state;
    state = Default;
    
    QByteArray outname;
    QByteArray object;
    
    QByteArray nameSpace;
    QByteArray className;
    
    for (int a = 1; a < argc; ++a) {
        QByteArray arg(argv[a]);
        const char first = arg[0];
        switch(state) {
        case Default:
            if (first == '-' || first == '/') {
                arg = arg.mid(1);
                arg.toLower();
                if (arg == "c") {
                    state = ClassName;
                } else if (arg == "o") {
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
                } else if (arg == "h") {
                    qWarning("dumpdoc Usage:\n\tdumpcpp object -c <classname> [-n <namespace>] [-o <filename>] [-nometaobject]"
                        "              \n\tobject   : object[/subobject]*"
                        "              \n\tsubobject: property\n"
                        "      \nexample:\n\tdumpcpp Outlook.Application/Session/CurrentUser -c CurrentUser -o outlook.currentuser");
                    return 0;
                }
            } else {
                object = arg;
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

        case ClassName:
            className = arg;
            state = Default;
            break;
            
        default:
            break;
        }
    }

    if (category == DoNothing)
        return 0;
    
    if (object.isEmpty()) {
        qWarning("dumpcpp: No object or type library name provided.\n"
            "         Use -h for help.");
        return -1;
    }

    if (QFile::exists(object) && generateTypeLibrary(object, outname, (ObjectCategory)category))
        return 0;

    QByteArray subobject = object;
    int index = subobject.indexOf('/');
    if (index != -1)
        subobject = subobject.left(index);
    
    QAxObject topobject(subobject);
    
    if (topobject.isNull()) {
        qWarning("dumpcpp: Could not instantiate COM object '%s'", subobject.data());
        return -2;
    }

    if (className.isEmpty()) {
        if (!subobject.contains('{') && !subobject.contains(' ')) {
            int classNameIndex = 0;
            if (subobject.contains('.')) {
                nameSpace = object.left(subobject.indexOf('.'));
                classNameIndex = subobject.indexOf('.') + 1;
            }
            if (object.contains('/'))
                classNameIndex = object.lastIndexOf('/') + 1;
            className = object.mid(classNameIndex);
        } else {
            BSTR progid;
            IOleObject *oleObject = 0;
            topobject.queryInterface(IID_IOleObject, (void**)&oleObject);
            if (oleObject) {
                oleObject->GetUserType(USERCLASSTYPE_FULL, &progid);
                className = BSTRToQString(progid).latin1();
                oleObject->Release();
            }
        }
    }
    
    if (className.isEmpty())
        className = "Class1";

    QAxObject *axobject = &topobject;
    while (index != -1 && axobject) {
        index++;
        subobject = object.mid(index);
        if (object.indexOf('/', index) != -1) {
            int oldindex = index;
            index = object.indexOf('/', index);
            subobject = object.mid(oldindex, index-oldindex);	    
        } else {
            index = -1;
        }
        
        axobject = axobject->querySubObject(subobject);
        category |= SubObject;
    }
    if (!axobject || axobject->isNull()) {
        qWarning("dumpcpp: Subobject '%s' does not exist in '%s'", subobject.data(), object.data());
        return -3;
    }

    if (outname.isEmpty()) {
        if (!nameSpace.isEmpty())
            outname = nameSpace.toLower() + ".";
        outname += className.toLower();
    }
    return generateClass(axobject, className, nameSpace, outname, (ObjectCategory)category) ? 0 : -4;
}

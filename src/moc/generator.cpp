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

#include "generator.h"
#include "outputrevision.h"
#include "utils.h"
#include <stdio.h>

// if the flags change, you MUST to change it in qmetaobject.cpp too
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
    MemberScriptable = 0x40
};

/*
  Attention!  This table is copied from qcorevariant.cpp. If you
  change one, change both.
*/
static const int ntypes = 37;
static const char* const type_map[ntypes] =
{
    0,
    "QVariantMap",
    "QVariantList",
    "QString",
    "QStringList",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
    "QColorGroup",
    "QIcon",
    "QPoint",
    "QImage",
    "int",
    "uint",
    "bool",
    "double",
    "",
    "QPointArray",
    "QRegion",
    "QBitmap",
    "QCursor",
    "QSizePolicy",
    "QDate",
    "QTime",
    "QDateTime",
    "QByteArray",
    "QBitArray",
    "QKeySequence",
    "QPen",
    "Q_LONGLONG",
    "Q_ULONGLONG",
    "QChar"
    "QUrl"
};

int qvariant_nameToType(const char* name)
{
    if (name) {
        if (strcmp(name, "QVariant") == 0)
            return 0xffffffff;

        if (strcmp(name, "QCString") == 0)
            name = "QByteArray";
        else if (strcmp(name, "Q_LLONG") == 0)
            name = "Q_LONGLONG";
        else if (strcmp(name, "Q_ULLONG") == 0)
            name = "Q_ULONGLONG";
        else if (strcmp(name, "QIconSet") == 0)
            name = "QIcon";
        else if (strcmp(name, "QCoreVariantMap") == 0)
            name = "QVariantMap";
        else if (strcmp(name, "QCoreVariantList") == 0)
            name = "QVariantList";

        for (int i = 1; i < ntypes; i++) {
            if (!strcmp(type_map[i], name))
                return i;
        }
    }
    return 0;
}

/*
  Returns true if the type is a QVariant types.
*/
bool isVariantType(const char* type)
{
    return qvariant_nameToType(type) != 0;
}


static inline QByteArray noRef(const QByteArray &type)
{
    if (type.endsWith('&'))
        return type.left(type.length()-1);
    return type;
}

Generator::Generator(FILE *outfile, ClassDef *classDef) :out(outfile), cdef(classDef)
{
    /*
      When calling a superclass implementation of func(), we use
      SuperClass::func() rather than NameSpace::SuperClass::func() to work
      around a bug in MSVC 6. The bug occurs if the super-class is in a
      namespace and the sub-class isn't.

      Exception: If the superclass has the same name as the subclass, we
      want non-MSVC users to have a working generated files.
    */
    if (cdef->superclassList.size()) {
        purestSuperClass = cdef->superclassList.first();
        /*
          Make sure qualified template arguments (e.g., foo<bar::baz>)
          don't interfere.
        */
        int pos = purestSuperClass.lastIndexOf("::", purestSuperClass.indexOf('<'));
        if (pos != -1) {
            purestSuperClass = purestSuperClass.mid(pos + 2);
            if (purestSuperClass == cdef->classname)
                purestSuperClass = cdef->superclassList.first();
        }
    }
}

int Generator::strreg(const char *s)
{
    int idx = 0;
    if (!s)
        s = "";
    for (int i = 0; i < strings.size(); ++i) {
        const QByteArray &str = strings.at(i);
        if (str == s)
            return idx;
        idx += str.length() + 1;
    }
    strings.append(s);
    return idx;
}

void Generator::generateCode()
{
    bool isQt = (cdef->classname == "Qt");
    bool isQObject = (cdef->classname == "QObject");

//
// build the data array
//
    int i = 0;


    // filter out undeclared enumerators and sets
    {
        QList<EnumDef> enumList;
        for (i = 0; i < cdef->enumList.count(); ++i) {
            EnumDef def = cdef->enumList.at(i);
            if (cdef->enumDeclarations.contains(def.name)) {
                enumList += def;
            }
            QByteArray alias = cdef->flagAliases.value(def.name);
            if (cdef->enumDeclarations.contains(alias)) {
                def.name = alias;
                enumList += def;
            }
        }
        cdef->enumList = enumList;
    }


    QByteArray qualifiedClassNameIdentifier = cdef->qualified;
    qualifiedClassNameIdentifier.replace(':', '_');

    int index = 10;
    fprintf(out, "static const uint qt_meta_data_%s[] = {\n", qualifiedClassNameIdentifier.constData());
    fprintf(out, "\n // content:\n");
    fprintf(out, "    %4d,       // revision\n", 1);
    fprintf(out, "    %4d,       // classname\n", strreg(cdef->qualified));
    fprintf(out, "    %4d, %4d, // classinfo\n", cdef->classInfoList.count(), cdef->classInfoList.count() ? index : 0);
    index += cdef->classInfoList.count() * 2;

    int memberCount = cdef->signalList.count() + cdef->slotList.count() + cdef->methodList.count();
    fprintf(out, "    %4d, %4d, // members\n", memberCount, memberCount ? index : 0);
    index += memberCount * 5;
    fprintf(out, "    %4d, %4d, // properties\n", cdef->propertyList.count(), cdef->propertyList.count() ? index : 0);
    index += cdef->propertyList.count() * 3;
    fprintf(out, "    %4d, %4d, // enums/sets\n", cdef->enumList.count(), cdef->enumList.count() ? index : 0);


//
// Build classinfo array
//
    generateClassInfos();

//
// Build signals array first, otherwise the signal indices would be wrong
//
    generateFunctions(cdef->signalList, "signal", MemberSignal);

//
// Build slots array
//
    generateFunctions(cdef->slotList, "slot", MemberSlot);

//
// Build method array
//
    generateFunctions(cdef->methodList, "method", MemberMethod);


//
// Build property array
//
    generateProperties();

//
// Build enums array
//
    generateEnums(index);

//
// Terminate data array
//
    fprintf(out, "\n       0        // eod\n};\n\n");

//
// Build stringdata array
//
    fprintf(out, "static const char qt_meta_stringdata_%s[] = {\n", qualifiedClassNameIdentifier.constData());
    fprintf(out, "    \"");
    int col = 0;
    int len = 0;
    for (i = 0; i < strings.size(); ++i) {
        QByteArray s = strings.at(i);
        len = s.length();
        if (col && col + len >= 72) {
            fprintf(out, "\"\n    \"");
            col = 0;
        } else if (len && s.at(0) >= '0' && s.at(0) <= '9') {
            fprintf(out, "\"\"");
            len += 2;
        }
        fprintf(out, "%s\\0", s.constData());
        col += len + 2;
    }
    fprintf(out, "\"\n};\n\n");


//
// Finally create and initialize the static meta object
//

    if (isQt)
        fprintf(out, "const QMetaObject QObject::staticQtMetaObject = {\n");
    else
        fprintf(out, "const QMetaObject %s::staticMetaObject = {\n", cdef->qualified.constData());

    if (isQObject)
        fprintf(out, "    { &staticQtMetaObject,\n      ");
    else if (cdef->superclassList.size())
        fprintf(out, "    { &%s::staticMetaObject,\n      ", purestSuperClass.constData());
    else
        fprintf(out, "    { 0, ");
    fprintf(out, "qt_meta_stringdata_%s,\n      qt_meta_data_%s}\n",
             qualifiedClassNameIdentifier.constData(), qualifiedClassNameIdentifier.constData());
    fprintf(out, "};\n");

    if (isQt || !cdef->hasQObject)
        return;

//
// Generate smart cast function
//
    fprintf(out, "\nvoid *%s::qt_metacast(const char *_clname)\n{\n", cdef->qualified.constData());
    fprintf(out, "    if (!_clname) return 0;\n");
    fprintf(out, "    if (!strcmp(_clname, qt_meta_stringdata_%s))\n"
                  "\treturn static_cast<void*>(const_cast<%s*>(this));\n",
            qualifiedClassNameIdentifier.constData(),  cdef->qualified.constData());
    for (i = 1; i < cdef->superclassList.size(); ++i) { // for all superclasses but the first one
        const char *cname = cdef->superclassList.at(i);
        fprintf(out, "    if (!strcmp(_clname, \"%s\"))\n\treturn static_cast<%s*>(const_cast<%s*>(this));\n",
                cname, cname,   qualifiedClassNameIdentifier.constData());
    }
    for (i = 0; i < cdef->interfaceList.size(); ++i) {
        const QList<QByteArray> &iface = cdef->interfaceList.at(i);
        for (int j = 0; j < iface.size(); ++j) {
            fprintf(out, "    if (!strcmp(_clname, %s_iid))\n\treturn ", iface.at(j).constData());
            for (int k = j; k >= 0; --k)
                fprintf(out, "static_cast<%s*>(", iface.at(k).constData());
            fprintf(out, "const_cast<%s*>(this)%s;\n",
                    qualifiedClassNameIdentifier.constData(), QByteArray(j+1, ')').constData());
        }
    }
    if (purestSuperClass.size() && !isQObject)
        fprintf(out, "    return %s::qt_metacast(_clname);\n", purestSuperClass.constData());
    else
        fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");

//
// Generate internal qt_metacall()  function
//
    generateMetacall();

//
// Generate internal signal functions
//
    for (int signalindex = 0; signalindex < cdef->signalList.size(); ++signalindex)
        generateSignal(&cdef->signalList[signalindex], signalindex);

}


void Generator::generateClassInfos()
{
    if (cdef->classInfoList.isEmpty())
        return;

    fprintf(out, "\n // classinfo: key, value\n");

    for (int i = 0; i < cdef->classInfoList.size(); ++i) {
        const ClassInfoDef &c = cdef->classInfoList.at(i);
        fprintf(out, "    %4d, %4d,\n", strreg(c.name), strreg(c.value));
    }
}

void Generator::generateFunctions(QList<FunctionDef>& list, const char *functype, int type)
{
    if (list.isEmpty())
        return;
    fprintf(out, "\n // %ss: signature, parameters, type, tag, flags\n", functype);

    for (int i = 0; i < list.count(); ++i) {
        const FunctionDef &f = list.at(i);

        QByteArray sig = f.name + '(';
        QByteArray arguments;

        for (int j = 0; j < f.arguments.count(); ++j) {
            const ArgumentDef &a = f.arguments.at(j);
            if (j) {
                sig += ",";
                arguments += ",";
            }
            sig += normalizeType(a.normalizedType, true); // remove scoping
            arguments += a.name;
        }
        sig += ')';

        char flags = type;
        if (f.access == FunctionDef::Private)
            flags |= AccessPrivate;
        else if (f.access == FunctionDef::Public)
            flags |= AccessPublic;
        else if (f.access == FunctionDef::Protected)
            flags |= AccessProtected;
        if (f.access == FunctionDef::Private)
            flags |= AccessPrivate;
        else if (f.access == FunctionDef::Public)
            flags |= AccessPublic;
        else if (f.access == FunctionDef::Protected)
            flags |= AccessProtected;
        if (f.isCompat)
            flags |= MemberCompatibility;
        if (f.wasCloned)
            flags |= MemberCloned;
        if (f.isScriptable)
            flags |= MemberScriptable;
        fprintf(out, "    %4d, %4d, %4d, %4d, 0x%02x,\n", strreg(sig),
                strreg(arguments), strreg(f.normalizedType), strreg(f.tag), flags);
    }
}

void Generator::generateProperties()
{
    //
    // specify get function, for compatibiliy we accept functions
    // returning pointers, or const char * for QByteArray.
    //
    for (int i = 0; i < cdef->propertyList.count(); ++i) {
        PropertyDef &p = cdef->propertyList[i];
        if (p.read.isEmpty())
            continue;
        for (int j = 0; j < cdef->publicList.count(); ++j) {
            const FunctionDef &f = cdef->publicList.at(j);
            if (f.name != p.read)
                continue;
            if (!f.isConst) // get  functions must be const
                continue;
            if (f.arguments.size()) // and must not take any arguments
                continue;
            PropertyDef::Specification spec = PropertyDef::ValueSpec;
            QByteArray tmp = f.normalizedType;
            if (p.type == "QByteArray" && tmp == "const char *")
                    tmp = "QByteArray";
            if (tmp.left(6) == "const ")
                tmp = tmp.mid(6);
            if (p.type != tmp && tmp.endsWith('*')) {
                tmp.chop(1);
                spec = PropertyDef::PointerSpec;
            } else if (f.type.endsWith('&')) { // raw type, not normalized type
                spec = PropertyDef::ReferenceSpec;
            }
            if (p.type != tmp)
                continue;
            p.gspec = spec;
            break;
        }
    }


    //
    // Create meta data
    //

    if (cdef->propertyList.count())
        fprintf(out, "\n // properties: name, type, flags\n");
    for (int i = 0; i < cdef->propertyList.count(); ++i) {
        const PropertyDef &p = cdef->propertyList.at(i);
        int flags = Invalid;
        if (!isVariantType(p.type)) {
            flags |= EnumOrFlag;
        } else {
            flags |= qvariant_nameToType(p.type) << 24;
        }
        if (!p.read.isEmpty())
            flags |= Readable;
        if (!p.write.isEmpty()) {
            flags |= Writable;
            if (p.stdCppSet())
                flags |= StdCppSet;
        }
        if (!p.reset.isEmpty())
            flags |= Resetable;

        if (p.override)
            flags |= Override;

        if (p.designable.isEmpty())
            flags |= ResolveDesignable;
        else if (p.designable != "false")
            flags |= Designable;

        if (p.scriptable.isEmpty())
            flags |= ResolveScriptable;
        else if (p.scriptable != "false")
            flags |= Scriptable;

        if (p.stored.isEmpty())
            flags |= ResolveStored;
        else if (p.stored != "false")
            flags |= Stored;

        if (p.editable.isEmpty())
            flags |= ResolveEditable;
        else if (p.editable != "false")
            flags |= Editable;

        fprintf(out, "    %4d, %4d, 0x%.8x,\n",
                 strreg(p.name),
                 strreg(p.type),
                 flags);
    }

}

void Generator::generateEnums(int index)
{
    if (cdef->enumDeclarations.isEmpty())
        return;

    fprintf(out, "\n // enums: name, flags, count, data\n");
    index += 4 * cdef->enumList.count();
    int i;
    for (i = 0; i < cdef->enumList.count(); ++i) {
        const EnumDef &e = cdef->enumList.at(i);
        fprintf(out, "    %4d, 0x%.1x, %4d, %4d,\n",
                 strreg(e.name),
                 cdef->enumDeclarations.value(e.name) ? 1 : 0,
                 e.values.count(),
                 index);
        index += e.values.count() * 2;
    }

    fprintf(out, "\n // enum data: key, value\n");
    for (i = 0; i < cdef->enumList.count(); ++i) {
        const EnumDef &e = cdef->enumList.at(i);
        for (int j = 0; j < e.values.count(); ++j) {
            const QByteArray &val = e.values.at(j);
            fprintf(out, "    %4d, %s::%s,\n",
                    strreg(val),
                    cdef->qualified.constData(),
                    val.constData());
        }
    }
}

void Generator::generateMetacall()
{
    bool isQObject = (cdef->classname == "QObject");

    fprintf(out, "\nint %s::qt_metacall(QMetaObject::Call _c, int _id, void **_a)\n{\n",
             cdef->qualified.constData());

    if (cdef->signalList.size())
        fprintf(out, "    int _id_global = _id;\n");
    if (purestSuperClass.size() && !isQObject)
        fprintf(out, "    _id = %s::qt_metacall(_c, _id, _a);\n", purestSuperClass.constData());

    fprintf(out, "    if (_id < 0)\n        return _id;\n");
    fprintf(out, "    ");

    bool needElse = false;
    QList<FunctionDef> memberList;
    memberList += cdef->signalList;
    memberList += cdef->slotList;
    memberList += cdef->methodList;

    int signalCount = cdef->signalList.size();
    if (memberList.size()) {
        needElse = true;
        fprintf(out, "if (_c == QMetaObject::InvokeMetaMember) {\n        ");
        if (signalCount) {
            fprintf(out, "if (_id < %d)\n", signalCount);
            fprintf(out, "            QMetaObject::activate(this, _id_global, _a);\n");
        }
    }
    if (memberList.size() > signalCount) {
        if (signalCount)
            fprintf(out, "        else ");
        fprintf(out, "switch (_id) {\n");
        for (int memberindex = signalCount; memberindex < memberList.size(); ++memberindex) {
            const FunctionDef &f = memberList.at(memberindex);
            fprintf(out, "        case %d: ", memberindex);
            if (f.normalizedType.size())
                fprintf(out, "{ %s _r = ", noRef(f.normalizedType).constData());
            if (f.inPrivateClass.size())
                fprintf(out, "%s->", f.inPrivateClass.constData());
            fprintf(out, "%s(", f.name.constData());
            int offset = 1;
            for (int j = 0; j < f.arguments.count(); ++j) {
                const ArgumentDef &a = f.arguments.at(j);
                if (j)
                    fprintf(out, ",");
                fprintf(out, "*(%s*)_a[%d]",noRef(a.normalizedType).constData(), offset++);
            }
            fprintf(out, ");");
            if (f.normalizedType.size())
                fprintf(out, "\n            if (_a[0]) *(%s*)_a[0] = _r; } ",
                        noRef(f.normalizedType).constData());
            fprintf(out, " break;\n");
        }
        fprintf(out, "        }\n");
    }
    if (memberList.size())
        fprintf(out, "        _id -= %d;\n    }", memberList.size());

    if (cdef->propertyList.size()) {
        bool needGet = false;
        bool needSet = false;
        bool needReset = false;
        bool needDesignable = false;
        bool needScriptable = false;
        bool needStored = false;
        bool needEditable = false;
        for (int i = 0; i < cdef->propertyList.size(); ++i) {
            const PropertyDef &p = cdef->propertyList.at(i);
            needGet |= !p.read.isEmpty();
            needSet |= !p.write.isEmpty();
            needReset |= !p.reset.isEmpty();
            needDesignable |= p.designable.endsWith(')');
            needScriptable |= p.scriptable.endsWith(')');
            needStored |= p.stored.endsWith(')');
            needEditable |= p.editable.endsWith(')');
        }
        bool needAnything = needGet
                            | needSet
                            | needReset
                            | needDesignable
                            | needScriptable
                            | needStored
                            | needEditable;
        if (!needAnything)
            goto skip_properties;
        fprintf(out, "\n#ifndef QT_NO_PROPERTIES\n     ");

        if (needElse)
            fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::ReadProperty) {\n");
        if (needGet) {
            fprintf(out, "        void *_v = _a[0];\n");
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (p.read.isEmpty())
                    continue;
                if (p.gspec == PropertyDef::PointerSpec)
                    fprintf(out, "        case %d: _a[0] = (void*)%s(); break;\n",
                            propindex, p.read.constData());
                else if (p.gspec == PropertyDef::ReferenceSpec)
                    fprintf(out, "        case %d: _a[0] = (void*)&%s(); break;\n",
                            propindex, p.read.constData());
                else if (cdef->enumDeclarations.value(p.type, false))
                    fprintf(out, "        case %d: *(int*)_v = QFlag(%s()); break;\n",
                            propindex, p.read.constData());
                else
                    fprintf(out, "        case %d: *(%s*)_v = %s(); break;\n",
                            propindex, p.type.constData(), p.read.constData());
            }
            fprintf(out, "        }\n");
        }

        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::WriteProperty) {\n");

        if (needSet) {
            fprintf(out, "        void *_v = _a[0];\n");
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (p.write.isEmpty())
                    continue;
                if (cdef->enumDeclarations.value(p.type, false)) {
                    fprintf(out, "        case %d: %s(QFlag(*(int*)_v)); break;\n",
                            propindex, p.write.constData());
                } else {
                    fprintf(out, "        case %d: %s(*(%s*)_v); break;\n",
                            propindex, p.write.constData(), p.type.constData());
                }
            }
            fprintf(out, "        }\n");
        }

        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::ResetProperty) {\n");
        if (needReset) {
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (!p.reset.endsWith(')'))
                    continue;
                fprintf(out, "        case %d: %s; break;\n",
                        propindex, p.reset.constData());
            }
            fprintf(out, "        }\n");
        }
        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::QueryPropertyDesignable) {\n");
        if (needDesignable) {
            fprintf(out, "        bool *_b = (bool*)_a[0];\n");
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (!p.designable.endsWith(')'))
                    continue;
                fprintf(out, "        case %d: *_b = %s; break;\n",
                         propindex, p.designable.constData());
            }
            fprintf(out, "        }\n");
        }
        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::QueryPropertyScriptable) {\n");
        if (needScriptable) {
            fprintf(out, "        bool *_b = (bool*)_a[0];\n");
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (!p.scriptable.endsWith(')'))
                    continue;
                fprintf(out, "        case %d: *_b = %s; break;\n",
                         propindex, p.scriptable.constData());
            }
            fprintf(out, "        }\n");
        }
        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::QueryPropertyStored) {\n");
        if (needStored) {
            fprintf(out, "        bool *_b = (bool*)_a[0];\n");
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (!p.stored.endsWith(')'))
                    continue;
                fprintf(out, "        case %d: *_b = %s; break;\n",
                         propindex, p.stored.constData());
            }
            fprintf(out, "        }\n");
        }
        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::QueryPropertyEditable) {\n");
        if (needEditable) {
            fprintf(out, "        bool *_b = (bool*)_a[0];\n");
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (!p.editable.endsWith(')'))
                    continue;
                fprintf(out, "        case %d: *_b = %s; break;\n",
                         propindex, p.editable.constData());
            }
            fprintf(out, "        }\n");
        }
        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());


        fprintf(out, "\n#endif // QT_NO_PROPERTIES");
    }
 skip_properties:
    if (memberList.size() || cdef->signalList.size() || cdef->propertyList.size())
        fprintf(out, "\n    ");
    fprintf(out,"return _id;\n}\n");
}


void Generator::generateSignal(FunctionDef *def,int index)
{
    if (def->wasCloned)
        return;
    fprintf(out, "\n// SIGNAL %d\n%s %s::%s(",
            index, def->type.constData(), cdef->qualified.constData(), def->name.constData());


    if (def->arguments.isEmpty() && def->normalizedType.isEmpty()) {
        fprintf(out, ")\n{\n"
                "    QMetaObject::activate(this, &staticMetaObject, %d, 0);\n"
                "};\n", index);
        return;
    }

    int offset = 1;
    for (int j = 0; j < def->arguments.count(); ++j) {
        const ArgumentDef &a = def->arguments.at(j);
        if (j)
            fprintf(out, ", ");
        fprintf(out, "%s _t%d%s", a.type.constData(), offset++, a.rightType.constData());
    }
    fprintf(out, ")\n{\n");
    if (def->type.size() && def->normalizedType.size())
        fprintf(out, "    %s _t0;\n", noRef(def->normalizedType).constData());

    fprintf(out, "    void *_a[] = { %s",
             def->normalizedType.isEmpty() ? "0" : "(void*)&_t0");
    int i;
    for (i = 1; i < offset; ++i)
        fprintf(out, ", (void*)&_t%d", i);
    fprintf(out, " };\n");
    int n = 1;
    for (i = 0; i < def->arguments.count(); ++i)
        if (def->arguments.at(i).isDefault)
            ++n;
    for (i = 0; i < n; ++i)
        fprintf(out, "    QMetaObject::activate(this, &staticMetaObject, %d, _a);\n", index + i);
    if (def->normalizedType.size())
        fprintf(out, "    return _t0;\n");
    fprintf(out, "}\n");
}

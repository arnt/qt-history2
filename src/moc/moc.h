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

#ifndef MOC_H
#define MOC_H

#include "scanner.h"
#include <qstringlist.h>
#include <qmap.h>
#include <ctype.h>

struct EnumDef
{
    QByteArray name;
    QList<QByteArray> values;
};

struct ArgumentDef
{
    ArgumentDef():isDefault(false){}
    QByteArray type, rightType, normalizedType, name;
    bool isDefault;
};

struct FunctionDef
{
    FunctionDef(): access(Private), isConst(false), inlineCode(false), wasCloned(false), isCompat(false), isScriptable(false), noConnect(false) {}
    QByteArray type, normalizedType;
    QByteArray tag;
    QByteArray name;

    QList<ArgumentDef> arguments;

    enum Access { Private, Protected, Public };
    Access access;
    bool isConst;
    bool inlineCode;
    bool wasCloned;

    QByteArray inPrivateClass;
    bool isCompat;
    bool isScriptable;
    bool noConnect;
};

struct PropertyDef
{
    PropertyDef():gspec(ValueSpec), override(false){}
    QByteArray name, type, read, write, reset, designable, scriptable, editable, stored;
    enum Specification  { ValueSpec, ReferenceSpec, PointerSpec };
    Specification gspec;
    bool override;
    bool stdCppSet() const {
        QByteArray s("set");
        s += toupper(name[0]);
        s += name.mid(1);
        return (s == write);
    }
};


struct ClassInfoDef
{
    QByteArray name;
    QByteArray value;
};

struct ClassDef {
    ClassDef():
        hasQObject(false){}
    QByteArray classname;
    QByteArray qualified;
    QList<QByteArray> superclassList;
    QList<QList<QByteArray> >interfaceList;

    bool hasQObject;

    QList<FunctionDef> signalList, slotList, publicList;
    QList<PropertyDef> propertyList;
    QList<ClassInfoDef> classInfoList;
    QMap<QByteArray, bool> enumDeclarations;
    QList<EnumDef> enumList;

    int begin;
    int end;
};

struct NamespaceDef {
    QByteArray name;
    int begin;
    int end;
};

class Moc
{
public:
    Moc()
        :index(0),
         noInclude(false),
         displayWarnings(true),
         generatedCode(false)
        {}

    QByteArray filename;
    Symbols symbols;

    int index;

    bool noInclude;
    bool displayWarnings;
    bool generatedCode;
    QByteArray includePath;
    QList<QByteArray> includeFiles;
    QList<ClassDef> classList;

    inline bool hasNext() const { return (index < symbols.size()); }
    inline Token next() { return symbols.at(index++).token; }
    bool test(Token);
    void next(Token);
    void next(Token, const char *msg);
    bool until(Token);
    QByteArray lexemUntil(Token);
    inline void prev() {--index;}
    Token lookup(int k = 1);
    inline const Symbol &symbol_lookup(int k = 1) { return symbols.at(index-1+k);}
    inline Token token() { return symbols.at(index-1).token;}
    inline QByteArray lexem() { return symbols.at(index-1).lexem();}
    inline const Symbol &symbol() { return symbols.at(index-1);}

    void setErrorMessage(const char *);
    void error(int rollback);
    void error(const char *msg = 0);
    void warning(const char * = 0);

    void parse();
    void generate(FILE *out);

    bool parseClassHead(ClassDef *def);
    inline bool inClass(const ClassDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inNamespace(const NamespaceDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    QByteArray parseType();

    bool parseEnum(EnumDef *def);

    void parseFunction(FunctionDef *def, bool inMacro = false);
    bool parsePublicFunction(FunctionDef *def);

    void parseSlots(ClassDef *def, FunctionDef::Access access);
    void parseSignals(ClassDef *def);
    void parseProperty(ClassDef *def, bool override);
    void parseEnumOrFlag(ClassDef *def, bool isFlag);
    void parseClassInfo(ClassDef *def);
    void parseInterfaces(ClassDef *def);
    void parseSlot(ClassDef *def, FunctionDef::Access access);
    void parseSlotInPrivate(ClassDef *def, FunctionDef::Access access);

    void parseFunctionArguments(FunctionDef *def);

};


inline bool Moc::test(Token token)
{
    if (index < symbols.size() && symbols.at(index).token == token) {
        ++index;
        return true;
    }
    return false;
}

inline Token Moc::lookup(int k)
{
    const int l = index - 1 + k;
    return l < symbols.size() ? symbols.at(l).token : NOTOKEN;
}

inline void Moc::next(Token token)
{
    if (!test(token))
        error();
}

inline void Moc::next(Token token, const char *msg)
{
    if (!test(token))
        error(msg);
}

QByteArray normalizeType(const char *s, bool fixScope = false);

#endif

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

#include "moc.h"
#include "generator.h"
#include "qdatetime.h"
#include "utils.h"
#include "outputrevision.h"
#include <stdio.h>

// WARNING: a copy of this function is in qmetaobject.cpp
static QByteArray normalizeTypeInternal(const char *t, const char *e, bool fixScope = true, bool adjustConst = true)
{
    int len = e - t;
    if (strncmp("void", t, len) == 0)
        return QByteArray();
    /*
      Convert 'char const *' into 'const char *'. Start at index 1,
      not 0, because 'const char *' is already OK.
    */
    QByteArray constbuf;
    for (int i = 1; i < len; i++) {
        if ( t[i] == 'c'
             && strncmp(t + i + 1, "onst", 4) == 0
             && (i + 5 >= len || !is_ident_char(t[i + 5]))
             && !is_ident_char(t[i-1])
            ) {
            constbuf = QByteArray(t, len);
            if (t[i-1] == ' ')
                constbuf.remove(i-1, 6);
            else
                constbuf.remove(i, 5);
            constbuf.prepend("const ");
            t = constbuf.data();
            e = constbuf.data() + constbuf.length();
            break;
        }
        /*
          We musn't convert 'char * const *' into 'const char **'
          and we must beware of 'Bar<const Bla>'.
        */
        if (t[i] == '&' || t[i] == '*' ||t[i] == '<')
            break;
    }
    if (adjustConst && e > t + 6 && strncmp("const ", t, 6) == 0) {
        if (*(e-1) == '&') { // treat const reference as value
            t += 6;
            --e;
        } else if (is_ident_char(*(e-1))) { // treat const value as value
            t += 6;
        }
    }
    QByteArray result;
    result.reserve(len);

    // some type substitutions for 'unsigned x'
    if (strncmp("unsigned ", t, 9) == 0) {
        if (strncmp("int", t+9, 3) == 0) {
            t += 9+3;
            result += "uint";
        } else if (strncmp("long", t+9, 4) == 0) {
            t += 9+4;
            result += "ulong";
        }
    }

    while (t != e) {
        char c = *t++;
        if (fixScope && c == ':' && *t == ':' ) {
            ++t;
            c = *t++;
            int i = result.size() - 1;
            while (i >= 0 && is_ident_char(result.at(i)))
                   --i;
            result.resize(i + 1);
        }
        result += c;
        if (c == '<') {
            //template recursion
            const char* tt = t;
            int templdepth = 1;
            while (t != e) {
                c = *t++;
                if (c == '<')
                    ++templdepth;
                if (c == '>')
                    --templdepth;
                if (templdepth == 0) {
                    result += normalizeTypeInternal(tt, t-1, fixScope, false);
                    result += c;
                    if (*t == '>')
                        result += ' '; // avoid >>
                    break;
                }
            }
        }
    }

    return result;
}

// only moc needs this function
QByteArray normalizeType(const char *s, bool fixScope)
{
    int len = qstrlen(s);
    char stackbuf[64];
    char *buf = (len >= 64 ? new char[len] : stackbuf);
    char *d = buf;
    char last = 0;
    while(*s && is_space(*s))
        s++;
    while (*s) {
        while (*s && !is_space(*s))
            last = *d++ = *s++;
        while (*s && is_space(*s))
            s++;
        if (*s && is_ident_char(*s) && is_ident_char(last))
            last = *d++ = ' ';
    }
    *d = '\0';
    QByteArray result = normalizeTypeInternal(buf, d, fixScope);
    if (buf != stackbuf)
        delete [] buf;
    return result;
}

static const char *error_msg = 0;

#ifdef Q_CC_MSVC
#define ErrorFormatString "%s(%d): "
#else
#define ErrorFormatString "%s:%d: "
#endif

void Moc::error(int rollback) {
    index -= rollback;
    error();
}
void Moc::error(const char *msg) {
    if (msg || error_msg)
        qFatal(ErrorFormatString "Error: %s",
               filename.constData(), symbol().lineNum, msg?msg:error_msg);
    else
        qFatal(ErrorFormatString "Parse error at \"%s\"",
               filename.constData(), symbol().lineNum, symbol().lexem().data());
}

void Moc::warning(const char *msg) {
    if (displayWarnings && msg)
        fprintf(stderr, ErrorFormatString "Warning: %s\n",
                filename.constData(), qMax(0, symbol().lineNum), msg);
}

bool Moc::until(Token target) {
    int braceCount = 0;
    int brackCount = 0;
    int parenCount = 0;
    int angleCount = 0;
    if (index) {
        switch(symbols.at(index-1).token) {
        case LBRACE: ++braceCount; break;
        case LBRACK: ++brackCount; break;
        case LPAREN: ++parenCount; break;
        case LANGLE: ++angleCount; break;
        default: ;
        }
    }
    while (index < symbols.size()) {
        Token t = symbols.at(index++).token;
        switch (t) {
        case LBRACE: ++braceCount; break;
        case RBRACE: --braceCount; break;
        case LBRACK: ++brackCount; break;
        case RBRACK: --brackCount; break;
        case LPAREN: ++parenCount; break;
        case RPAREN: --parenCount; break;
        case LANGLE: ++angleCount; break;
        case RANGLE: --angleCount; break;
        default: break;
        }
        if (t == target
            && braceCount <= 0
            && brackCount <= 0
            && parenCount <= 0
            && (target != RANGLE || angleCount <= 0))
            return true;

        if (braceCount < 0 || brackCount < 0 || parenCount < 0
            || (target == RANGLE && angleCount < 0)) {
            --index;
            break;
        }
    }
    return false;
}

QByteArray Moc::lexemUntil(Token target)
{
    int from = index;
    until(target);
    QByteArray s;
    while (from <= index)
        s += symbols.at(from++-1).lexem();
    return s;
}

bool Moc::parseClassHead(ClassDef *def)
{
    // figure out whether this is a class declaration, or only a
    // forward or variable declaration.
    int i = 0;
    Token token;
    do {
        token = lookup(i++);
        if (token == COLON || token == LBRACE)
            break;
        if (token == SEMIC || token == RANGLE)
            return false;
    } while (token);

    next(IDENTIFIER);
    QByteArray name = lexem();

    // support "class IDENT name" and "class IDENT(IDENT) name"
    if (test(LPAREN)) {
        until(RPAREN);
        next(IDENTIFIER);
        name = lexem();
    } else  if (test(IDENTIFIER)) {
        name = lexem();
    }

    def->qualified += name;
    while (test(SCOPE)) {
        def->qualified += lexem();
        if (test(IDENTIFIER)) {
            name = lexem();
            def->qualified += name;
        }
    }
    def->classname = name;
    if (test(COLON)) {
        do {
            test(VIRTUAL);
            FunctionDef::Access access = FunctionDef::Public;
            if (test(PRIVATE))
                access = FunctionDef::Private;
            else if (test(PROTECTED))
                access = FunctionDef::Protected;
            else
                test(PUBLIC);
            test(VIRTUAL);
            def->superclassList += qMakePair(parseType(), access);
        } while (test(COMMA));
    }
    next(LBRACE);
    def->begin = index - 1;
    until(RBRACE);
    def->end = index ;
    index = def->begin + 1;
    return true;
};

QByteArray Moc::parseType()
{
    QByteArray s;
    while (test(CONST) || test(VOLATILE) || test(SIGNED) || test(UNSIGNED)) {
        s += lexem();
        s += ' ';
    }
    test(ENUM) || test(CLASS) || test(STRUCT);
    for(;;) {
        switch (next()) {
        case IDENTIFIER:
        case CHAR:
        case SHORT:
        case INT:
        case LONG:
        case FLOAT:
        case DOUBLE:
        case VOID:
        case BOOL:
            s += lexem();
            break;
        default:
            prev();
            ;
        }
        if (test(LANGLE)) {
            QByteArray templ = lexemUntil(RANGLE);
            for (int i = 0; i < templ.size(); ++i) {
                s += templ.at(i);
                if (templ.at(i) == '>' && i < templ.size()-1 && templ.at(i+1) == '>')
                    s += ' ';
            }
        }
        if (test(SCOPE))
            s += lexem();
        else
            break;
    }
    while (test(CONST) || test(VOLATILE) || test(SIGNED) || test(UNSIGNED)
           || test(STAR) || test(AND)) {
        s += ' ';
        s += lexem();
    }
    return s;
}

bool Moc::parseEnum(EnumDef *def)
{
    if (!test(IDENTIFIER))
        return false; // anonymous enum
    def->name = lexem();
    if (!test(LBRACE))
        return false;
    do {
        next(IDENTIFIER);
        def->values += lexem();
    } while (test(EQ) ? until(COMMA) : test(COMMA));
    next(RBRACE);
    return true;
}

void Moc::parseFunctionArguments(FunctionDef *def)
{
    Q_UNUSED(def);
    while (hasNext()) {
        ArgumentDef  arg;
        arg.type = parseType();
        if (arg.type == "void")
            break;
        if (test(IDENTIFIER))
            arg.name = lexem();
        if (test(LBRACK))
            arg.rightType += lexemUntil(RBRACK);
        if (test(CONST) || test(VOLATILE)) {
            arg.rightType += ' ';
            arg.rightType += lexem();
        }
        arg.normalizedType = normalizeType(arg.type + ' ' + arg.rightType);
        if (test(EQ))
            arg.isDefault = true;
        def->arguments += arg;
        if (!until(COMMA))
            break;
    }
}

void Moc::parseFunction(FunctionDef *def, bool inMacro)
{
    def->isVirtual = test(VIRTUAL);
    while (test(INLINE) || test(STATIC))
        ;
    bool templateFunction = (lookup() == TEMPLATE);
    def->type = parseType();
    if (def->type.isEmpty()) {
        if (templateFunction)
            error("Template function as signal or slot");
        else
            error();
}
    if (test(LPAREN)) {
        def->name = def->type;
        def->type = "int";
    } else {
        def->name = parseType();
        while (!def->name.isEmpty() && lookup() != LPAREN) {
            if (def->type == "QT_MOC_COMPAT" || def->type == "QT3_SUPPORT")
                def->isCompat = true;
            else if (def->type == "Q_INVOKABLE")
                def->isInvokable = true;
            else if (def->type == "Q_SCRIPTABLE")
                def->isInvokable = def->isScriptable = true;
            else {
                if (!def->tag.isEmpty())
                    def->tag += ' ';
                def->tag += def->type;
            }
            def->type = def->name;
            def->name = parseType();
        }
        next(LPAREN, "Not a signal or slot declaration");
    }

    def->normalizedType = normalizeType(def->type);

    if (!test(RPAREN)) {
        parseFunctionArguments(def);
        next(RPAREN);
    }
    def->isConst = test(CONST);
    if (inMacro) {
        next(RPAREN);
    } else {
        if (test(SEMIC))
            ;
        else if ((def->inlineCode = test(LBRACE)))
            until(RBRACE);
        else if (test(EQ))
            until(SEMIC);
        else
            error();
    }
}

// like parseFunction, but never aborts with an error
bool Moc::parseMaybeFunction(FunctionDef *def)
{
    def->type = parseType();
    if (def->type.isEmpty())
        return false;
    if (test(LPAREN)) {
        def->name = def->type;
        def->type = "int";
    } else {
        def->name = parseType();
        while (!def->name.isEmpty() && lookup() != LPAREN) {
            if (def->type == "QT_MOC_COMPAT" || def->type == "QT3_SUPPORT")
                def->isCompat = true;
            else if (def->type == "Q_INVOKABLE")
                def->isInvokable = true;
            else if (def->type == "Q_SCRIPTABLE")
                def->isInvokable = def->isScriptable = true;
            else {
                if (!def->tag.isEmpty())
                    def->tag += ' ';
                def->tag += def->type;
            }
            def->type = def->name;
            def->name = parseType();
        }
        if (!test(LPAREN))
            return false;
    }

    def->normalizedType = normalizeType(def->type);

    if (!test(RPAREN)) {
        parseFunctionArguments(def);
        if (!test(RPAREN))
            return false;
    }
    def->isConst = test(CONST);
    return true;
}


void Moc::parse()
{
    QList<NamespaceDef> namespaceList;
    bool templateClass = false;
    while (hasNext()) {
        Token t = next();
        if (t == NAMESPACE) {
            int rewind = index;
            if (test(IDENTIFIER) && !test(SEMIC)) {
                NamespaceDef def;
                def.name = lexem();
                next(LBRACE);
                def.begin = index - 1;
                until(RBRACE);
                def.end = index;
                index = def.begin + 1;
                namespaceList += def;
            }
            index = rewind;
        } else if (t == SEMIC || t == RBRACE) {
            templateClass = false;
        } else if (t == TEMPLATE) {
            templateClass = true;
        }
        if (t != CLASS)
            continue;
        ClassDef def;
        FunctionDef::Access access = FunctionDef::Private;
        if (parseClassHead(&def)) {
            for (int i = namespaceList.size() - 1; i >= 0; --i)
                if (inNamespace(&namespaceList.at(i)))
                    def.qualified.prepend(namespaceList.at(i).name + "::");
            while (inClass(&def) && hasNext()) {
                switch ((t = next())) {
                case PRIVATE:
                    access = FunctionDef::Private;
                    if (test(SIGNALS))
                        error("Signals cannot have access specifier");
                    break;
                case PROTECTED:
                    access = FunctionDef::Protected;
                    if (test(SIGNALS))
                        error("Signals cannot have access specifier");
                    break;
                case PUBLIC:
                    access = FunctionDef::Public;
                    if (test(SIGNALS))
                        error("Signals cannot have access specifier");
                    break;
                case CLASS: {
                    ClassDef nestedDef;
                    if (parseClassHead(&nestedDef)) {
                        while (inClass(&nestedDef) && inClass(&def)) {
                            t = next();
                            if (t >= Q_META_TOKEN_BEGIN && t < Q_META_TOKEN_END)
                                error("Meta object features not supported for nested classes");
                        }
                    }
                } break;
                case SIGNALS:
                    parseSignals(&def);
                    break;
                case SLOTS:
                    switch (lookup(-1)) {
                    case PUBLIC:
                    case PROTECTED:
                    case PRIVATE:
                        parseSlots(&def, access);
                        break;
                    default:
                        error("Missing access specifier for slots");
                    }
                    break;
                case Q_OBJECT_TOKEN:
                    def.hasQObject = true;
                    if (templateClass)
                        error("Template classes not supported by Q_OBJECT");
                    if (def.classname != "Qt" && def.classname != "QObject" && def.superclassList.isEmpty())
                        error("Class contains Q_OBJECT macro but does not inherit from QObject");
                    break;
                case Q_GADGET_TOKEN:
                    def.hasQGadget = true;
                    if (templateClass)
                        error("Template classes not supported by Q_GADGET");
                    break;
                case Q_PROPERTY_TOKEN:
                    parseProperty(&def, false);
                    break;
                case Q_OVERRIDE_TOKEN:
                    parseProperty(&def, true);
                    break;
                case Q_ENUMS_TOKEN:
                    parseEnumOrFlag(&def, false);
                    break;
                case Q_FLAGS_TOKEN:
                    parseEnumOrFlag(&def, true);
                    break;
                case Q_DECLARE_FLAGS_TOKEN:
                    parseFlag(&def);
                    break;
                case Q_CLASSINFO_TOKEN:
                    parseClassInfo(&def);
                    break;
                case Q_INTERFACES_TOKEN:
                    parseInterfaces(&def);
                    break;
                case Q_PRIVATE_SLOT_TOKEN:
                    parseSlotInPrivate(&def, access);
                    break;
                case ENUM: {
                    EnumDef enumDef;
                    if (parseEnum(&enumDef))
                        def.enumList += enumDef;
                } break;
                default:
                    FunctionDef funcDef;
                    funcDef.access = access;
                    int rewind = index;
                    if (parseMaybeFunction(&funcDef)) {
                        if (access == FunctionDef::Public)
                            def.publicList += funcDef;
                        if (funcDef.isInvokable) {
                            def.methodList += funcDef;
                            while (funcDef.arguments.size() > 0 && funcDef.arguments.last().isDefault) {
                                funcDef.wasCloned = true;
                                funcDef.arguments.removeLast();
                                def.methodList += funcDef;
                            }
                        }
                    } else {
                        index = rewind;
                    }
                }
            }

            next(RBRACE);

            if (!def.hasQObject && def.signalList.isEmpty() && def.slotList.isEmpty()
                && def.propertyList.isEmpty() && def.enumDeclarations.isEmpty())
                continue; // no meta object code required


            if (!def.hasQObject && !def.hasQGadget)
                error("Class declarations lacks Q_OBJECT macro.");

            classList += def;
        }
    }

}

void Moc::generate(FILE *out)
{

    QDateTime dt = QDateTime::currentDateTime();
    QByteArray dstr = dt.toString().toLatin1();
    QByteArray fn = filename;
    int i = filename.length()-1;
    while (i>0 && filename[i-1] != '/' && filename[i-1] != '\\')
        --i;                                // skip path
    if (i >= 0)
        fn = filename.mid(i);
    fprintf(out, "/****************************************************************************\n"
            "** Meta object code from reading C++ file '%s'\n**\n" , (const char*)fn);
    fprintf(out, "** Created: %s\n"
            "**      by: The Qt Meta Object Compiler version %d (Qt %s)\n**\n" , dstr.data(), mocOutputRevision, QT_VERSION_STR);
    fprintf(out, "** WARNING! All changes made in this file will be lost!\n"
            "*****************************************************************************/\n\n");


    if (!noInclude) {
        if (includePath.size() && includePath.right(1) != "/")
            includePath += "/";
        for (int i = 0; i < includeFiles.size(); ++i) {
            QByteArray inc = includeFiles.at(i);
            if (inc[0] != '<' && inc[0] != '"') {
                if (includePath.size() && includePath != "./")
                    inc.prepend(includePath);
                inc = "\"" + inc + "\"";
            }
            fprintf(out, "#include %s\n", inc.constData());
        }
    }
    if (classList.size() && classList.first().classname == "Qt")
        fprintf(out, "#include <QtCore/qobject.h>\n");

    fprintf(out, "#if !defined(Q_MOC_OUTPUT_REVISION)\n"
            "#error \"The header file '%s' doesn't include <QObject>.\"\n", (const char *)fn);
    fprintf(out, "#elif Q_MOC_OUTPUT_REVISION != %d\n", mocOutputRevision);
    fprintf(out, "#error \"This file was generated using the moc from %s."
            " It\"\n#error \"cannot be used with the include files from"
            " this version of Qt.\"\n#error \"(The moc has changed too"
            " much.)\"\n", QT_VERSION_STR);
    fprintf(out, "#endif\n\n");


    for (i = 0; i < classList.size(); ++i) {
        Generator generator(out, &classList[i]);
        generator.generateCode();
    }
}



void Moc::parseSlots(ClassDef *def, FunctionDef::Access access)
{
    next(COLON);
    while (inClass(def) && hasNext()) {
        switch (next()) {
        case PUBLIC:
        case PROTECTED:
        case PRIVATE:
        case SIGNALS:
        case SLOTS:
            prev();
            return;
        case SEMIC:
            continue;
        default:
            prev();
        }

        FunctionDef funcDef;
        funcDef.access = access;
        parseFunction(&funcDef);
        def->slotList += funcDef;
        while (funcDef.arguments.size() > 0 && funcDef.arguments.last().isDefault) {
            funcDef.wasCloned = true;
            funcDef.arguments.removeLast();
            def->slotList += funcDef;
        }
    }
}

void Moc::parseSignals(ClassDef *def)
{
    next(COLON);
    while (inClass(def) && hasNext()) {
        switch (next()) {
        case PUBLIC:
        case PROTECTED:
        case PRIVATE:
        case SIGNALS:
        case SLOTS:
            prev();
            return;
        case SEMIC:
            continue;
        default:
            prev();
        }
        FunctionDef funcDef;
        funcDef.access = FunctionDef::Protected;
        parseFunction(&funcDef);
        if (funcDef.isVirtual)
            error("Signals cannot be declared virtual");
        if (funcDef.isConst)
            error("Signals cannot have const qualifier");
        if (funcDef.inlineCode)
            error("Not a signal declaration");
        def->signalList += funcDef;
        while (funcDef.arguments.size() > 0 && funcDef.arguments.last().isDefault) {
            funcDef.wasCloned = true;
            funcDef.arguments.removeLast();
            def->signalList += funcDef;
        }
    }
}


void Moc::parseProperty(ClassDef *def, bool override)
{
    next(LPAREN);
    PropertyDef propDef;
    propDef.override = override;
    QByteArray type = parseType();
    if (type.isEmpty())
        error();
    if (!override)
        propDef.designable = propDef.scriptable = propDef.stored = "true";
    /*
      The Q_PROPERTY construct cannot contain any commas, since
      commas separate macro arguments. We therefore expect users
      to type "QMap" instead of "QMap<QString, QVariant>". For
      coherence, we also expect the same for
      QValueList<QVariant>, the other template class supported by
      QVariant.
    */
    type = normalizeType(type);
    if (type == "QMap")
        type = "QMap<QString,QVariant>";
    else if (type == "QValueList")
        type = "QValueList<QVariant>";
    else if (type == "LongLong")
        type = "qint64";
    else if (type == "ULongLong")
        type = "quint64";
    propDef.type = type;

    next();
    propDef.name = lexem();
    while (test(IDENTIFIER)) {
        QByteArray l = lexem();
        QByteArray v, v2;
        if (test(LPAREN)) {
            v = lexemUntil(RPAREN);
        } else {
            next(IDENTIFIER);
            v = lexem();
            if (test(LPAREN))
                v2 = lexemUntil(RPAREN);
            else if (v != "true" && v != "false")
                v2 = "()";
        }
        switch (l[0]) {
        case 'R':
            if (l == "READ")
                propDef.read = v;
            else if (l == "RESET")
                propDef.reset = v + v2;
            else
                error(2);
            break;
        case 'S':
            if (l == "SCRIPTABLE")
                propDef.scriptable = v + v2;
            else if (l == "STORED")
                propDef.stored = v + v2;
            else
                error(2);
            break;
        case 'W': if (l != "WRITE") error(2);
            propDef.write = v;
            break;
        case 'D': if (l != "DESIGNABLE") error(2);
            propDef.designable = v + v2;
            break;
        case 'E': if (l != "EDITABLE") error(2);
            propDef.editable = v + v2;
            break;
        default:
            error(2);
        }
    }
    next(RPAREN);
    def->propertyList += propDef;
}

void Moc::parseEnumOrFlag(ClassDef *def, bool isFlag)
{
    next(LPAREN);
    QByteArray identifier;
    while (test(IDENTIFIER)) {
        identifier = lexem();
        while (test(SCOPE) && test(IDENTIFIER)) {
            identifier += "::";
            identifier += lexem();
        }
        def->enumDeclarations[identifier] = isFlag;
    }
    next(RPAREN);
}

void Moc::parseFlag(ClassDef *def)
{
    next(LPAREN);
    QByteArray flagName, enumName;
    while (test(IDENTIFIER)) {
        flagName = lexem();
        while (test(SCOPE) && test(IDENTIFIER)) {
            flagName += "::";
            flagName += lexem();
        }
    }
    next(COMMA);
    while (test(IDENTIFIER)) {
        enumName = lexem();
        while (test(SCOPE) && test(IDENTIFIER)) {
            enumName += "::";
            enumName += lexem();
        }
    }

    def->flagAliases.insert(enumName, flagName);
    next(RPAREN);
}

void Moc::parseClassInfo(ClassDef *def)
{
    next(LPAREN);
    ClassInfoDef infoDef;
    next(STRING_LITERAL);
    const Symbol *sym = &symbol();
    infoDef.name = sym->lexem_data.mid(sym->from+1, sym->len-2); // cut away quotes
    next(COMMA);
    next(STRING_LITERAL);
    sym = &symbol();
    infoDef.value = sym->lexem_data.mid(sym->from+1, sym->len-2);  // cut away quotes
    def->classInfoList += infoDef;
}

void Moc::parseInterfaces(ClassDef *def)
{
    next(LPAREN);
    while (test(IDENTIFIER)) {
        QList<QByteArray> iface;
        iface += lexem();
        while (test(COLON)) {
            next(IDENTIFIER);
            iface += lexem();
        }
        def->interfaceList += iface;
    }
    next(RPAREN);

}

void Moc::parseSlotInPrivate(ClassDef *def, FunctionDef::Access access)
{
    next(LPAREN);
    FunctionDef funcDef;
    next(IDENTIFIER);
    funcDef.inPrivateClass = lexem();
    next(COMMA);
    funcDef.access = access;
    parseFunction(&funcDef, true);
    def->slotList += funcDef;
    while (funcDef.arguments.size() > 0 && funcDef.arguments.last().isDefault) {
        funcDef.wasCloned = true;
        funcDef.arguments.removeLast();
        def->slotList += funcDef;
    }
}


#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamReader>
#include <QtCore/QVector>
#include "../global.h"

Q_DECLARE_METATYPE(QXmlStreamReader*)
Q_DECLARE_METATYPE(QScript::Wrapper<QXmlStreamReader*>::pointer_type)
Q_DECLARE_METATYPE(QIODevice*)
Q_DECLARE_METATYPE(QByteArray*)
Q_DECLARE_METATYPE(QXmlStreamAttribute)
Q_DECLARE_METATYPE(QVector<QXmlStreamAttribute>)
Q_DECLARE_METATYPE(QXmlStreamEntityDeclaration)
Q_DECLARE_METATYPE(QVector<QXmlStreamEntityDeclaration>)
Q_DECLARE_METATYPE(QXmlStreamNamespaceDeclaration)
Q_DECLARE_METATYPE(QVector<QXmlStreamNamespaceDeclaration>)
Q_DECLARE_METATYPE(QXmlStreamNotationDeclaration)
Q_DECLARE_METATYPE(QVector<QXmlStreamNotationDeclaration>)
Q_DECLARE_METATYPE(QStringRef)

static inline QScriptValue newXmlStreamReader(QScriptEngine *eng, QXmlStreamReader *reader)
{
    return eng->newVariant(qVariantFromValue(QScript::Wrapper<QXmlStreamReader*>::wrap(reader)));
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newXmlStreamReader(eng, new QXmlStreamReader());
    QScriptValue arg = ctx->argument(0);
    if (QIODevice *device = qscriptvalue_cast<QIODevice*>(arg)) {
        return newXmlStreamReader(eng, new QXmlStreamReader(device));
    } else if (QByteArray *ba = qscriptvalue_cast<QByteArray*>(arg)) {
        return newXmlStreamReader(eng, new QXmlStreamReader(*ba));
    } else {
        return newXmlStreamReader(eng, new QXmlStreamReader(arg.toString()));
    }
}

static QScriptValue addData(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, addData);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *ba = qscriptvalue_cast<QByteArray*>(arg))
        self->addData(*ba);
    else
        self->addData(arg.toString());
    return eng->undefinedValue();
}

static QScriptValue atEnd(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, atEnd);
    return QScriptValue(eng, self->atEnd());
}

static QScriptValue attributes(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, attributes);
    return eng->toScriptValue(static_cast<QVector<QXmlStreamAttribute> >(self->attributes()));
}

static QScriptValue characterOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, characterOffset);
    return QScriptValue(eng, qsreal(self->characterOffset()));
}

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, clear);
    self->clear();
    return eng->undefinedValue();
}

static QScriptValue columnNumber(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, columnNumber);
    return QScriptValue(eng, qsreal(self->columnNumber()));
}

static QScriptValue device(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, device);
    return eng->toScriptValue(self->device());
}

static QScriptValue entityDeclarations(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, entityDeclarations);
    return eng->toScriptValue(self->entityDeclarations());
}

static QScriptValue error(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, error);
    return QScriptValue(eng, self->error());
}

static QScriptValue errorString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, errorString);
    return QScriptValue(eng, self->errorString());
}

static QScriptValue hasError(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, hasError);
    return QScriptValue(eng, self->hasError());
}

static QScriptValue isCDATA(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isCDATA);
    return QScriptValue(eng, self->isCDATA());
}

static QScriptValue isCharacters(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isCharacters);
    return QScriptValue(eng, self->isCharacters());
}

static QScriptValue isComment(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isComment);
    return QScriptValue(eng, self->isComment());
}

static QScriptValue isDTD(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isDTD);
    return QScriptValue(eng, self->isDTD());
}

static QScriptValue isEndDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isEndDocument);
    return QScriptValue(eng, self->isEndDocument());
}

static QScriptValue isEndElement(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isEndElement);
    return QScriptValue(eng, self->isEndElement());
}

static QScriptValue isEntityReference(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isEntityReference);
    return QScriptValue(eng, self->isEntityReference());
}

static QScriptValue isProcessingInstruction(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isProcessingInstruction);
    return QScriptValue(eng, self->isProcessingInstruction());
}

static QScriptValue isStandaloneDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isStandaloneDocument);
    return QScriptValue(eng, self->isStandaloneDocument());
}

static QScriptValue isStartDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isStartDocument);
    return QScriptValue(eng, self->isStartDocument());
}

static QScriptValue isStartElement(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isStartElement);
    return QScriptValue(eng, self->isStartElement());
}

static QScriptValue isWhitespace(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, isWhitespace);
    return QScriptValue(eng, self->isWhitespace());
}

static QScriptValue lineNumber(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, lineNumber);
    return QScriptValue(eng, qsreal(self->lineNumber()));
}

static QScriptValue name(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, name);
    return eng->toScriptValue(self->name());
}

static QScriptValue namespaceDeclarations(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, namespaceDeclarations);
    return eng->toScriptValue(self->namespaceDeclarations());
}

static QScriptValue namespaceProcessing(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, namespaceProcessing);
    return eng->toScriptValue(self->namespaceProcessing());
}

static QScriptValue namespaceUri(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, namespaceUri);
    return eng->toScriptValue(self->namespaceUri());
}

static QScriptValue notationDeclarations(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, notationDeclarations);
    return eng->toScriptValue(self->notationDeclarations());
}

static QScriptValue processingInstructionData(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, processingInstructionData);
    return eng->toScriptValue(self->processingInstructionData());
}

static QScriptValue processingInstructionTarget(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, processingInstructionTarget);
    return eng->toScriptValue(self->processingInstructionTarget());
}

static QScriptValue qualifiedName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, qualifiedName);
    return eng->toScriptValue(self->qualifiedName());
}

static QScriptValue raiseError(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, raiseError);
    self->raiseError(ctx->argument(0).toString());
    return eng->undefinedValue();
}

static QScriptValue readElementText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, readElementText);
    return QScriptValue(eng, self->readElementText());
}

static QScriptValue readNext(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, readNext);
    return QScriptValue(eng, self->readNext());
}

static QScriptValue setDevice(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, setDevice);
    self->setDevice(qscriptvalue_cast<QIODevice*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setNamespaceProcessing(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, setNamespaceProcessing);
    self->setNamespaceProcessing(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, text);
    return eng->toScriptValue(self->text());
}

static QScriptValue tokenString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, tokenString);
    return QScriptValue(eng, self->tokenString());
}

static QScriptValue tokenType(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, tokenType);
    return QScriptValue(eng, self->tokenType());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamReader, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamReader"));
}

QScriptValue constructXmlStreamReaderClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamReader(eng, new QXmlStreamReader());
    ADD_PROTO_FUNCTION(proto, addData);
    ADD_PROTO_FUNCTION(proto, atEnd);
    ADD_PROTO_FUNCTION(proto, attributes);
    ADD_PROTO_FUNCTION(proto, characterOffset);
    ADD_PROTO_FUNCTION(proto, clear);
    ADD_PROTO_FUNCTION(proto, columnNumber);
    ADD_PROTO_FUNCTION(proto, device);
    ADD_PROTO_FUNCTION(proto, entityDeclarations);
    ADD_PROTO_FUNCTION(proto, error);
    ADD_PROTO_FUNCTION(proto, errorString);
    ADD_PROTO_FUNCTION(proto, hasError);
    ADD_PROTO_FUNCTION(proto, isCDATA);
    ADD_PROTO_FUNCTION(proto, isCharacters);
    ADD_PROTO_FUNCTION(proto, isComment);
    ADD_PROTO_FUNCTION(proto, isDTD);
    ADD_PROTO_FUNCTION(proto, isEndDocument);
    ADD_PROTO_FUNCTION(proto, isEndElement);
    ADD_PROTO_FUNCTION(proto, isEntityReference);
    ADD_PROTO_FUNCTION(proto, isProcessingInstruction);
    ADD_PROTO_FUNCTION(proto, isStandaloneDocument);
    ADD_PROTO_FUNCTION(proto, isStartElement);
    ADD_PROTO_FUNCTION(proto, isWhitespace);
    ADD_PROTO_FUNCTION(proto, lineNumber);
    ADD_PROTO_FUNCTION(proto, name);
    ADD_PROTO_FUNCTION(proto, namespaceDeclarations);
    ADD_PROTO_FUNCTION(proto, namespaceProcessing);
    ADD_PROTO_FUNCTION(proto, namespaceUri);
    ADD_PROTO_FUNCTION(proto, notationDeclarations);
    ADD_PROTO_FUNCTION(proto, processingInstructionData);
    ADD_PROTO_FUNCTION(proto, processingInstructionTarget);
    ADD_PROTO_FUNCTION(proto, qualifiedName);
    ADD_PROTO_FUNCTION(proto, raiseError);
    ADD_PROTO_FUNCTION(proto, readElementText);
    ADD_PROTO_FUNCTION(proto, readNext);
    ADD_PROTO_FUNCTION(proto, setDevice);
    ADD_PROTO_FUNCTION(proto, setNamespaceProcessing);
    ADD_PROTO_FUNCTION(proto, text);
    ADD_PROTO_FUNCTION(proto, tokenString);
    ADD_PROTO_FUNCTION(proto, tokenType);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerMetaTypeWrapper<QScript::Wrapper<QXmlStreamReader*> >(eng, proto);
    qScriptRegisterSequenceMetaType<QVector<QXmlStreamAttribute> >(eng);
    qScriptRegisterSequenceMetaType<QVector<QXmlStreamEntityDeclaration> >(eng);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, NoError);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, CustomError);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, NotWellFormedError);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, PrematureEndOfDocumentError);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, UnexpectedElementError);

    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, NoToken);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, Invalid);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, StartDocument);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, EndDocument);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, StartElement);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, EndElement);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, Characters);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, Comment);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, DTD);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, EntityReference);
    ADD_ENUM_VALUE(ctorFun, QXmlStreamReader, ProcessingInstruction);

    return ctorFun;
}

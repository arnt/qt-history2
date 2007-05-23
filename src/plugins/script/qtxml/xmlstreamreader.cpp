#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamReader>
#include <QtCore/QVector>
#include "../global.h"

DECLARE_POINTER_METATYPE(QXmlStreamReader)
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
    return QScript::wrapPointer(eng, reader);
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
    DECLARE_SELF(QXmlStreamReader, addData);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *ba = qscriptvalue_cast<QByteArray*>(arg))
        self->addData(*ba);
    else
        self->addData(arg.toString());
    return eng->undefinedValue();
}

static QScriptValue atEnd(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, atEnd);
    return QScriptValue(eng, self->atEnd());
}

static QScriptValue attributes(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, attributes);
    return eng->toScriptValue(static_cast<QVector<QXmlStreamAttribute> >(self->attributes()));
}

static QScriptValue characterOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, characterOffset);
    return QScriptValue(eng, qsreal(self->characterOffset()));
}

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, clear);
    self->clear();
    return eng->undefinedValue();
}

static QScriptValue columnNumber(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, columnNumber);
    return QScriptValue(eng, qsreal(self->columnNumber()));
}

static QScriptValue device(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, device);
    return eng->toScriptValue(self->device());
}

static QScriptValue entityDeclarations(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, entityDeclarations);
    return eng->toScriptValue(self->entityDeclarations());
}

static QScriptValue error(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, error);
    return QScriptValue(eng, self->error());
}

static QScriptValue errorString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, errorString);
    return QScriptValue(eng, self->errorString());
}

static QScriptValue hasError(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, hasError);
    return QScriptValue(eng, self->hasError());
}

static QScriptValue isCDATA(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isCDATA);
    return QScriptValue(eng, self->isCDATA());
}

static QScriptValue isCharacters(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isCharacters);
    return QScriptValue(eng, self->isCharacters());
}

static QScriptValue isComment(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isComment);
    return QScriptValue(eng, self->isComment());
}

static QScriptValue isDTD(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isDTD);
    return QScriptValue(eng, self->isDTD());
}

static QScriptValue isEndDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isEndDocument);
    return QScriptValue(eng, self->isEndDocument());
}

static QScriptValue isEndElement(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isEndElement);
    return QScriptValue(eng, self->isEndElement());
}

static QScriptValue isEntityReference(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isEntityReference);
    return QScriptValue(eng, self->isEntityReference());
}

static QScriptValue isProcessingInstruction(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isProcessingInstruction);
    return QScriptValue(eng, self->isProcessingInstruction());
}

static QScriptValue isStandaloneDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isStandaloneDocument);
    return QScriptValue(eng, self->isStandaloneDocument());
}

static QScriptValue isStartDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isStartDocument);
    return QScriptValue(eng, self->isStartDocument());
}

static QScriptValue isStartElement(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isStartElement);
    return QScriptValue(eng, self->isStartElement());
}

static QScriptValue isWhitespace(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, isWhitespace);
    return QScriptValue(eng, self->isWhitespace());
}

static QScriptValue lineNumber(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, lineNumber);
    return QScriptValue(eng, qsreal(self->lineNumber()));
}

static QScriptValue name(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, name);
    return eng->toScriptValue(self->name());
}

static QScriptValue namespaceDeclarations(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, namespaceDeclarations);
    return eng->toScriptValue(self->namespaceDeclarations());
}

static QScriptValue namespaceProcessing(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, namespaceProcessing);
    return eng->toScriptValue(self->namespaceProcessing());
}

static QScriptValue namespaceUri(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, namespaceUri);
    return eng->toScriptValue(self->namespaceUri());
}

static QScriptValue notationDeclarations(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, notationDeclarations);
    return eng->toScriptValue(self->notationDeclarations());
}

static QScriptValue processingInstructionData(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, processingInstructionData);
    return eng->toScriptValue(self->processingInstructionData());
}

static QScriptValue processingInstructionTarget(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, processingInstructionTarget);
    return eng->toScriptValue(self->processingInstructionTarget());
}

static QScriptValue qualifiedName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, qualifiedName);
    return eng->toScriptValue(self->qualifiedName());
}

static QScriptValue raiseError(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, raiseError);
    self->raiseError(ctx->argument(0).toString());
    return eng->undefinedValue();
}

static QScriptValue readElementText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, readElementText);
    return QScriptValue(eng, self->readElementText());
}

static QScriptValue readNext(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, readNext);
    return QScriptValue(eng, self->readNext());
}

static QScriptValue setDevice(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, setDevice);
    self->setDevice(qscriptvalue_cast<QIODevice*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setNamespaceProcessing(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, setNamespaceProcessing);
    self->setNamespaceProcessing(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, text);
    return eng->toScriptValue(self->text());
}

static QScriptValue tokenString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, tokenString);
    return QScriptValue(eng, self->tokenString());
}

static QScriptValue tokenType(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, tokenType);
    return QScriptValue(eng, self->tokenType());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamReader, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamReader"));
}

QScriptValue constructXmlStreamReaderClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamReader(eng, new QXmlStreamReader());
    ADD_METHOD(proto, addData);
    ADD_METHOD(proto, atEnd);
    ADD_METHOD(proto, attributes);
    ADD_METHOD(proto, characterOffset);
    ADD_METHOD(proto, clear);
    ADD_METHOD(proto, columnNumber);
    ADD_METHOD(proto, device);
    ADD_METHOD(proto, entityDeclarations);
    ADD_METHOD(proto, error);
    ADD_METHOD(proto, errorString);
    ADD_METHOD(proto, hasError);
    ADD_METHOD(proto, isCDATA);
    ADD_METHOD(proto, isCharacters);
    ADD_METHOD(proto, isComment);
    ADD_METHOD(proto, isDTD);
    ADD_METHOD(proto, isEndDocument);
    ADD_METHOD(proto, isEndElement);
    ADD_METHOD(proto, isEntityReference);
    ADD_METHOD(proto, isProcessingInstruction);
    ADD_METHOD(proto, isStandaloneDocument);
    ADD_METHOD(proto, isStartElement);
    ADD_METHOD(proto, isWhitespace);
    ADD_METHOD(proto, lineNumber);
    ADD_METHOD(proto, name);
    ADD_METHOD(proto, namespaceDeclarations);
    ADD_METHOD(proto, namespaceProcessing);
    ADD_METHOD(proto, namespaceUri);
    ADD_METHOD(proto, notationDeclarations);
    ADD_METHOD(proto, processingInstructionData);
    ADD_METHOD(proto, processingInstructionTarget);
    ADD_METHOD(proto, qualifiedName);
    ADD_METHOD(proto, raiseError);
    ADD_METHOD(proto, readElementText);
    ADD_METHOD(proto, readNext);
    ADD_METHOD(proto, setDevice);
    ADD_METHOD(proto, setNamespaceProcessing);
    ADD_METHOD(proto, text);
    ADD_METHOD(proto, tokenString);
    ADD_METHOD(proto, tokenType);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QXmlStreamReader>(eng, proto);
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

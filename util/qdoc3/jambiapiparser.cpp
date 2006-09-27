/*
    jambiapiparser.cpp

    TODO:
        * deal with enum_1
*/

#include <QtXml>

#include "cppcodeparser.h"
#include "jambiapiparser.h"
#include "node.h"
#include "tree.h"

JambiApiParser::JambiApiParser(Tree *cppTree)
    : cppTre(cppTree), javaTre(0), metJapiTag(false)
{
}

JambiApiParser::~JambiApiParser()
{
}

void JambiApiParser::initializeParser(const Config &config)
{
    CodeParser::initializeParser(config);
}

void JambiApiParser::terminateParser()
{
    CodeParser::terminateParser();
}

QString JambiApiParser::language()
{
    return "Java";
}

QString JambiApiParser::sourceFileNameFilter()
{
    return "*.japi";
}

void JambiApiParser::parseSourceFile(const Location &location, const QString &filePath, Tree *tree)
{
    javaTre = tree;
    metJapiTag = false;

    QXmlSimpleReader reader;
    reader.setContentHandler(this);
    reader.setErrorHandler(this);

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        location.warning(tr("Cannot open JAPI file '%1'").arg(filePath));
        return;
    }

    japiLocation = Location(filePath);
    QXmlInputSource xmlSource(&file);
    reader.parse(xmlSource);
}

void JambiApiParser::doneParsingSourceFiles(Tree * /* tree */)
{
    if (javaTre) {
        jambifyDocs(javaTre->root());
        javaTre = 0;
    }
}

bool JambiApiParser::startElement(const QString & /* namespaceURI */,
                                  const QString & /* localName */,
                                  const QString &qName,
                                  const QXmlAttributes &attributes)
{
    if (!metJapiTag && qName != "japi") {
        // ### The file is not a JAPI file.
        return true;
    }
    metJapiTag = true;

    EnumNode *javaEnum = 0;
    EnumNode *cppEnum = 0;
    InnerNode *javaParent = javaTre->root();
    InnerNode *cppParent = cppTre->root();

    for (int i = 0; i < classAndEnumStack.count(); ++i) {
        const ClassOrEnumInfo &info = classAndEnumStack.at(i);
        if (info.cppNode) {
            if (info.cppNode->type() == Node::Enum) {
                Q_ASSERT(info.javaNode->type() == Node::Enum);
                javaEnum = static_cast<EnumNode *>(info.javaNode);
                cppEnum = static_cast<EnumNode *>(info.cppNode);
            } else {
                Q_ASSERT(info.javaNode->type() == Node::Class
                         || info.javaNode->type() == Node::Namespace);
                javaParent = static_cast<InnerNode *>(info.javaNode);
                cppParent = static_cast<InnerNode *>(info.cppNode);
            }
        }
    }

    if (qName == "class" || qName == "enum") {
        Node::Type type = (qName == "class") ? Node::Class : Node::Enum;

        ClassOrEnumInfo info;
        info.tag = qName;
        info.javaName = attributes.value("java");
        info.javaImplements = attributes.value("javaimplements");
        info.cppName = attributes.value("cpp");
        info.cppNode = cppTre->findNode(info.cppName.split("::"), type, cppParent);
        if (!info.cppNode && type == Node::Class) {
            type = Node::Namespace;
            info.cppNode = cppTre->findNode(info.cppName.split("::"), type, cppParent);
        }

        if (!info.cppNode) {
            japiLocation.warning(tr("Cannot find C++ class or enum '%1'").arg(info.cppName));
        } else {
            if (qName == "class") {
                info.javaNode = new ClassNode(javaParent, info.javaName); // ###
            } else {
                info.javaNode = new EnumNode(javaParent, info.javaName);
            }
            info.javaNode->setLocation(japiLocation);
            info.javaNode->setDoc(info.cppNode->doc());
        }
        classAndEnumStack.push(info);
    } else if (qName == "method") {
        QString javaSignature = attributes.value("java");
        QString cppSignature = attributes.value("cpp");

        CppCodeParser cppParser;
        const FunctionNode *cppNode = cppParser.findFunctionNode(cppSignature, cppTre,
                                                                 cppParent,
                                                                 true /* fuzzy */);
        if (!cppNode) {
            bool quiet = false;

#if 0   // ### get rid of this
            /*
                Functions reimplemented from the second base class
                sometimes aren't implemented in C++ (e.g.,
                QWidget::heightMM(), inherited from QPaintDevice).
            */
            QString javaImplements = classAndEnumStack.top().javaImplements;
            if (!javaImplements.isEmpty()) {
                if (javaImplements.endsWith("Interface"))   // evil
                    javaImplements.chop(9);
                Node *otherCppClass = cppTre->findNode(QStringList(javaImplements), Node::Class);
                if (otherCppClass
                        && cppParser.findFunctionNode(cppSignature, cppTre, otherCppClass,
                                                      true /* fuzzy */))
                    quiet = true;
            }
#endif

            /*
                Default constructors sometimes don't exist in C++.
            */
            if (!quiet && javaSignature == "public " + javaParent->name() + "()")
                quiet = true;

            if (!quiet)
                japiLocation.warning(tr("Cannot find C++ function '%1' ('%2')")
                                     .arg(cppSignature).arg(cppParent->name()));
        }

        FunctionNode *javaNode = new FunctionNode(javaParent, javaSignature /* wrong! */);
        javaNode->setLocation(japiLocation);
        if (cppNode)
            javaNode->setDoc(cppNode->doc());
    } else if (qName == "variablesetter" || qName == "variablegetter") {
        QString javaSignature = attributes.value("java");
        QString cppVariable = attributes.value("cpp");

        VariableNode *cppNode = static_cast<VariableNode *>(cppParent->findNode(cppVariable,
                                                                                Node::Variable));
        FunctionNode *javaNode = new FunctionNode(javaParent, javaSignature /* wrong! */);
        javaNode->setLocation(japiLocation);

        if (!cppNode) {
#if 0
            japiLocation.warning(tr("Cannot find C++ variable '%1' ('%2')")
                                 .arg(cppVariable).arg(cppParent->name()));
#endif
            javaNode->setDoc(Doc(japiLocation,
                                 "This method is used internally internal by Qt "
                                 "Jambi. Do not use it in your applications.",
                                 QSet<QString>()));
        } else {
            javaNode->setDoc(cppNode->doc());
        }
    } else if (qName == "enum-value") {
        QString javaName = attributes.value("java");
        QString cppName = attributes.value("cpp");
        QString value = attributes.value("value");

        EnumItem item(javaName, value);
        if (javaEnum)
            javaEnum->addItem(item);
    }

    return true;
}

bool JambiApiParser::endElement(const QString & /* namespaceURI */,
                                const QString & /* localName */,
                                const QString &qName)
{
    if (qName == "class" || qName == "enum")
        classAndEnumStack.pop();
    return true;
}

bool JambiApiParser::fatalError(const QXmlParseException &exception)
{
    japiLocation.setLineNo(exception.lineNumber());
    japiLocation.setColumnNo(exception.columnNumber());
    japiLocation.warning(tr("Syntax error in JAPI file (%1)").arg(exception.message()));
    return true;
}

void JambiApiParser::jambifyDocs(Node *node)
{
    const Doc &doc = node->doc();
    if (!doc.isEmpty()) {
        Doc newDoc;
//        newDoc.setLocation(doc.location());

    }

    if (node->isInnerNode()) {
        InnerNode *innerNode = static_cast<InnerNode *>(node);
        foreach (Node *child, innerNode->childNodes())
            jambifyDocs(child);
    }
}

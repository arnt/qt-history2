#include <QFile>

#include "apigenerator.h"
#include "codemarker.h"
#include "tree.h"

static QString indentStr(int indent)
{
    QString str;
    str.fill(QChar(' '), indent * 4);
    return str;
}

static bool lessThanName(Node *node1, Node *node2)
{
    return node1->name() < node2->name();
}

QString ApiGenerator::format()
{
    return "API";
}

void ApiGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    QFile outFile("api");
    outFile.open(QIODevice::WriteOnly);

    out.setDevice(&outFile);
    generateNode(tree->root(), marker);
    out.flush();
}

void ApiGenerator::generateNode(const Node *node, CodeMarker *marker, int indent)
{
    if (node->access() == Node::Private)
        return;

    switch (node->type()) {
    case Node::Namespace:
        if (!node->name().isEmpty()) {
            out << indentStr(indent) << "Namespace: " << node->name() << "\n";
            ++indent;
        }
        break;
    case Node::Class:
        {
            const ClassNode *classe = static_cast<const ClassNode *>(node);
            out << indentStr(indent) << "Class: " << node->name();
            foreach (RelatedClass baseClass, classe->baseClasses()) {
                if (baseClass.access == Node::Public)
                    out << " inherits " << baseClass.dataTypeWithTemplateArgs;
            }
            out << "\n";
            ++indent;
        }
        break;
    case Node::Enum:
        {
            const EnumNode *enume = static_cast<const EnumNode *>(node);
            out << indentStr(indent) << "Enum: " << node->name() << "\n";
            ++indent;

            QStringList enumNames;
            foreach (EnumItem item, enume->items())
                enumNames << item.name();
            qHeapSort(enumNames);

            foreach (QString name, enumNames)
                out << indentStr(indent) << "Enum value: " << name << "\n";
        }
        break;
    case Node::Typedef:
        out << indentStr(indent) << "Typedef: " << node->name() << "\n";
        ++indent;
        break;
    case Node::Function:
        {
            out << indentStr(indent) << "Function: "
                << plainCode(marker->markedUpSynopsis(node, 0, CodeMarker::Detailed)) << "\n";
            ++indent;
        }
        break;
    case Node::Property:
        {
            const PropertyNode *property = static_cast<const PropertyNode *>(node);
            out << indentStr(indent) << "Property: " << property->name()
                << " type " << property->dataType() << "\n";
            ++indent;
        }
        break;
    default:
        ;
    }

    if (node->isInnerNode()) {
        const InnerNode *inner = static_cast<const InnerNode *>(node);
        NodeList nodes = inner->childNodes();
        qHeapSort(nodes.begin(), nodes.end(), lessThanName);
        foreach (const Node *child, nodes)
            generateNode(child, marker, indent);
    }
}

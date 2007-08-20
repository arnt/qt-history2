/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  tree.h
*/

#ifndef TREE_H
#define TREE_H

#include "node.h"
#include <QDomElement>
#include <QXmlStreamWriter>

class QStringList;
class TreePrivate;

class Tree
{
public:
    enum FindFlag { SearchBaseClasses = 0x1, SearchEnumValues = 0x2, NonFunction = 0x4 };

    Tree();
    ~Tree();

    Node *findNode(const QStringList &path, Node *relative = 0, int findFlags = 0);
    Node *findNode(const QStringList &path, Node::Type type, Node *relative = 0,
                   int findFlags = 0);
    FunctionNode *findFunctionNode(const QStringList &path, Node *relative = 0,
                                   int findFlags = 0);
    FunctionNode *findFunctionNode(const QStringList &parentPath, const FunctionNode *clone,
                                   Node *relative = 0, int findFlags = 0);
    void addBaseClass(ClassNode *subclass, Node::Access access, const QStringList &basePath,
		      const QString &dataTypeWithTemplateArgs);
    void addPropertyFunction(PropertyNode *property, const QString &funcName,
			     PropertyNode::FunctionRole funcRole);
    void addToGroup(Node *node, const QString &group);
    void resolveInheritance(NamespaceNode *rootNode = 0);
    void resolveProperties();
    void resolveGroups();
    void resolveTargets();
    void fixInheritance(NamespaceNode *rootNode = 0);
    void setVersion(const QString &version) { vers = version; }
    NamespaceNode *root() { return &roo; }

    QString version() const { return vers; }
    const Node *findNode(const QStringList &path, const Node *relative = 0, int findFlags = 0) const;
    const Node *findNode(const QStringList &path, Node::Type type, const Node *relative = 0,
                         int findFlags = 0) const;
    const FunctionNode *findFunctionNode(const QStringList &path, const Node *relative = 0,
                                         int findFlags = 0) const;
    const FunctionNode *findFunctionNode(const QStringList &parentPath, const FunctionNode *clone,
                                         const Node *relative = 0, int findFlags = 0) const;
    const FakeNode *findFakeNodeByTitle(const QString &title) const;
    const Node *findUnambiguousTarget(const QString &target, Atom *&atom) const;
    Atom *findTarget(const QString &target, const Node *node) const;
    const NamespaceNode *root() const { return &roo; }
    void readIndexes(const QStringList &indexFiles);
    bool generateIndexSection(QXmlStreamWriter &writer, const Node *node) const;
    void generateIndexSections(QXmlStreamWriter &writer, const Node *node) const;
    void generateIndex(const QString &fileName, const QString &url,
                       const QString &title) const;
    void addExternalLink(const QString &url, const Node *relative);
    QString fullDocumentName(const Node *node) const;

private:
    void resolveInheritance(int pass, ClassNode *classe);
    FunctionNode *findVirtualFunctionInBaseClasses(ClassNode *classe, FunctionNode *clone);
    void fixPropertyUsingBaseClasses(ClassNode *classe, PropertyNode *property);
    NodeList allBaseClasses(const ClassNode *classe) const;
    void readIndexFile(const QString &path);
    void readIndexSection(const QDomElement &element, InnerNode *parent,
                          const QString &indexUrl);
    QString readIndexText(const QDomElement &element);
    void resolveIndex();

    NamespaceNode roo;
    QString vers;
    TreePrivate *priv;
};

#endif

/*
  tree.h
*/

#ifndef TREE_H
#define TREE_H

#include "node.h"

class QStringList;
class TreePrivate;

class Tree
{
public:
    Tree();
    ~Tree();

    Node *findNode(const QStringList &path, Node *relative = 0);
    Node *findNode(const QStringList &path, Node::Type type, Node *relative = 0);
    FunctionNode *findFunctionNode(const QStringList &path, Node *relative = 0);
    FunctionNode *findFunctionNode(const QStringList &parentPath, const FunctionNode *clone,
                                   Node *relative = 0);
    void addBaseClass(ClassNode *subclass, Node::Access access, const QStringList &basePath,
		      const QString &dataTypeWithTemplateArgs);
    void addPropertyFunction(PropertyNode *property, const QString &funcName,
			     PropertyNode::FunctionRole funcRole);
    void addToGroup(Node *node, const QString &group);
    void resolveInheritance();
    void resolveProperties();
    void resolveGroups();
    void fixInheritance();
    void setVersion(const QString &version) { vers = version; }
    NamespaceNode *root() { return &roo; }

    QString version() const { return vers; }
    const Node *findNode(const QStringList &path, const Node *relative = 0) const;
    const Node *findNode(const QStringList &path, Node::Type type, const Node *relative = 0) const;
    const FunctionNode *findFunctionNode(const QStringList &path, const Node *relative = 0) const;
    const FunctionNode *findFunctionNode(const QStringList &parentPath, const FunctionNode *clone,
                                         const Node *relative = 0) const;
    const NamespaceNode *root() const { return &roo; }

private:
    void resolveInheritance(int pass, ClassNode *classe);
    FunctionNode *findFunctionInBaseClasses(ClassNode *classe, FunctionNode *clone);

    NamespaceNode roo;
    QString vers;
    TreePrivate *priv;
};

#endif

/*
  tree.cpp
*/

#include <QtCore>

#include "node.h"
#include "text.h"
#include "tree.h"

struct InheritanceBound
{
    Node::Access access;
    QStringList basePath;
    QString dataTypeWithTemplateArgs;

    InheritanceBound()
	: access(Node::Public) { }
    InheritanceBound( Node::Access access0, const QStringList& basePath0,
		      const QString &dataTypeWithTemplateArgs0)
	: access(access0), basePath(basePath0),
	  dataTypeWithTemplateArgs(dataTypeWithTemplateArgs0) { }
};

struct Target
{
    Node *node;
    Atom *atom;
};

typedef QMap<PropertyNode::FunctionRole, QString> RoleMap;
typedef QMap<PropertyNode *, RoleMap> PropertyMap;
typedef QMultiMap<QString, Node *> GroupMap;
typedef QMultiHash<QString, FakeNode *> FakeNodeHash;
typedef QMultiHash<QString, Target> TargetHash;

class TreePrivate
{
public:
    QMap<ClassNode *, QList<InheritanceBound> > unresolvedInheritanceMap;
    PropertyMap unresolvedPropertyMap;
    GroupMap groupMap;
    FakeNodeHash fakeNodesByTitle;
    TargetHash targetHash;
};

Tree::Tree()
    : roo( 0, "" )
{
    priv = new TreePrivate;
}

Tree::~Tree()
{
    delete priv;
}

Node *Tree::findNode(const QStringList &path, Node *relative, int findFlags)
{
    return const_cast<Node *>(const_cast<const Tree *>(this)->findNode(path, relative, findFlags));
}

const Node *Tree::findNode(const QStringList &path, const Node *relative, int findFlags) const
{
    if (!relative)
        relative = root();

    do {
        const Node *node = relative;
        int i;

        for (i = 0; i < path.size(); ++i) {
	    if (node == 0 || !node->isInnerNode())
	        break;

            const Node *next = static_cast<const InnerNode *>(node)->findNode(path.at(i));
            if (!next && (findFlags & SearchEnumValues) && i == path.size() - 1)
                next = static_cast<const InnerNode *>(node)->findEnumNodeForValue(path.at(i));

            if (!next && node->type() == Node::Class && (findFlags & SearchBaseClasses)) {
                NodeList baseClasses = allBaseClasses(static_cast<const ClassNode *>(node));
                foreach (const Node *baseClass, baseClasses) {
                    next = static_cast<const InnerNode *>(baseClass)->findNode(path.at(i));
                    if (!next && (findFlags & SearchEnumValues) && i == path.size() - 1)
                        next = static_cast<const InnerNode *>(node)
                                        ->findEnumNodeForValue(path.at(i));
                    if (next)
                        break;
                }
            }
            node = next;
        }
        if (node && i == path.size() && (!(findFlags & NonFunction) || node->type() != Node::Function))
            return node;
        relative = relative->parent();
    } while (relative);

    return 0;
}

Node *Tree::findNode(const QStringList &path, Node::Type type, Node *relative, int findFlags)
{
    return const_cast<Node *>(const_cast<const Tree *>(this)->findNode(path, type, relative,
                                                                       findFlags));
}

const Node *Tree::findNode(const QStringList &path, Node::Type type, const Node *relative,
                           int findFlags) const
{
    const Node *node = findNode(path, relative, findFlags);
    if (node != 0 && node->type() == type)
	return node;
    return 0;
}

FunctionNode *Tree::findFunctionNode(const QStringList& path, Node *relative, int findFlags)
{
    return const_cast<FunctionNode *>(
                const_cast<const Tree *>(this)->findFunctionNode(path, relative, findFlags));
}

const FunctionNode *Tree::findFunctionNode(const QStringList &path, const Node *relative,
                                           int findFlags) const
{
    if (!relative)
        relative = root();
    do {
        const Node *node = relative;
        int i;

        for (i = 0; i < path.size(); ++i) {
	    if (node == 0 || !node->isInnerNode())
	        break;

            const Node *next;
            if (i == path.size() - 1)
                next = ((InnerNode *) node)->findFunctionNode(path.at(i));
            else
                next = ((InnerNode *) node)->findNode(path.at(i));

            if (!next && node->type() == Node::Class && (findFlags & SearchBaseClasses)) {
                NodeList baseClasses = allBaseClasses(static_cast<const ClassNode *>(node));
                foreach (const Node *baseClass, baseClasses) {
                    if (i == path.size() - 1)
                        next = static_cast<const InnerNode *>(baseClass)->
                                findFunctionNode(path.at(i));
                    else
                        next = static_cast<const InnerNode *>(baseClass)->findNode(path.at(i));

                    if (next)
                        break;
                }
            }

            node = next;
        }
        if (node && i == path.size())
            return static_cast<const FunctionNode *>(node);
        relative = relative->parent();
    } while (relative);

    return 0;
}

FunctionNode *Tree::findFunctionNode(const QStringList &parentPath, const FunctionNode *clone,
                                     Node *relative, int findFlags)
{
    return const_cast<FunctionNode *>(
		const_cast<const Tree *>(this)->findFunctionNode(parentPath, clone,
                                      				 relative, findFlags));
}

const FunctionNode *Tree::findFunctionNode(const QStringList &parentPath, const FunctionNode *clone,
                                           const Node *relative, int findFlags) const
{
    const Node *parent = findNode(parentPath, relative, findFlags);
    if ( parent == 0 || !parent->isInnerNode() ) {
	return 0;
    } else {
	return ((InnerNode *)parent)->findFunctionNode(clone);
    }
}

static const int NumSuffixes = 3;
static const char * const suffixes[NumSuffixes] = { "", "s", "es" };

const FakeNode *Tree::findFakeNodeByTitle(const QString &title) const
{
    for (int pass = 0; pass < NumSuffixes; ++pass) {
        FakeNodeHash::const_iterator i =
                priv->fakeNodesByTitle.find(Doc::canonicalTitle(title + suffixes[pass]));
        if (i != priv->fakeNodesByTitle.constEnd()) {
            FakeNodeHash::const_iterator j = i;
            ++j;
            if (j == priv->fakeNodesByTitle.constEnd() || j.key() != i.key())
                return i.value();
        }
    }
    return 0;
}

const Node *Tree::findUnambiguousTarget(const QString &target, Atom *&atom) const
{
    for (int pass = 0; pass < NumSuffixes; ++pass) {
        TargetHash::const_iterator i =
                priv->targetHash.find(Doc::canonicalTitle(target + suffixes[pass]));
        if (i != priv->targetHash.constEnd()) {
            TargetHash::const_iterator j = i;
            ++j;
            if (j == priv->targetHash.constEnd() || j.key() != i.key()) {
                atom = i.value().atom;
                return i.value().node;
            }
        }
    }
    return 0;
}

Atom *Tree::findTarget(const QString &target, const Node *node) const
{
    for (int pass = 0; pass < NumSuffixes; ++pass) {
        QString key = Doc::canonicalTitle(target + suffixes[pass]);
        TargetHash::const_iterator i = priv->targetHash.find(key);

        if (i != priv->targetHash.constEnd()) {
            do {
                if (i.value().node == node)
                    return i.value().atom;
                ++i;
            } while (i != priv->targetHash.constEnd() && i.key() == key);
        }
    }
    return 0;
}

void Tree::addBaseClass( ClassNode *subclass, Node::Access access,
			 const QStringList &basePath,
			 const QString &dataTypeWithTemplateArgs )
{
    priv->unresolvedInheritanceMap[subclass].append(
	    InheritanceBound(access, basePath, dataTypeWithTemplateArgs));
}


void Tree::addPropertyFunction(PropertyNode *property, const QString &funcName,
			       PropertyNode::FunctionRole funcRole)
{
    priv->unresolvedPropertyMap[property].insert(funcRole, funcName);
}

void Tree::addToGroup(Node *node, const QString &group)
{
    priv->groupMap.insert(group, node);
}

void Tree::resolveInheritance()
{
    for ( int pass = 0; pass < 2; pass++ ) {
	NodeList::ConstIterator c = root()->childNodes().begin();
	while ( c != root()->childNodes().end() ) {
	    if ( (*c)->type() == Node::Class )
		resolveInheritance( pass, (ClassNode *) *c );
	    ++c;
	}
	priv->unresolvedInheritanceMap.clear();
    }
}

void Tree::resolveProperties()
{
    PropertyMap::ConstIterator propEntry;
    
    propEntry = priv->unresolvedPropertyMap.begin();
    while (propEntry != priv->unresolvedPropertyMap.end()) {
	PropertyNode *property = propEntry.key();
        InnerNode *parent = property->parent();
	QString getterName = (*propEntry)[PropertyNode::Getter];
	QString setterName = (*propEntry)[PropertyNode::Setter];
	QString resetterName = (*propEntry)[PropertyNode::Resetter];

	NodeList::ConstIterator c = parent->childNodes().begin();
        while (c != parent->childNodes().end()) {
	    if ((*c)->type() == Node::Function) {
		FunctionNode *function = static_cast<FunctionNode *>(*c);
                if (function->status() == property->status()
                        && function->access() == property->access()) {
		    if (function->name() == getterName) {
	                property->addFunction(function, PropertyNode::Getter);
	            } else if (function->name() == setterName) {
	                property->addFunction(function, PropertyNode::Setter);
	            } else if (function->name() == resetterName) {
	                property->addFunction(function, PropertyNode::Resetter);
                    }
                }
	    }
	    ++c;
        }
	++propEntry;
    }

    propEntry = priv->unresolvedPropertyMap.begin();
    while (propEntry != priv->unresolvedPropertyMap.end()) {
	PropertyNode *property = propEntry.key();
        // redo it to set the property functions
        if (property->overriddenFrom())
            property->setOverriddenFrom(property->overriddenFrom());
	++propEntry;
    }

    priv->unresolvedPropertyMap.clear();
}

void Tree::resolveInheritance(int pass, ClassNode *classe)
{
    if ( pass == 0 ) {
	QList<InheritanceBound> bounds = priv->unresolvedInheritanceMap[classe];
	QList<InheritanceBound>::ConstIterator b = bounds.begin();
	while ( b != bounds.end() ) {
	    ClassNode *baseClass = (ClassNode *)findNode((*b).basePath, Node::Class);
	    if (baseClass)
		classe->addBaseClass((*b).access, baseClass, (*b).dataTypeWithTemplateArgs);
	    ++b;
	}
    } else {
	NodeList::ConstIterator c = classe->childNodes().begin();
	while ( c != classe->childNodes().end() ) {
	    if ( (*c)->type() == Node::Function ) {
		FunctionNode *func = (FunctionNode *) *c;
		FunctionNode *from = findVirtualFunctionInBaseClasses( classe, func );
		if ( from != 0 ) {
		    if ( func->virtualness() == FunctionNode::NonVirtual )
			func->setVirtualness( FunctionNode::ImpureVirtual );
		    func->setReimplementedFrom( from );
		}
	    } else if ((*c)->type() == Node::Property) {
                fixPropertyUsingBaseClasses(classe, static_cast<PropertyNode *>(*c));
            }
	    ++c;
	}
    }
}

void Tree::resolveGroups()
{
    GroupMap::const_iterator i;
    QString prevGroup;
    for (i = priv->groupMap.constBegin(); i != priv->groupMap.constEnd(); ++i) {
        if (i.value()->access() == Node::Private)
            continue;

        FakeNode *fake = static_cast<FakeNode *>(findNode(QStringList(i.key()), Node::Fake));
        if (fake && fake->subType() == FakeNode::Group) {
            fake->addGroupMember(i.value());
        } else {
            if (prevGroup != i.key())
                i.value()->doc().location().warning(tr("No such group '%1'").arg(i.key()));
        }

        prevGroup = i.key();
    }

    priv->groupMap.clear();
}

void Tree::resolveTargets()
{
    // need recursion

    foreach (Node *child, roo.childNodes()) {
        if (child->type() == Node::Fake) {
            FakeNode *node = static_cast<FakeNode *>(child);
            priv->fakeNodesByTitle.insert(Doc::canonicalTitle(node->title()), node);
        }

        if (child->doc().hasTableOfContents()) {
            const QList<Atom *> &toc = child->doc().tableOfContents();
            Target target;
            target.node = child;

            for (int i = 0; i < toc.size(); ++i) {
                target.atom = toc.at(i);
                QString title = Text::sectionHeading(target.atom).toString();
                if (!title.isEmpty())
                    priv->targetHash.insert(Doc::canonicalTitle(title), target);
            }
        }
        if (child->doc().hasKeywords()) {
            const QList<Atom *> &keywords = child->doc().keywords();
            Target target;
            target.node = child;

            for (int i = 0; i < keywords.size(); ++i) {
                target.atom = keywords.at(i);
                priv->targetHash.insert(Doc::canonicalTitle(target.atom->string()), target);
            }
        }
        if (child->doc().hasTargets()) {
            const QList<Atom *> &toc = child->doc().targets();
            Target target;
            target.node = child;

            for (int i = 0; i < toc.size(); ++i) {
                target.atom = toc.at(i);
                priv->targetHash.insert(Doc::canonicalTitle(target.atom->string()), target);
            }
        }
    }
}

void Tree::fixInheritance()
{
    NodeList::ConstIterator c = root()->childNodes().begin();
    while ( c != root()->childNodes().end() ) {
	if ( (*c)->type() == Node::Class )
	    static_cast<ClassNode *>(*c)->fixBaseClasses();
	++c;
    }
}

FunctionNode *Tree::findVirtualFunctionInBaseClasses(ClassNode *classe, FunctionNode *clone)
{
    QList<RelatedClass>::ConstIterator r = classe->baseClasses().begin();
    while ( r != classe->baseClasses().end() ) {
	FunctionNode *func;
	if ( ((func = findVirtualFunctionInBaseClasses((*r).node, clone)) != 0 ||
	      (func = (*r).node->findFunctionNode(clone)) != 0) &&
	     func->virtualness() != FunctionNode::NonVirtual )
	    return func;
 	++r;
    }
    return 0;
}

void Tree::fixPropertyUsingBaseClasses(ClassNode *classe, PropertyNode *property)
{
    QList<RelatedClass>::const_iterator r = classe->baseClasses().begin();
    while (r != classe->baseClasses().end()) {
	PropertyNode *baseProperty = static_cast<PropertyNode *>(r->node->findNode(property->name(), Node::Property));
        if (baseProperty) {
            fixPropertyUsingBaseClasses(r->node, baseProperty);
            property->setOverriddenFrom(baseProperty);
        } else {
            fixPropertyUsingBaseClasses(r->node, property);
        }
 	++r;
    }
}

NodeList Tree::allBaseClasses(const ClassNode *classe) const
{
    NodeList result;
    foreach (RelatedClass r, classe->baseClasses()) {
        result += r.node;
        result += allBaseClasses(r.node);
    }
    return result;
}

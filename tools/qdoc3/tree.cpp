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
  tree.cpp
*/

#include <QtCore>
#include <QDomDocument>

#include "atom.h"
#include "htmlgenerator.h"
#include "location.h"
#include "node.h"
#include "text.h"
#include "tree.h"

QT_BEGIN_NAMESPACE

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
    int priority;
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
    QList<QPair<ClassNode*,QString> > basesList;
    QList<QPair<FunctionNode*,QString> > relatedList;
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
                        next = static_cast<const InnerNode *>(baseClass)
                                        ->findEnumNodeForValue(path.at(i));
                    if (next)
                        break;
                }
            }
            node = next;
        }
        if (node && i == path.size()
                && (!(findFlags & NonFunction) || node->type() != Node::Function
                    || ((FunctionNode *)node)->metaness() == FunctionNode::MacroWithoutParams))
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
        if (node && i == path.size() && node->type() == Node::Function) {
            // CppCodeParser::processOtherMetaCommand ensures that reimplemented
            // functions are private.
            const FunctionNode *func = static_cast<const FunctionNode*>(node);

            while (func->access() == Node::Private) {
                const FunctionNode *from = func->reimplementedFrom();
                if (from != 0) {
                    if (from->access() != Node::Private)
                        return from;
                    else
                        func = from;
                } else
                    break;
            }
            return func;
        }
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
            if (j != priv->fakeNodesByTitle.constEnd() && j.key() == i.key()) {
                i.value()->doc().location().warning(
                    tr("Page '%1' defined in more than one location").arg(title));
                while (j != priv->fakeNodesByTitle.constEnd()) {
                    if (j.key() == i.key())
                        j.value()->doc().location().warning(tr("(defined here)"));
                    ++j;
                }
            }
            return i.value();
        }
    }
    return 0;
}

const Node *Tree::findUnambiguousTarget(const QString &target, Atom *&atom) const
{
    Target bestTarget = {0, 0, INT_MAX};
    int numBestTargets = 0;

    for (int pass = 0; pass < NumSuffixes; ++pass) {
        TargetHash::const_iterator i =
                priv->targetHash.find(Doc::canonicalTitle(target + suffixes[pass]));
        if (i != priv->targetHash.constEnd()) {
            TargetHash::const_iterator j = i;
            do {
                const Target &candidate = j.value();
                if (candidate.priority < bestTarget.priority) {
                    bestTarget = candidate;
                    numBestTargets = 1;
                } else if (candidate.priority == bestTarget.priority) {
                    ++numBestTargets;
                }
                ++j;
            } while (j != priv->targetHash.constEnd() && j.key() == i.key());

            if (numBestTargets == 1) {
                atom = bestTarget.atom;
                return bestTarget.node;
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

void Tree::resolveInheritance(NamespaceNode *rootNode)
{
    if (!rootNode)
        rootNode = root();

    for ( int pass = 0; pass < 2; pass++ ) {
	NodeList::ConstIterator c = rootNode->childNodes().begin();
	while ( c != rootNode->childNodes().end() ) {
	    if ( (*c)->type() == Node::Class )
		resolveInheritance( pass, (ClassNode *) *c );
            else if ( (*c)->type() == Node::Namespace ) {
                NamespaceNode *ns = static_cast<NamespaceNode*>(*c);
                resolveInheritance( ns );
            }
	    ++c;
	}
        if (rootNode == root())
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
            target.priority = 3;

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
            target.priority = 1;

            for (int i = 0; i < keywords.size(); ++i) {
                target.atom = keywords.at(i);
                priv->targetHash.insert(Doc::canonicalTitle(target.atom->string()), target);
            }
        }
        if (child->doc().hasTargets()) {
            const QList<Atom *> &toc = child->doc().targets();
            Target target;
            target.node = child;
            target.priority = 2;

            for (int i = 0; i < toc.size(); ++i) {
                target.atom = toc.at(i);
                priv->targetHash.insert(Doc::canonicalTitle(target.atom->string()), target);
            }
        }
    }
}

void Tree::fixInheritance(NamespaceNode *rootNode)
{
    if (!rootNode)
        rootNode = root();

    NodeList::ConstIterator c = rootNode->childNodes().begin();
    while ( c != rootNode->childNodes().end() ) {
	if ( (*c)->type() == Node::Class )
	    static_cast<ClassNode *>(*c)->fixBaseClasses();
        else if ( (*c)->type() == Node::Namespace ) {
            NamespaceNode *ns = static_cast<NamespaceNode*>(*c);
            fixInheritance( ns );
        }
	++c;
    }
}

FunctionNode *Tree::findVirtualFunctionInBaseClasses(ClassNode *classe, FunctionNode *clone)
{
    QList<RelatedClass>::ConstIterator r = classe->baseClasses().begin();
    while ( r != classe->baseClasses().end() ) {
	FunctionNode *func;
        if ( ((func = findVirtualFunctionInBaseClasses((*r).node, clone)) != 0 ||
	      (func = (*r).node->findFunctionNode(clone)) != 0) ) {
	    if (func->virtualness() != FunctionNode::NonVirtual)
	        return func;
        }
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

void Tree::readIndexes(const QStringList &indexFiles)
{
    foreach (QString indexFile, indexFiles)
        readIndexFile(indexFile);
}

void Tree::readIndexFile(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        QDomDocument document;
        document.setContent(&file);
        file.close();

        QDomElement indexElement = document.documentElement();
        QString indexUrl = indexElement.attribute("url", "");
        priv->basesList.clear();
        priv->relatedList.clear();

        // Scan all elements in the XML file, constructing a map that contains
        // base classes for each class found.

        QDomElement child = indexElement.firstChildElement();
        while (!child.isNull()) {
            readIndexSection(child, root(), indexUrl);
            child = child.nextSiblingElement();
        }

        // Now that all the base classes have been found for this index,
        // arrange them into an inheritance hierarchy.

        resolveIndex();
    }
}

void Tree::readIndexSection(const QDomElement &element,
    InnerNode *parent, const QString &indexUrl)
{
    QString name = element.attribute("name");

    Node *section;
    Location location;

    if (element.nodeName() == "namespace") {
        section = new NamespaceNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + name.toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(name.toLower() + ".html");

    } else if (element.nodeName() == "class") {
        section = new ClassNode(parent, name);
        priv->basesList.append(QPair<ClassNode*,QString>(
            static_cast<ClassNode*>(section), element.attribute("bases")));

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + name.toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(name.toLower() + ".html");

    } else if (element.nodeName() == "page") {
        FakeNode::SubType subtype;
        if (element.attribute("subtype") == "example")
            subtype = FakeNode::Example;
        else if (element.attribute("subtype") == "header")
            subtype = FakeNode::HeaderFile;
        else if (element.attribute("subtype") == "file")
            subtype = FakeNode::File;
        else if (element.attribute("subtype") == "group")
            subtype = FakeNode::Group;
        else if (element.attribute("subtype") == "module")
            subtype = FakeNode::Module;
        else if (element.attribute("subtype") == "page")
            subtype = FakeNode::Page;
        else if (element.attribute("subtype") == "externalpage")
            subtype = FakeNode::ExternalPage;
        else
            return;

        FakeNode *fakeNode = new FakeNode(parent, name, subtype);
        fakeNode->setTitle(element.attribute("title"));

        if (element.hasAttribute("location"))
            name = element.attribute("location", "");

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + name);
        else if (!indexUrl.isNull())
            location = Location(name);

        section = fakeNode;

    } else if (element.nodeName() == "enum") {
        EnumNode *enumNode = new EnumNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

        QDomElement child = element.firstChildElement("value");
        while (!child.isNull()) {
            EnumItem item(child.attribute("name"), child.attribute("value"));
            enumNode->addItem(item);
            child = child.nextSiblingElement("value");
        }

        section = enumNode;

    } else if (element.nodeName() == "typedef") {
        section = new TypedefNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

    } else if (element.nodeName() == "property") {
        section = new PropertyNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

    } else if (element.nodeName() == "function") {
        FunctionNode::Virtualness virt;
        if (element.attribute("virtual") == "non")
            virt = FunctionNode::NonVirtual;
        else if (element.attribute("virtual") == "impure")
            virt = FunctionNode::ImpureVirtual;
        else if (element.attribute("virtual") == "pure")
            virt = FunctionNode::PureVirtual;
        else
            return;

        FunctionNode::Metaness meta;
        if (element.attribute("meta") == "plain")
            meta = FunctionNode::Plain;
        else if (element.attribute("meta") == "signal")
            meta = FunctionNode::Signal;
        else if (element.attribute("meta") == "slot")
            meta = FunctionNode::Slot;
        else if (element.attribute("meta") == "constructor")
            meta = FunctionNode::Ctor;
        else if (element.attribute("meta") == "destructor")
            meta = FunctionNode::Dtor;
        else if (element.attribute("meta") == "macro")
            meta = FunctionNode::MacroWithParams;
        else if (element.attribute("meta") == "macrowithparams")
            meta = FunctionNode::MacroWithParams;
        else if (element.attribute("meta") == "macrowithoutparams")
            meta = FunctionNode::MacroWithoutParams;
        else
            return;

        FunctionNode *functionNode = new FunctionNode(parent, name);
        functionNode->setReturnType(element.attribute("return"));
        functionNode->setVirtualness(virt);
        functionNode->setMetaness(meta);
        functionNode->setConst(element.attribute("const") == "true");
        functionNode->setStatic(element.attribute("static") == "true");
        functionNode->setOverload(element.attribute("overload") == "true");

        if (element.hasAttribute("relates")
            && element.attribute("relates") != parent->name()) {
            priv->relatedList.append(
                QPair<FunctionNode*,QString>(functionNode,
                                             element.attribute("relates")));
        }

        QDomElement child = element.firstChildElement("parameter");
        while (!child.isNull()) {
            Parameter parameter(child.attribute("left"),
                                child.attribute("right"),
                                child.attribute("name"),
                                child.attribute("default"));
            functionNode->addParameter(parameter);
            child = child.nextSiblingElement("parameter");
        }

        section = functionNode;

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

    } else if (element.nodeName() == "variable") {
        section = new VariableNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + "/" + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

    } else if (element.nodeName() == "keyword") {
        Target target;
        target.node = parent;
        target.priority = 1;
        target.atom = new Atom(Atom::Target, name);
        priv->targetHash.insert(name, target);
        return;

    } else if (element.nodeName() == "target") {
        Target target;
        target.node = parent;
        target.priority = 2;
        target.atom = new Atom(Atom::Target, name);
        priv->targetHash.insert(name, target);
        return;

    } else if (element.nodeName() == "contents") {
        Target target;
        target.node = parent;
        target.priority = 3;
        target.atom = new Atom(Atom::Target, name);
        priv->targetHash.insert(name, target);
        return;

    } else
        return;

    QString access = element.attribute("access");
    if (access == "public")
        section->setAccess(Node::Public);
    else if (access == "protected")
        section->setAccess(Node::Protected);
    else if (access == "private")
        section->setAccess(Node::Private);
    else
        section->setAccess(Node::Public);

    if (element.nodeName() != "page") {
        QString threadSafety = element.attribute("threadsafety");
        if (threadSafety == "non-reentrant")
            section->setThreadSafeness(Node::NonReentrant);
        else if (threadSafety == "reentrant")
            section->setThreadSafeness(Node::Reentrant);
        else if (threadSafety == "thread safe")
            section->setThreadSafeness(Node::ThreadSafe);
        else
            section->setThreadSafeness(Node::UnspecifiedSafeness);
    } else
        section->setThreadSafeness(Node::UnspecifiedSafeness);

    section->setModuleName(element.attribute("module"));
    section->setUrl(indexUrl);

    // Create some content for the node.
    QSet<QString> emptySet;

    Doc doc(location, " ", emptySet); // placeholder
    section->setDoc(doc);

    InnerNode *inner = dynamic_cast<InnerNode*>(section);
    if (inner) {
        QDomElement child = element.firstChildElement();

        while (!child.isNull()) {
            if (element.nodeName() == "class")
                readIndexSection(child, inner, indexUrl);
            else if (element.nodeName() == "page")
                readIndexSection(child, inner, indexUrl);
            else if (element.nodeName() == "namespace" && !name.isEmpty())
                // The root node in the index is a namespace with an empty name.
                readIndexSection(child, inner, indexUrl);
            else
                readIndexSection(child, parent, indexUrl);

            child = child.nextSiblingElement();
        }
    }
}

QString Tree::readIndexText(const QDomElement &element)
{
    QString text;
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        if (child.isText())
            text += child.toText().nodeValue();
        child = child.nextSibling();
    }
    return text;
}

void Tree::resolveIndex()
{
    QPair<ClassNode*,QString> pair;

    foreach (pair, priv->basesList) {
        foreach (QString base, pair.second.split(",")) {
            Node *baseClass = root()->findNode(base, Node::Class);
            if (baseClass) {
                pair.first->addBaseClass(Node::Public,
                                         static_cast<ClassNode*>(baseClass));
            }
        }
    }

    QPair<FunctionNode*,QString> relatedPair;

    foreach (relatedPair, priv->relatedList) {
        Node *classNode = root()->findNode(relatedPair.second, Node::Class);
        if (classNode)
            relatedPair.first->setRelates(static_cast<ClassNode*>(classNode));
    }
}

/*!
    Generate the index section with the given \a writer for the \a node
    specified, returning true if an element was written; otherwise returns
    false.
*/

bool Tree::generateIndexSection(QXmlStreamWriter &writer, const Node *node) const
{
    if (!node->url().isEmpty())
        return false;

    QString nodeName;
    switch (node->type()) {
        case Node::Namespace:
            nodeName = "namespace";
            break;
        case Node::Class:
            nodeName = "class";
            break;
        case Node::Fake:
            nodeName = "page";
            break;
        case Node::Enum:
            nodeName = "enum";
            break;
        case Node::Typedef:
            nodeName = "typedef";
            break;
        case Node::Property:
            nodeName = "property";
            break;
        case Node::Function:
            nodeName = "function";
            break;
        case Node::Variable:
            nodeName = "variable";
            break;
        case Node::Target:
            nodeName = "target";
            break;
        default:
            return false;
    }

    QString access;
    switch (node->access()) {
        case Node::Public:
            access = "public";
            break;
        case Node::Protected:
            access = "protected";
            break;
        case Node::Private:     // Do not include private nodes in the index.
        default:
            return false;
    }

    QString objName = node->name();

    // Special case: only the root node should have an empty name.
    if (objName.isEmpty() && node != root())
        return false;

    writer.writeStartElement(nodeName);

    QXmlStreamAttributes attributes;
    writer.writeAttribute("access", access);

    if (node->type() != Node::Fake) {
        QString threadSafety;
        switch (node->threadSafeness()) {
            case Node::NonReentrant:
                threadSafety = "non-reentrant";
                break;
            case Node::Reentrant:
                threadSafety = "reentrant";
                break;
            case Node::ThreadSafe:
                threadSafety = "thread safe";
                break;
            case Node::UnspecifiedSafeness:
            default:
                threadSafety = "unspecified";
                break;
        }
        writer.writeAttribute("threadsafety", threadSafety);
    }

    writer.writeAttribute("name", objName);
    writer.writeAttribute("href", fullDocumentName(node));
    writer.writeAttribute("location", node->location().fileName());

    switch (node->type()) {

    case Node::Class:
        {
            // Classes contain information about their base classes.

            const ClassNode *classNode = static_cast<const ClassNode*>(node);
            QList<RelatedClass> bases = classNode->baseClasses();
            QStringList baseStrings;
            foreach (RelatedClass related, bases) {
                ClassNode *baseClassNode = related.node;
                baseStrings.append(baseClassNode->name());
            }
            writer.writeAttribute("bases", baseStrings.join(","));
            writer.writeAttribute("module", node->moduleName());
        }
        break;

    case Node::Namespace:
        writer.writeAttribute("module", node->moduleName());
        break;

    case Node::Fake:
        {
            // Fake nodes (such as manual pages) contain subtypes, titles and other
            // attributes.

            const FakeNode *fakeNode = static_cast<const FakeNode*>(node);
            switch (fakeNode->subType()) {
                case FakeNode::Example:
                    writer.writeAttribute("subtype", "example");
                    break;
                case FakeNode::HeaderFile:
                    writer.writeAttribute("subtype", "header");
                    break;
                case FakeNode::File:
                    writer.writeAttribute("subtype", "file");
                    break;
                case FakeNode::Group:
                    writer.writeAttribute("subtype", "group");
                    break;
                case FakeNode::Module:
                    writer.writeAttribute("subtype", "module");
                    break;
                case FakeNode::Page:
                    writer.writeAttribute("subtype", "page");
                    break;
                case FakeNode::ExternalPage:
                    writer.writeAttribute("subtype", "externalpage");
                    break;
                default:
                    break;
            }
            writer.writeAttribute("title", fakeNode->title());
            writer.writeAttribute("fulltitle", fakeNode->fullTitle());
            writer.writeAttribute("subtitle", fakeNode->subTitle());
            writer.writeAttribute("location", fakeNode->doc().location().fileName());
        }
        break;

    case Node::Function:
        {
            // Function nodes contain information about the type of function being
            // described.

            const FunctionNode *functionNode = static_cast<const FunctionNode*>(node);

            switch (functionNode->virtualness()) {
                case FunctionNode::NonVirtual:
                    writer.writeAttribute("virtual", "non");
                    break;
                case FunctionNode::ImpureVirtual:
                    writer.writeAttribute("virtual", "impure");
                    break;
                case FunctionNode::PureVirtual:
                    writer.writeAttribute("virtual", "pure");
                    break;
                default:
                    break;
            }
            switch (functionNode->metaness()) {
                case FunctionNode::Plain:
                    writer.writeAttribute("meta", "plain");
                    break;
                case FunctionNode::Signal:
                    writer.writeAttribute("meta", "signal");
                    break;
                case FunctionNode::Slot:
                    writer.writeAttribute("meta", "slot");
                    break;
                case FunctionNode::Ctor:
                    writer.writeAttribute("meta", "constructor");
                    break;
                case FunctionNode::Dtor:
                    writer.writeAttribute("meta", "destructor");
                    break;
                case FunctionNode::MacroWithParams:
                    writer.writeAttribute("meta", "macrowithparams");
                    break;
                case FunctionNode::MacroWithoutParams:
                    writer.writeAttribute("meta", "macrowithoutparams");
                    break;
                default:
                    break;
            }
            writer.writeAttribute("const", functionNode->isConst()?"true":"false");
            writer.writeAttribute("static", functionNode->isStatic()?"true":"false");
            writer.writeAttribute("overload", functionNode->isOverload()?"true":"false");
            if (functionNode->relates())
                writer.writeAttribute("relates", functionNode->relates()->name());
            const PropertyNode *propertyNode = functionNode->associatedProperty();
            if (propertyNode)
                writer.writeAttribute("associated-property", propertyNode->name());
        }
        break;

    case Node::Property:
        {
            const PropertyNode *propertyNode = static_cast<const PropertyNode*>(node);
            foreach (Node *fnNode, propertyNode->getters()) {
                const FunctionNode *functionNode = static_cast<const FunctionNode*>(fnNode);
                if (functionNode) {
                    writer.writeStartElement("getter");
                    writer.writeAttribute("name", functionNode->name());
                    writer.writeEndElement(); // getter
                }
            }
            foreach (Node *fnNode, propertyNode->setters()) {
                const FunctionNode *functionNode = static_cast<const FunctionNode*>(fnNode);
                if (functionNode) {
                    writer.writeStartElement("setter");
                    writer.writeAttribute("name", functionNode->name());
                    writer.writeEndElement(); // getter
                }
            }
            foreach (Node *fnNode, propertyNode->resetters()) {
                const FunctionNode *functionNode = static_cast<const FunctionNode*>(fnNode);
                if (functionNode) {
                    writer.writeStartElement("resetter");
                    writer.writeAttribute("name", functionNode->name());
                    writer.writeEndElement(); // getter
                }
            }
        }
        break;

    default:
        break;
    }

    // Inner nodes and function nodes contain child nodes of some sort, either
    // actual child nodes or function parameters. For these, we close the
    // opening tag, create child elements, then add a closing tag for the
    // element. Elements for all other nodes are closed in the opening tag.

    const InnerNode *inner = dynamic_cast<const InnerNode*>(node);
    if (inner) {

        // For internal pages, we canonicalize the target, keyword and content
        // item names so that they can be used by qdoc for other sets of
        // documentation.
        // The reason we do this here is that we don't want to ruin
        // externally composed indexes, containing non-qdoc-style target names
        // when reading in indexes.

        if (inner->doc().hasTargets()) {
            bool external = false;
            if (inner->type() == Node::Fake) {
                const FakeNode *fakeNode = static_cast<const FakeNode *>(inner);
                if (fakeNode->subType() == FakeNode::ExternalPage)
                    external = true;
            }

            foreach (Atom *target, inner->doc().targets()) {
                QString targetName = target->string();
                if (!external)
                    targetName = Doc::canonicalTitle(targetName);

                writer.writeStartElement("target");
                writer.writeAttribute("name", targetName);
                writer.writeEndElement(); // target
            }
        }
        if (inner->doc().hasKeywords()) {
            foreach (Atom *keyword, inner->doc().keywords()) {
                writer.writeStartElement("keyword");
                writer.writeAttribute("name", Doc::canonicalTitle(keyword->string()));
                writer.writeEndElement(); // keyword
            }
        }
        if (inner->doc().hasTableOfContents()) {
            foreach (Atom *item, inner->doc().tableOfContents()) {
                QString title = Text::sectionHeading(item).toString();
                writer.writeStartElement("contents");
                writer.writeAttribute("name", Doc::canonicalTitle(title));
                writer.writeAttribute("title", title);
                writer.writeEndElement(); // contents
            }
        }

    } else if (node->type() == Node::Function) {

        const FunctionNode *functionNode = static_cast<const FunctionNode*>(node);
        foreach (Parameter parameter, functionNode->parameters()) {
            // Do not supply a default value for the parameter; it will only
            // cause disagreement when it is read from an index file later on.
            writer.writeStartElement("parameter");
            writer.writeAttribute("left", parameter.leftType());
            writer.writeAttribute("right", parameter.rightType());
            writer.writeAttribute("name", parameter.name());
            //writer.writeAttribute("default", parameter.defaultValue());
            writer.writeEndElement(); // parameter
        }

    } else if (node->type() == Node::Enum) {

        const EnumNode *enumNode = static_cast<const EnumNode*>(node);
        foreach (EnumItem item, enumNode->items()) {
            writer.writeStartElement("value");
            writer.writeAttribute("name", item.name());
            writer.writeAttribute("value", item.value());
            writer.writeEndElement(); // value
        }
    }

    return true;
}

void Tree::generateIndexSections(QXmlStreamWriter &writer,
                                 const Node *node) const
{
    if (generateIndexSection(writer, node)) {

        if (node->isInnerNode()) {
            const InnerNode *inner = static_cast<const InnerNode *>(node);

            // Recurse to write an element for this child node and all its children.
            foreach (Node *child, inner->childNodes())
                generateIndexSections(writer, child);

/*
            foreach (Node *child, inner->relatedNodes()) {
                QDomElement childElement = generateIndexSections(document, child);
                element.appendChild(childElement);
            }
*/
        }
        writer.writeEndElement();
    }
}

void Tree::generateIndex(const QString &fileName, const QString &url,
                         const QString &title) const
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return ;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("QDOCINDEX");

    writer.writeStartElement("INDEX");
    writer.writeAttribute("url", url);
    writer.writeAttribute("title", title);
    writer.writeAttribute("version", version());

    generateIndexSections(writer, root());

    writer.writeEndElement(); // INDEX
    writer.writeEndElement(); // QDOCINDEX
    writer.writeEndDocument();
    file.close();
}

void Tree::addExternalLink(const QString &url, const Node *relative)
{
    FakeNode *fakeNode = new FakeNode(root(), url, FakeNode::ExternalPage);
    fakeNode->setAccess(Node::Public);

    // Create some content for the node.
    QSet<QString> emptySet;
    Location location(relative->doc().location());
    Doc doc(location, " ", emptySet); // placeholder
    fakeNode->setDoc(doc);
}

QString Tree::fullDocumentName(const Node *node) const
{
    if (node->type() == Node::Namespace) {

        // The root namespace has no name - check for this before creating
        // an attribute containing the location of any documentation.

        if (!node->fileBase().isEmpty())
            return node->fileBase() + ".html";
        else
            return "";
    } else if (node->type() == Node::Fake) {
        return node->fileBase() + ".html";
    }

    QString parentName;
    Node *parentNode = 0;

    if ((parentNode = node->relates()))
        parentName = fullDocumentName(node->relates());
    else if ((parentNode = node->parent()))
        parentName = fullDocumentName(node->parent());

    switch (node->type()) {
        case Node::Class:
            if (parentNode && parentNode->type() == Node::Class)
                return parentNode->fileBase().toLower()
                     + "-" + node->fileBase().toLower()
                     + ".html";
            else
                return node->fileBase() + ".html";
        case Node::Function:
            {
                // Functions can be overloaded.
                const FunctionNode *functionNode = static_cast<const FunctionNode *>(node);
                if (functionNode->overloadNumber() > 1)
                    return parentName + "#" + node->fileBase()
                           + "-" + QString::number(functionNode->overloadNumber());
                return parentName + "#" + node->fileBase();
            }

        case Node::Enum:
            return parentName + "#" + node->fileBase() + "-enum";
        case Node::Typedef:
            return parentName + "#" + node->fileBase() + "-typedef";
        case Node::Property:
            return parentName + "#" + node->fileBase() + "-prop";
        case Node::Variable:
            return parentName + "#" + node->fileBase() + "-var";
        case Node::Target:
            return parentName + "#" + node->fileBase();
        case Node::Namespace:
        case Node::Fake:
        default:
            break;
    }

    return "";
}

QT_END_NAMESPACE

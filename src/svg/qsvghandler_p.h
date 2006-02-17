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

#ifndef QSVGHANDLER_P_H
#define QSVGHANDLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtXml/qxml.h"
#include "QtCore/qhash.h"
#include "QtCore/qstack.h"
#include "qsvgstyle_p.h"

class QSvgNode;
class QSvgTinyDocument;
class QXmlAttributes;
class QSvgHandler;
class QColor;

typedef QSvgNode *(*FactoryMethod)(QSvgNode *,
                                   const QXmlAttributes &,
                                   QSvgHandler *);
typedef bool (*ParseMethod)(QSvgNode *,
                            const QXmlAttributes &,
                            QSvgHandler *);

typedef QSvgStyleProperty *(*StyleFactoryMethod)(QSvgNode *,
                                                 const QXmlAttributes &,
                                                 QSvgHandler *);
typedef bool (*StyleParseMethod)(QSvgStyleProperty *,
                                 const QXmlAttributes &,
                                 QSvgHandler *);

class QSvgHandler : public QXmlDefaultHandler
{
public:
    enum LengthType {
        PERCENT,
        PX,
        PC,
        PT,
        MM,
        CM,
        IN,
        OTHER
    };

public:
    QSvgHandler();

    QSvgTinyDocument *document() const;

    void setDefaultCoordinateSystem(LengthType type);
    LengthType defaultCoordinateSystem() const;

    void pushColor(const QColor &color);
    QColor currentColor() const;

public:
    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const;
private:
    void init();

    QSvgTinyDocument *m_doc;
    QStack<QSvgNode*> m_nodes;

    QList<QSvgNode*>  m_resolveNodes;

    enum CurrentNode
    {
        Unknown,
        Graphics,
        Style
    };
    QStack<CurrentNode> m_skipNodes;

    QSvgRefCounter<QSvgStyleProperty> m_style;

    LengthType m_defaultCoords;

    QStack<QColor> m_colorStack;
    QStack<int>    m_colorTagCount;
private:
    static QHash<QString, FactoryMethod> s_groupFactory;
    static QHash<QString, FactoryMethod> s_graphicsFactory;
    static QHash<QString, ParseMethod>   s_utilFactory;

    static QHash<QString, StyleFactoryMethod>   s_styleFactory;
    static QHash<QString, StyleParseMethod>     s_styleUtilFactory;
};

#endif // QSVGHANDLER_P_H

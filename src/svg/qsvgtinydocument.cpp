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

#include "qsvgtinydocument_p.h"
#include "qsvghandler_p.h"
#include "qsvgfont_p.h"

#include "qpainter.h"
#include "qxml.h"
#include "qfile.h"
#include "qbytearray.h"
#include "qqueue.h"
#include "qstack.h"
#include "qdebug.h"

QSvgTinyDocument::QSvgTinyDocument()
    : QSvgStructureNode(0),
      m_animated(false)
{
}

QSvgTinyDocument::~QSvgTinyDocument()
{
}

QSvgTinyDocument * QSvgTinyDocument::load(const QString &fileName)
{
    QSvgHandler handler;
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("Cannot open file '%s', because: %s",
                 qPrintable(fileName), qPrintable(file.errorString()));
        return 0;
    }

    QXmlInputSource xmlInputSource(&file);
    if (reader.parse(xmlInputSource)) {
        return handler.document();
    }
    return 0;
}

QSvgTinyDocument * QSvgTinyDocument::load(const QByteArray &contents)
{
    QSvgHandler handler;
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QXmlInputSource xmlInputSource;
    xmlInputSource.setData(contents);
    if (reader.parse(xmlInputSource)) {
        return handler.document();
    }
    return 0;
}

void QSvgTinyDocument::draw(QPainter *p)
{
    if (m_time.isNull()) {
        m_time.start();
    }

    //XXX set default style on the painter
    if (!m_viewBox.isEmpty())
        p->setWindow(m_viewBox);
    else if (!m_size.isEmpty())
        p->setWindow(0, 0, m_size.width(), m_size.height());
    else
        p->setWindow(0, 0, 100, 100);//XXX bogus!!
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::black);
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    QList<QSvgNode*>::iterator itr = m_renderers.begin();
    applyStyle(p);
    while (itr != m_renderers.end()) {
        QSvgNode *node = *itr;
        if (node->isVisible())
            node->draw(p);
        ++itr;
    }
    revertStyle(p);
}


void QSvgTinyDocument::draw(QPainter *p, const QString &id)
{
    QSvgNode *node = scopeNode(id);

    if (!node) {
        qDebug("Couldn't find node %s. Skipping rendering.", qPrintable(id));
        return;
    }

    QRectF bounds = node->bounds();
    QPointF topLeft;
    QRect vb = m_viewBox;
    if (!m_viewBox.isEmpty()) {
        topLeft = m_viewBox.topLeft();
        vb.setSize(bounds.size().toSize());
        p->setWindow(vb);
    } else {
        if (!vb.size().isEmpty())
            p->setWindow(0, 0, vb.width(), vb.height());
        else
            p->setWindow(0, 0, 100, 100);//XXX bogus!!
        topLeft = QPointF(0, 0);
    }
    
    //XXX set default style on the painter
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::black);
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    QStack<QSvgNode*> parentApplyStack;
    QQueue<QSvgNode*> parentRevertQueue;
    QSvgNode *parent = node->parent();
    while (parent) {
        parentApplyStack.push(parent);
        parentRevertQueue.enqueue(parent);
        parent = parent->parent();
    }

    foreach(QSvgNode *par, parentApplyStack) {
        par->applyStyle(p);
    }

    p->translate(-(bounds.topLeft()-topLeft));
    
    node->draw(p);

    p->translate(bounds.topLeft()-topLeft);
    
    foreach(QSvgNode *par, parentRevertQueue) {
        par->revertStyle(p);
    }
}


QSvgNode::Type QSvgTinyDocument::type() const
{
    return DOC;
}

void QSvgTinyDocument::setWidth(int len, bool percent)
{
    m_size.setWidth(len);
    m_widthPercent = percent;
}

void QSvgTinyDocument::setHeight(int len, bool percent)
{
    m_size.setHeight(len);
    m_heightPercent = percent;
}

void QSvgTinyDocument::setViewBox(const QRect &rect)
{
    m_viewBox = rect;
}

void QSvgTinyDocument::addSvgFont(QSvgFont *font)
{
    m_fonts.insert(font->familyName(), font);
}

QSvgFont * QSvgTinyDocument::svgFont(const QString &family) const
{
    return m_fonts[family];
}

void QSvgTinyDocument::restartAnimation()
{
    m_time.restart();
}

bool QSvgTinyDocument::animated() const
{
    return m_animated;
}

void QSvgTinyDocument::setAnimated(bool a)
{
    m_animated = a;
}

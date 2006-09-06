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
      m_animated(false),
      m_animationDuration(0),
      m_fps(30)
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

    QSvgTinyDocument *doc = 0;
    QXmlInputSource xmlInputSource(&file);
    if (reader.parse(xmlInputSource)) {
        doc = handler.document();
        doc->m_animationDuration = handler.animationDuration();
    }
    return doc;
}

QSvgTinyDocument * QSvgTinyDocument::load(const QByteArray &contents)
{
    QSvgHandler handler;
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QSvgTinyDocument *doc = 0;    
    QXmlInputSource xmlInputSource;
    xmlInputSource.setData(contents);
    if (reader.parse(xmlInputSource)) {
        doc = handler.document();
        doc->m_animationDuration = handler.animationDuration();
    }
    return doc;
}

void QSvgTinyDocument::draw(QPainter *p, const QRectF &bounds)
{
    if (m_time.isNull()) {
        m_time.start();
    }

    if (m_viewBox.isNull()) {
        QMatrix matx = QMatrix();
        m_viewBox = transformedBounds(matx);
    }

    p->save();

    //sets default style on the painter
    //### not the most optimal way
    adjustWindowBounds(p, bounds, m_viewBox);
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
    p->restore();
}


void QSvgTinyDocument::draw(QPainter *p, const QString &id,
                            const QRectF &boundingWindow)
{
    QSvgNode *node = scopeNode(id);

    if (!node) {
        qDebug("Couldn't find node %s. Skipping rendering.", qPrintable(id));
        return;
    }

    p->save();

    QMatrix matx = QMatrix();
    QRectF bounds = node->transformedBounds(matx);

    adjustWindowBounds(p, boundingWindow, bounds);
    matx = p->worldMatrix();
    
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
    //reset the world matrix so that our parents don't affect
    //the position
    QMatrix om = p->worldMatrix();
    p->setWorldMatrix(matx);
    
    node->draw(p);

    p->setWorldMatrix(om);
    
    foreach(QSvgNode *par, parentRevertQueue) {
        par->revertStyle(p);
    }

    //p->fillRect(bounds.adjusted(-5, -5, 5, 5), QColor(0, 0, 255, 100));

    p->restore();
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

void QSvgTinyDocument::setViewBox(const QRectF &rect)
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

void QSvgTinyDocument::draw(QPainter *p)
{
    draw(p, QRectF());
}

void QSvgTinyDocument::adjustWindowBounds(QPainter *p, 
                                          const QRectF &d,
                                          const QRectF &c)
{
    QPaintDevice *dev = p->device();
    QRectF current = c;

    if (current.isNull()) {
        if (!m_size.isEmpty()) {
            current = QRectF(0, 0, m_size.width(), m_size.height());
        } else {
            current = QRectF(0, 0, 100, 100);
        }
    }
    QRectF desired = d.isNull() ? QRectF(0, 0, dev->width(), dev->height()) : d;

    if (current != desired) {
        QMatrix mat;
        mat.scale(desired.width()/current.width(),
                  desired.height()/current.height());
        QRectF c2 = mat.mapRect(current);
        p->translate(desired.x()-c2.x(),
                     desired.y()-c2.y());
        p->scale(desired.width()/current.width(),
                 desired.height()/current.height());
       
        //qDebug()<<"two "<<mat<<", pt = "<<QPointF(desired.x()-c2.x(),
        //                                          desired.y()-c2.y());
        
    }
}

QRectF QSvgTinyDocument::boundsOnElement(const QString &id) const
{
    QRectF bounds;
    QMatrix matx;

    const QSvgNode *node = scopeNode(id);

    if (!node) {
        node = this;
    }
    
    bounds = node->transformedBounds(matx);
    return bounds;
}

bool QSvgTinyDocument::elementExists(const QString &id) const
{
    QSvgNode *node = scopeNode(id);

    return (node!=0);
}

QMatrix QSvgTinyDocument::matrixForElement(const QString &id) const
{
    QSvgNode *node = scopeNode(id);
    QMatrix mat;
    
    if (!node) {
        qDebug("Couldn't find node %s. Skipping rendering.", qPrintable(id));
        return mat;
    }
    QStack<QSvgNode*> parentApplyStack;
    QSvgNode *parent = node->parent();
    while (parent) {
        parentApplyStack.push(parent);
        parent = parent->parent();
    }

    QImage dummyImg(2, 2, QImage::Format_ARGB32_Premultiplied);
    QPainter dummy(&dummyImg);
    foreach(QSvgNode *par, parentApplyStack) {
        par->applyStyle(&dummy);
    }
    node->applyStyle(&dummy);
    mat = dummy.worldMatrix();
    
    return mat;
}

int QSvgTinyDocument::currentFrame() const
{
    double runningPercentage = qMin(m_time.elapsed()/double(m_animationDuration), 1.);

    int totalFrames = m_fps * m_animationDuration;
    
    return int(runningPercentage * totalFrames);
}

void QSvgTinyDocument::setCurrentFrame(int frame)
{
    int totalFrames = m_fps * m_animationDuration;
    double framePercentage = frame/double(totalFrames);
    double timeForFrame = m_animationDuration * framePercentage; //in S
    timeForFrame *= 1000; //in ms
    int timeToAdd = int(timeForFrame - m_time.elapsed());
    m_time = m_time.addMSecs(timeToAdd);
}

void QSvgTinyDocument::setFramesPerSecond(int num)
{
    m_fps = num;
}


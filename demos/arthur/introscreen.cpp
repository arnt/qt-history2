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

#include "introscreen.h"

#include <qdebug.h>
#include <qevent.h>
#include <qfile.h>
#include <qpainter.h>
#include <qtextdocument.h>
#include <qtextdocumentfragment.h>
#include <qabstracttextdocumentlayout.h>

IntroScreen::IntroScreen(QWidget *widget)
    : DemoWidget(widget), mouseDown(false), lastAnimationStep(0), animationStepDelta(0)
{
    QFile textFile(":/res/html/qt4-arthur.html");
    if (!textFile.open(QFile::ReadOnly))
        text = "Unable to load resource file: 'qt4-arthur.html'";
    else
        text = textFile.readAll();

    // Initialize text...
    textDocument = new QTextDocument(this);
    textDocument->setHtml(text);
    textDocument->setPageSize(QSize(400, 400));
}

void IntroScreen::paintEvent(QPaintEvent *)
{
    int w = width(), h = height();

    QPainter p(this);

    drawBackground(&p);

    QRect textRect(100, 0, w-200, h);

    p.setBrush(QColor(255, 255, 255, attributes->alpha ? 191 : 255));
    p.drawRect(textRect.x() - 20, textRect.y(), textRect.width() + 40, textRect.height());

    p.setPen(Qt::NoPen);
    int blockHeight = (int)textDocument->documentLayout()->documentSize().height();

    int ypos;
    if (mouseDown)
        ypos = lastAnimationStep;
    else
        ypos = (animationStep() / 10) - animationStepDelta;
    ypos = (-ypos % blockHeight);

    p.setClipRegion(textRect);

    p.translate(textRect.x(), ypos);

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = palette();

    for ( ; ypos < h; ypos += blockHeight) {
        ctx.clip = QRect(0, -ypos, w-200, h);
        textDocument->documentLayout()->draw(&p, ctx);
        p.translate(0, blockHeight);
    }

    QColor grey = palette().color(QPalette::Background);
}

void IntroScreen::mousePressEvent(QMouseEvent *e)
{
    oldMousePoint = e->pos();
    mouseDown = true;
    lastAnimationStep = (animationStep() / 10) - animationStepDelta;
}

void IntroScreen::mouseReleaseEvent(QMouseEvent *)
{
    mouseDown = false;
    animationStepDelta = (animationStep() / 10) - lastAnimationStep;
    lastAnimationStep = 0;
}

void IntroScreen::mouseMoveEvent(QMouseEvent *e)
{
    lastAnimationStep += oldMousePoint.y() - e->y();
    oldMousePoint = e->pos();
    repaint();
}

void IntroScreen::resizeEvent(QResizeEvent *e)
{
    DemoWidget::resizeEvent(e);
    textDocument->setPageSize(QSize(e->size().width() - 200, e->size().height()));
}


#include "introscreen.h"

#include <qdebug.h>
#include <qevent.h>
#include <qfile.h>
#include <qpainter.h>
#include <qtextdocument.h>
#include <qtextdocumentfragment.h>
#include <qabstracttextdocumentlayout.h>

IntroScreen::IntroScreen(QWidget *widget)
    : DemoWidget(widget), mouseDown(false)
{
    QFile textFile("..\\..\\doc\\html\\painting.html");
    if (!textFile.open(IO_ReadOnly))
        text = "Unable to load text file: 'text.html'";
    else
        text = textFile.readAll();

    // Create the gradients...
    QImage top(100, 100, 32);
    QImage bottom(100, 100, 32);
    for (int y=0; y<100; ++y) {
        int topPix = 0x00ffffff | (((100-y)*255/100)<<24);
        int botPix = 0x00ffffff | ((y*255/100)<<24);
        for (int x=0; x<100; ++x) {
            top.setPixel(x, y, topPix);
            bottom.setPixel(x, y, botPix);
        }
    }
    top.setAlphaBuffer(true);
    bottom.setAlphaBuffer(true);
    topGradient = top;
    bottomGradient = bottom;

    // Initialize text...
    textDocument = new QTextDocument(this);
    textDocument->setHtml(text);
}

void IntroScreen::paintEvent(QPaintEvent *)
{
    int w = width(), h = height();

    if (mouseDown)
        --animationStep;

    QPainter p(this);

    p.fillRect(0, 0, w, h, Qt::white);
    p.setPen(Qt::NoPen);

    QRect textRect(100, 0, w-200, h);

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.showCursor = false;
    ctx.palette = palette();

    QAbstractTextDocumentLayout *textLayout = textDocument->documentLayout();
    textLayout->setPageSize(textRect.size());

    int blockHeight = textLayout->sizeUsed().height();
    int ypos = (-animationStep % blockHeight);

    p.translate(textRect.x(), ypos);

    for ( ; ypos < h; ypos += blockHeight) {
        textLayout->draw(&p, ctx);
        p.translate(0, blockHeight);
    }

    QColor grey = palette().color(QPalette::Background);
    p.resetXForm();

    p.fillRect(0, 0, 100, h, QBrush(QPoint(0, 0), grey, QPoint(100, 0), Qt::white));
    p.fillRect(w-100, 0, 100, h, QBrush(QPoint(w, 0), grey, QPoint(w-100, 0), Qt::white));

    if (attributes->alpha) {
        p.drawTiledPixmap(QRect(textRect.x(), 0, textRect.width(), 100), topGradient);
        p.drawTiledPixmap(QRect(textRect.x(), h-100, textRect.width(), 100), bottomGradient);
    }
}

void IntroScreen::mousePressEvent(QMouseEvent *e)
{
    oldMousePoint = e->pos();
    mouseDown = true;
}

void IntroScreen::mouseReleaseEvent(QMouseEvent *)
{
    mouseDown = false;
}

void IntroScreen::mouseMoveEvent(QMouseEvent *e)
{
    animationStep += oldMousePoint.y() - e->y();
    oldMousePoint = e->pos();
    repaint();
}

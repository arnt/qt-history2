
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
    QFile textFile("text.html");
    if (!textFile.open(IO_ReadOnly))
        text = "Unable to load text file: 'text.html'";
    else
        text = textFile.readAll();

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

    fillBackground(&p);

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

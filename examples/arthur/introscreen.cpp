
#include "introscreen.h"

#include <qabstracttextdocumentlayout.h>
#include <qdebug.h>
#include <qevent.h>
#include <qfile.h>
#include <qpainter.h>
#include <qtextdocument.h>
#include <qtextdocumentfragment.h>
#include <private/qtextdocumentlayout_p.h>

IntroScreen::IntroScreen(QWidget *widget)
    : DemoWidget(widget)
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
//     QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    textDocument = new QTextDocument(this);
    textDocument->setHtml(text);
    textCursor = QTextCursor(textDocument);
//     textCursor.insertFragment(QTextDocumentFragment::fromHTML(text));
}

void IntroScreen::paintEvent(QPaintEvent *)
{
    int w = width(), h = height();

    QPainter p(this);

    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawRect(0, 0, w, h);

    QRect textRect(100, 0, w-200, h);

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.showCursor = false;
    ctx.cursor = textCursor;
    ctx.palette = palette();

    QAbstractTextDocumentLayout *textLayout = textDocument->documentLayout();
    textLayout->setPageSize(textRect.size());

    QTextDocumentLayout *layout = static_cast<QTextDocumentLayout*>(textLayout);

    int blockHeight = layout->rootFrameSize().height();
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

    p.drawTiledPixmap(QRect(textRect.x(), 0, textRect.width(), 100), topGradient);
    p.drawTiledPixmap(QRect(textRect.x(), h-100, textRect.width(), 100), bottomGradient);
}

void IntroScreen::mousePressEvent(QMouseEvent *e)
{
    oldMousePoint = e->pos();
    stopAnimation();
}

void IntroScreen::mouseReleaseEvent(QMouseEvent *)
{
    startAnimation();
}

void IntroScreen::mouseMoveEvent(QMouseEvent *e)
{
    animationStep += oldMousePoint.y() - e->y();
    oldMousePoint = e->pos();
    repaint();
}

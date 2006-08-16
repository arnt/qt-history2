/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "arthurwidgets.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QtEvents>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QFile>
#include <QTextBrowser>
#include <QBoxLayout>

extern QPixmap cached(const QString &img);

ArthurFrame::ArthurFrame(QWidget *parent)
    : QWidget(parent)
    , m_prefer_image(false)
{
#ifdef QT_OPENGL_SUPPORT
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    f.setStencil(true);
    f.setAlpha(true);
    f.setAlphaBufferSize(8);
    QGLFormat::setDefaultFormat(f);

    glw = new GLWidget(this);
    glw->setGeometry(0, 0, width(), height());
    glw->setAutoFillBackground(false);
    glw->disableAutoBufferSwap();

    enableOpenGL(true);
    enableOpenGL(false);
#endif
    m_document = 0;
    m_show_doc = false;

    m_tile = QPixmap(100, 100);
    m_tile.fill(Qt::white);
    QPainter pt(&m_tile);
    QColor color(240, 240, 240);
    pt.fillRect(0, 0, 50, 50, color);
    pt.fillRect(50, 50, 50, 50, color);
    pt.end();

//     QPalette pal = palette();
//     pal.setBrush(backgroundRole(), m_tile);
//     setPalette(pal);

#ifdef Q_WS_X11
    QPixmap xRenderPixmap(1, 1);
    m_prefer_image = !xRenderPixmap.x11PictureHandle();
#endif
}


void ArthurFrame::paintEvent(QPaintEvent *e)
{
    static QImage *static_image = 0;
    QPainter painter;
    if (preferImage()) {
        if (!static_image) {
            static_image = new QImage(size(), QImage::Format_RGB32);
        } else if (static_image->size() != size()) {
            delete static_image;
            static_image = new QImage(size(), QImage::Format_RGB32);
        }
        painter.begin(static_image);

        int o = 10;

        QBrush bg = palette().brush(QPalette::Background);
        painter.fillRect(0, 0, o, o, bg);
        painter.fillRect(width() - o, 0, o, o, bg);
        painter.fillRect(0, height() - o, o, o, bg);
        painter.fillRect(width() - o, height() - o, o, o, bg);
    } else {
#ifdef QT_OPENGL_SUPPORT
        if (m_use_opengl) {
            painter.begin(glw);
            painter.fillRect(QRectF(0, 0, glw->width(), glw->height()), palette().color(backgroundRole()));
        } else {
            painter.begin(this);
        }
#else
        painter.begin(this);
#endif
    }

    painter.setClipRect(e->rect());

    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath clipPath;

    QRect r = rect();
    double left = r.x() + 1;
    double top = r.y() + 1;
    double right = r.right();
    double bottom = r.bottom();
    double radius2 = 8 * 2;

    clipPath.moveTo(right - radius2, top);
    clipPath.arcTo(right - radius2, top, radius2, radius2, 90, -90);
    clipPath.arcTo(right - radius2, bottom - radius2, radius2, radius2, 0, -90);
    clipPath.arcTo(left, bottom - radius2, radius2, radius2, 270, -90);
    clipPath.arcTo(left, top, radius2, radius2, 180, -90);
    clipPath.closeSubpath();

    painter.save();
    painter.setClipPath(clipPath, Qt::IntersectClip);

    painter.drawTiledPixmap(rect(), m_tile);

    // client painting

    paint(&painter);

    painter.restore();

    painter.save();
    if (m_show_doc)
        paintDescription(&painter);
    painter.restore();

    int level = 180;
    painter.setPen(QPen(QColor(level, level, level), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(clipPath);

    if (preferImage()) {
        painter.end();
        painter.begin(this);
        painter.drawImage(e->rect(), *static_image, e->rect());
    }

#ifdef QT_OPENGL_SUPPORT
    if (m_use_opengl && (inherits("PathDeformRenderer") || inherits("PathStrokeRenderer") || inherits("CompositionRenderer") || m_show_doc))
        glw->swapBuffers();
#endif
}

void ArthurFrame::resizeEvent(QResizeEvent *e)
{
#ifdef QT_OPENGL_SUPPORT
    glw->setGeometry(0, 0, e->size().width()-1, e->size().height()-1);
#endif
    QWidget::resizeEvent(e);
}

void ArthurFrame::setDescriptionEnabled(bool enabled)
{
    if (m_show_doc != enabled) {
        m_show_doc = enabled;
        emit descriptionEnabledChanged(m_show_doc);
        update();
    }
}

void ArthurFrame::loadDescription(const QString &fileName)
{
    QFile textFile(fileName);
    QString text;
    if (!textFile.open(QFile::ReadOnly))
        text = QString("Unable to load resource file: '%1'").arg(fileName);
    else
        text = textFile.readAll();
    setDescription(text);
}


void ArthurFrame::setDescription(const QString &text)
{
    m_document = new QTextDocument(this);
    m_document->setHtml(text);
}

void ArthurFrame::paintDescription(QPainter *painter)
{
    if (!m_document)
        return;

    int pageWidth = qMax(width() - 100, 100);
    int pageHeight = qMax(height() - 100, 100);
    if (pageWidth != m_document->pageSize().width()) {
        m_document->setPageSize(QSize(pageWidth, pageHeight));
    }

    QRect textRect(width() / 2 - pageWidth / 2,
                   height() / 2 - pageHeight / 2,
                   pageWidth,
                   pageHeight);
    int pad = 10;
    QRect clearRect = textRect.adjusted(-pad, -pad, pad, pad);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 63));
    int shade = 10;
    painter->drawRect(clearRect.x() + clearRect.width() + 1,
                      clearRect.y() + shade,
                      shade,
                      clearRect.height() + 1);
    painter->drawRect(clearRect.x() + shade,
                      clearRect.y() + clearRect.height() + 1,
                      clearRect.width() - shade + 1,
                      shade);

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setBrush(QColor(255, 255, 255, 220));
    painter->setPen(Qt::black);
    painter->drawRect(clearRect);

    painter->setClipRegion(textRect, Qt::IntersectClip);
    painter->translate(textRect.topLeft());

    QAbstractTextDocumentLayout::PaintContext ctx;

    QLinearGradient g(0, 0, 0, textRect.height());
    g.setColorAt(0, Qt::black);
    g.setColorAt(0.9, Qt::black);
    g.setColorAt(1, Qt::transparent);

    QPalette pal = palette();
    pal.setBrush(QPalette::Text, g);

    ctx.palette = pal;
    ctx.clip = QRect(0, 0, textRect.width(), textRect.height());
    m_document->documentLayout()->draw(painter, ctx);
}

void ArthurFrame::loadSourceFile(const QString &sourceFile)
{
    m_sourceFileName = sourceFile;
}

void ArthurFrame::showSource()
{
    // Check for existing source
    if (qFindChild<QTextBrowser *>(this))
        return;

    QString contents;
    if (m_sourceFileName.isEmpty()) {
        contents = QString("No source for widget: '%1'").arg(objectName());
    } else {
        QFile f(m_sourceFileName);
        if (!f.open(QFile::ReadOnly))
            contents = QString("Could not open file: '%1'").arg(m_sourceFileName);
        else
            contents = f.readAll();
    }

    contents.replace('&', "&amp;");
    contents.replace('<', "&lt;");
    contents.replace('>', "&gt;");

    QStringList keywords;
    keywords << "for " << "if " << "switch " << " int " << "#include " << "const"
             << "void " << "uint " << "case " << "double " << "#define " << "static"
             << "new" << "this";

    foreach (QString keyword, keywords)
        contents.replace(keyword, QLatin1String("<font color=olive>") + keyword + QLatin1String("</font>"));
    contents.replace("(int ", "(<font color=olive><b>int </b></font>");

    QStringList ppKeywords;
    ppKeywords << "#ifdef" << "#ifndef" << "#if" << "#endif" << "#else";

    foreach (QString keyword, ppKeywords)
        contents.replace(keyword, QLatin1String("<font color=navy>") + keyword + QLatin1String("</font>"));

    contents.replace(QRegExp("(\\d\\d?)"), QLatin1String("<font color=navy>\\1</font>"));

    QRegExp commentRe("(//.+)\\n");
    commentRe.setMinimal(true);
    contents.replace(commentRe, QLatin1String("<font color=red>\\1</font>\n"));

    QRegExp stringLiteralRe("(\".+\")");
    stringLiteralRe.setMinimal(true);
    contents.replace(stringLiteralRe, QLatin1String("<font color=green>\\1</font>"));

    QString html = contents;
    html.prepend("<html><pre>");
    html.append("</pre></html>");

    QTextBrowser *sourceViewer = new QTextBrowser(0);
    sourceViewer->setWindowTitle("Source: " + m_sourceFileName.mid(5));
    sourceViewer->setParent(this, Qt::Dialog);
    sourceViewer->setAttribute(Qt::WA_DeleteOnClose);
    sourceViewer->setHtml(html);
    sourceViewer->resize(600, 600);
    sourceViewer->show();
}

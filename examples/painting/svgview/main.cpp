#include "qsvgview.h"

#include <QScrollArea>
#include <QApplication>
#include <QKeyEvent>
#include <QtDebug>
#include <QScrollBar>

#ifndef QT_NO_OPENGL
#include <QtOpenGL>
#endif

class SvgWindow : public QScrollArea
{
public:
    SvgWindow(const QString &file, const QString &type)
    {
        QWidget *view = 0;

        if (type == QLatin1String("-gl")) {
#ifndef QT_NO_OPENGL
            view = new QSvgGLView(file, this);
#else
            qWarning()<<"OpenGL is not supported on this system.";
            exit(1);
#endif
        } else if (type == QLatin1String("-native")) {
            view = new QSvgNativeView(file, this);
        } else
            view = new QSvgRasterView(file, this);
        setWidget(view);
    }

    void keyPressEvent(QKeyEvent *e)
    {
        if (e->key() == Qt::Key_Escape) {
            showNormal();
        } else if (e->key() == Qt::Key_F) {
            showFullScreen();
        }
    }

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);

private:
    QPoint mousePressPos;
    QPoint scrollBarValuesOnMousePress;
};

void SvgWindow::mousePressEvent(QMouseEvent *e)
{
    mousePressPos = e->pos();
    scrollBarValuesOnMousePress.rx() = horizontalScrollBar()->value();
    scrollBarValuesOnMousePress.ry() = verticalScrollBar()->value();
}

void SvgWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (mousePressPos.isNull()) {
        e->ignore();
        return;
    }
    e->accept();
    horizontalScrollBar()->setValue(scrollBarValuesOnMousePress.x() - e->pos().x() + mousePressPos.x());
    verticalScrollBar()->setValue(scrollBarValuesOnMousePress.y() - e->pos().y() + mousePressPos.y());
    horizontalScrollBar()->update();
    verticalScrollBar()->update();
}

void SvgWindow::mouseReleaseEvent(QMouseEvent *)
{
    mousePressPos = QPoint();
}

static void usage(const char *prog)
{
    qWarning()<<"Usage:"<<prog<<" <renderer> file";
    qWarning()<<"Please specify a SVG file to load.";
    qWarning()<<"Available rendereres: '-gl', '-native' and '-image'";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

#ifndef QT_NO_OPENGL
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(f);
#endif
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    QString file, type;
    if (argc == 3) {
        QString dummy = argv[1];
        if (dummy.startsWith('-')) {
            type = argv[1];
            file = argv[2];
        } else {
            file = argv[1];
            type = argv[2];
        }
    } else {
        file = argv[1];
        if (file.startsWith('-')) {
            usage(argv[0]);
            return 1;
        }
    }

    SvgWindow *area = new SvgWindow(file, type);
    area->show();
    return app.exec();
}

#include <QtGui>

class Widget : public QWidget
{
public:
    Widget(){ setAttribute(Qt::WA_InputMethodEnabled); }
    QSize sizeHint() const { return QSize(20, 20); }
    bool event(QEvent *e) {
        if (e->type() == QEvent::ContextMenu)
            return false;
        qDebug() << e;
        return QWidget::event(e);
    }
};


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Widget w;
    app.setMainWidget(&w);
    w.show();
    return app.exec();
}

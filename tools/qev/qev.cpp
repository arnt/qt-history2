/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_INTERNAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

QT_USE_NAMESPACE

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
    w.show();
    return app.exec();
}

#ifndef VIEW3D_H
#define VIEW3D_H

#include <QtGui/QWidget>

class QScrollBar;
class QGLWidget;
class FormWindow;

class View3DWidget;

class View3D : public QWidget
{
    Q_OBJECT

public:
    View3D(FormWindow *form_window, QWidget *parent);

public slots:
    void updateForm();

private:
    View3DWidget *m_3d_widget;
    FormWindow *m_form_window;

    void addWidget(int depth, QWidget *w);
    void addTexture(QWidget *w);
};

#endif // VIEW3D_H


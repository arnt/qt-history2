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

#ifndef VIEW3D_H
#define VIEW3D_H

#include <QtGui/QWidget>

class QScrollBar;
class QGLWidget;
class QDesignerFormWindowInterface;

class View3DWidget;

class View3D : public QWidget
{
    Q_OBJECT

public:
    View3D(QDesignerFormWindowInterface *form_window, QWidget *parent);

public slots:
    void updateForm();

private:
    View3DWidget *m_3d_widget;
    QDesignerFormWindowInterface *m_form_window;

    void addWidget(int depth, QWidget *w);
    void addTexture(QWidget *w);
};

#endif // VIEW3D_H


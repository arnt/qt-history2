#include <QScrollBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtOpenGL/QGLWidget>
#include <QPalette>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>

#include <qdebug.h>

#include "qextensionmanager.h"
#include "abstractmetadatabase.h"
#include "container.h"
#include "formwindow.h"
#include "view3d.h"

#define FORM_LIST_ID 1
#define SELECTION_BUFSIZE 512

/*******************************************************************************
** View3DWidget
*/

class View3DWidget : public QGLWidget
{
    Q_OBJECT
public:
    View3DWidget(QWidget *parent);
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    void clear();

    void addTexture(QWidget *w, const QPixmap &pm);

    void beginAddingWidgets(QWidget *form);
    void addWidget(int depth, QWidget *w);
    void endAddingWidgets();

    QWidget *widgetAt(const QPoint &pos);
    
signals:
    void updateForm();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void keyReleaseEvent(QKeyEvent *);

private:
    QWidget *m_form;
    QPoint oldPos;
    bool layerColoring;

    typedef QMap<QWidget*, GLuint> TextureMap;
    TextureMap m_texture_map;
    
    typedef QMap<GLuint, QWidget*> WidgetNameMap;
    GLuint m_next_widget_name;
    WidgetNameMap m_widget_name_map;
};

View3DWidget::View3DWidget(QWidget *parent)
    : QGLWidget(parent), layerColoring(true), m_next_widget_name(0)
{
    setFocusPolicy(Qt::StrongFocus);
}

static int nearestSize(int v)
{
    int n = 0, last = 0;
    for (int s = 0; s < 32; ++s) {
        if (((v>>s) & 1) == 1) {
            ++n;
            last = s;
        }
    }
    if (n > 1)
        return 1 << (last+1);
    return 1 << last;
}

void View3DWidget::addTexture(QWidget *w, const QPixmap &pm)
{
    int tx_w = nearestSize(pm.width());
    int tx_h = nearestSize(pm.height());

    QPixmap tmp(tx_w, tx_h);
    tmp.fill(QColor(0,0,0));
    QPainter p(&tmp);
    p.drawPixmap(0, tx_h - pm.height(), pm);
    p.end();
    QImage tex = tmp.toImage();
    tex = QGLWidget::convertToGLFormat(tex);

    GLuint tx_id;
    glGenTextures(1, &tx_id);
    glBindTexture(GL_TEXTURE_2D, tx_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width(), tex.height(), 0, GL_RGBA,
                GL_UNSIGNED_BYTE, tex.bits());
    m_texture_map[w] = tx_id;
}

void View3DWidget::addWidget(int depth, QWidget *widget)
{
    TextureMap::const_iterator it = m_texture_map.find(widget);
    Q_ASSERT(it != m_texture_map.end());
    glBindTexture(GL_TEXTURE_2D, *it);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR should be used

    QRect geometry = widget->geometry();
    int w = m_form->size().width();
    int h = m_form->size().height();
    int max = qMax(w, h);
    QRect r = widget->geometry();
    QPoint pos = widget->mapToGlobal(QPoint(0, 0));
    r.moveTopLeft(m_form->mapFromGlobal(pos));

    float s = r.width()/float(nearestSize(r.width()));
    float t = r.height()/float(nearestSize(r.height()));

    if (layerColoring)
        glColor4f(1.0 - depth/10.0, 1.0 - depth/10.0, 1.0, 1.0 - depth/20.0); //
    else
        glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);
    glLoadName(m_next_widget_name);
    glTexCoord2f(0.0, 0.0); glVertex3f(r.left() - w/2, r.bottom() - h/2, depth*max/8);
    glTexCoord2f(0.0, t);   glVertex3f(r.left() - w/2, r.top() - h/2, depth*max/8);
    glTexCoord2f(s, t);     glVertex3f(r.right() - w/2, r.top() - h/2, depth*max/8);
    glTexCoord2f(s, 0.0);   glVertex3f(r.right() - w/2, r.bottom()- h/2, depth*max/8);
    glEnd();

    m_widget_name_map[m_next_widget_name++] = widget;
}

void View3DWidget::clear()
{
    glDeleteLists(FORM_LIST_ID, 1);
    foreach (GLuint id, m_texture_map)
        glDeleteTextures(1, &id);
    m_texture_map.clear();
    m_widget_name_map.clear();
    m_next_widget_name = 0;
}

void View3DWidget::beginAddingWidgets(QWidget *form)
{
    m_form = form;
    glNewList(FORM_LIST_ID, GL_COMPILE);
    glInitNames();
    glPushName(-1);
    m_next_widget_name = 0;
}

void View3DWidget::endAddingWidgets()
{
    glEndList();
}

void View3DWidget::initializeGL()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    qglClearColor(palette().color(QPalette::Background).dark());
    glColor3f (1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glShadeModel(GL_FLAT);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void View3DWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-width()/2, width()/2, height()/2, -height()/2, -999999, 999999);
}

void View3DWidget::paintGL()
{
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCallList(FORM_LIST_ID);

    glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-width()/2, -height()/2, 0.0);
    glColor4f(1.0, 1.0, 1.0, 0.4);
    glRecti(0, height() - 35, width(), height());
    glColor4f(0.0, 0.0, 0.0, 1.0);
    renderText(10, height() - 35 + 15, "Press and hold left/right mouse button to tilt the view.");
    renderText(10, height() - 35 + 15*2, "Use the mouse wheel to zoom in/out. Press 't' to toggle layer coloring.");
    glPopMatrix();
    glPopAttrib();
}

QWidget *View3DWidget::widgetAt(const QPoint &pos)
{
    GLuint selectBuf[SELECTION_BUFSIZE];
    glSelectBuffer(SELECTION_BUFSIZE, selectBuf);
    glRenderMode (GL_SELECT);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glCallList(FORM_LIST_ID);

    
}

void View3DWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_T) {
        layerColoring = !layerColoring;
        emit updateForm();
    } else if (e->key() == Qt::Key_R) {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    updateGL();
}

void View3DWidget::mousePressEvent(QMouseEvent *e)
{
    oldPos = e->pos();
    
    qDebug() << "View3DWidget::mousePressEvent():" << widgetAt(e->pos());
}

void View3DWidget::mouseReleaseEvent(QMouseEvent *e)
{
    oldPos = e->pos();
}

void View3DWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & (Qt::LeftButton | Qt::RightButton)) {
        GLfloat rx = (GLfloat) (e->x() - oldPos.x()) / width();
        GLfloat ry = (GLfloat) (e->y() - oldPos.y()) / height();
    
        makeCurrent();
        glMatrixMode(GL_MODELVIEW);
        if (e->buttons() & Qt::LeftButton) {
            // Left button down - rotate around X and Y axes
            glRotatef(-180*ry, 1, 0, 0);
            glRotatef(180*rx, 0, 1, 0);
        } else if (e->buttons() & Qt::RightButton) {
            // Right button down - rotate around X and Z axes
            glRotatef(-180*ry, 1, 0, 0);
            glRotatef(-180*rx, 0, 0, 1);
        }
        updateGL();
        oldPos = e->pos();
    } else {
        
    }
}

void View3DWidget::wheelEvent(QWheelEvent *e)
{
    makeCurrent();
    glMatrixMode(GL_MODELVIEW);
    if (e->delta() < 0)
        glScalef(0.9, 0.9, 0.9);
    else
        glScalef(1.1, 1.1, 1.1);
    updateGL();
}

/*******************************************************************************
** Misc tools
*/

struct WalkWidgetTreeFunction
{
    virtual void operator () (int depth, QWidget *widget) const = 0;
};

static bool skipWidget(AbstractFormEditor *core, QWidget *widget)
{
    AbstractMetaDataBaseItem *item = core->metaDataBase()->item(widget);
    if (item == 0)
        return true;
    QString name = widget->metaObject()->className();
    if (name == "QLayoutWidget")
        return true;
    
    return false;
}

static void walkWidgetTree(AbstractFormEditor *core, int depth, QWidget *widget, const WalkWidgetTreeFunction &func)
{
    if (widget == 0)
        return;
    if (!widget->isVisible())
        return;
    
    if (!skipWidget(core, widget))
        func(depth++, widget);
        
    QObjectList child_obj_list = widget->children();
    foreach (QObject *child_obj, child_obj_list) {
        QWidget *child = qt_cast<QWidget*>(child_obj);
        if (child != 0)
            walkWidgetTree(core, depth, child, func);
    }
}

static void grabWidget_helper(QWidget *widget, QPixmap &res, QPixmap &buf,
                              const QRect &r, const QPoint &offset, AbstractFormEditor *core)
{
    buf.fill(widget, r.topLeft());
    QPainter::setRedirected(widget, &buf, r.topLeft());
    QPaintEvent e(r & widget->rect());
    QApplication::sendEvent(widget, &e);
    QPainter::restoreRedirected(widget);
    {
        QPainter pt(&res);
        pt.drawPixmap(offset.x(), offset.y(), buf, 0, 0, r.width(), r.height());
    }

    const QObjectList children = widget->children();
    for (int i = 0; i < children.size(); ++i) {
        QWidget *child = qt_cast<QWidget*>(children.at(i));
        if (child == 0 || child->isTopLevel())
            continue;
        if (child->isHidden() || !child->geometry().intersects(r))
            continue;
        if (core->metaDataBase()->item(child) != 0)
            continue;
        QRect cr = r & child->geometry();
        cr.translate(-child->pos());
        grabWidget_helper(child, res, buf, cr, offset + child->pos(), core);
    }
}

static QPixmap grabWidget(QWidget * widget, AbstractFormEditor *core)
{
    QPixmap res, buf;

    if (!widget)
        return res;

    QRect r = widget->rect();

    res.resize(r.size());
    buf.resize(r.size());
    if(!res || !buf)
        return res;

    grabWidget_helper(widget, res, buf, r, QPoint(), core);
    return res;
}

/*******************************************************************************
** View3D
*/

struct AddTexture : public WalkWidgetTreeFunction
{
    inline AddTexture(AbstractFormEditor *core, View3DWidget *w) 
        : m_core(core), m_3d_widget(w) {}
    inline virtual void operator ()(int, QWidget *w) const
        { m_3d_widget->addTexture(w, ::grabWidget(w, m_core).toImage()); }
    AbstractFormEditor *m_core;
    View3DWidget *m_3d_widget;
};

struct AddWidget : public WalkWidgetTreeFunction
{
    inline AddWidget(View3DWidget *w) : m_3d_widget(w) {}
    inline virtual void operator ()(int depth, QWidget *w) const
        { m_3d_widget->addWidget(depth, w); }
    View3DWidget *m_3d_widget;
};

View3D::View3D(FormWindow *form_window, QWidget *parent)
    : QWidget(parent)
{
    m_form_window = form_window;
    m_3d_widget = new View3DWidget(this);
    connect(m_3d_widget, SIGNAL(updateForm()), this, SLOT(updateForm()));

    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_3d_widget, 0, 0, 1, 1);

    updateForm();
}

void View3D::updateForm()
{
    QWidget *w = m_form_window->mainContainer();
    if (w == 0)
        return;

    m_3d_widget->clear();

    walkWidgetTree(m_form_window->core(), 0, w, AddTexture(m_form_window->core(), m_3d_widget));
    m_3d_widget->beginAddingWidgets(w);
    walkWidgetTree(m_form_window->core(), 0, w, AddWidget(m_3d_widget));
    m_3d_widget->endAddingWidgets();
}

#include "view3d.moc"

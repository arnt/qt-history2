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

#ifndef CONNECTIONEDIT_H
#define CONNECTIONEDIT_H

#include <QWidget>
#include <QMap>
#include <QMultiMap>
#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QPolygonF>

#include "shared_global.h"

class AbstractFormWindow;
class QtUndoStack;
class Connection;
class ConnectionEdit;

struct QT_SHARED_EXPORT CETypes
{
    typedef QList<Connection*> ConnectionList;
    typedef QMap<Connection*, Connection*> ConnectionSet;
    typedef QMap<QWidget*, QWidget*> WidgetSet;
    struct EndPoint {
        enum Type { Source, Target };
        EndPoint(Connection *_con = 0, Type _type = Source) : con(_con), type(_type) {}
        bool isNull() const { return con == 0; }
        bool operator == (const EndPoint &other) const { return con == other.con && type == other.type; }
        bool operator != (const EndPoint &other) const { return !operator == (other); }
        Connection *con;
        Type type;
    };
    enum LineDir { UpDir = 0, DownDir, RightDir, LeftDir };
};

class QT_SHARED_EXPORT Connection : public CETypes
{
public:
    Connection(ConnectionEdit *edit);
    Connection(ConnectionEdit *edit, QWidget *source, QWidget *target);
    virtual ~Connection() {}
    
    QWidget *widget(EndPoint::Type type) const 
        { return type == EndPoint::Source ? m_source : m_target; }
    QPoint endPointPos(EndPoint::Type type) const;
    QRect endPointRect(EndPoint::Type) const;
    void setEndPoint(EndPoint::Type type, QWidget *w, const QPoint &pos)
        { type == EndPoint::Source ? setSource(w, pos) : setTarget(w, pos); }
            
    virtual QRegion region() const;
    bool contains(const QPoint &pos) const;
    virtual void paint(QPainter *p) const;

    void update(bool update_widgets = true) const;
    void checkWidgets();

    QString label(EndPoint::Type type) const 
        { return type == EndPoint::Source ? m_source_label : m_target_label; }
    void setLabel(EndPoint::Type type, const QString &text);
    QRect labelRect(EndPoint::Type type) const;
    QPixmap labelPixmap(EndPoint::Type type) const
        { return type == EndPoint::Source ? m_source_label_pm : m_target_label_pm; }
        
    ConnectionEdit *edit() const { return m_edit; }
            
    virtual void inserted() {}
    virtual void removed() {}
    
private:
    QPoint m_source_pos, m_target_pos;
    QWidget *m_source, *m_target;
    QList<QPoint> m_knee_list;
    QPolygonF m_arrow_head;
    ConnectionEdit *m_edit;
    QString m_source_label, m_target_label;
    QPixmap m_source_label_pm, m_target_label_pm;
    QRect m_source_rect, m_target_rect;
    
    void setSource(QWidget *source, const QPoint &pos);
    void setTarget(QWidget *target, const QPoint &pos);
    void updateKneeList();
    void trimLine();
    void updatePixmap(EndPoint::Type type);
    LineDir labelDir(EndPoint::Type type) const;
};

class QT_SHARED_EXPORT ConnectionEdit : public QWidget, public CETypes
{
    Q_OBJECT
public:
    ConnectionEdit(QWidget *parent, AbstractFormWindow *form);
    
    inline QWidget *background() const { return m_bg_widget; }
    void setBackground(QWidget *background);
    
    void setSelected(Connection *con, bool sel);
    bool selected(const Connection *con) const;
    void selectNone();
    
    int connectionCount() const { return m_con_list.size(); }
    Connection *connection(int i) const { return m_con_list.at(i); }
    
    void clear();
    
public slots:    
    void updateBackground();
    void widgetRemoved(QWidget *w);
    void updateLines();
    
protected:    
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    virtual Connection *createConnection(QWidget *source, QWidget *target);
    virtual void modifyConnection(Connection *con);
    
    virtual QWidget *widgetAt(const QPoint &pos) const;
    QRect widgetRect(QWidget *w) const;
    void addConnection(Connection *con);

    enum State { Editing, Connecting, Dragging };
    State state() const;
        
private:    
    QWidget *m_bg_widget;
    QtUndoStack *m_undo_stack;
    QPixmap m_bg_pixmap;

    Connection *m_tmp_con; // the connection we are currently editing
    ConnectionList m_con_list;
    bool m_start_connection_on_drag;
    void startConnection(QWidget *source, const QPoint &pos);
    void continueConnection(QWidget *target, const QPoint &pos);
    void endConnection(QWidget *target, const QPoint &pos);

    void findObjectsUnderMouse(const QPoint &pos);
    EndPoint m_end_point_under_mouse;
    QPointer<QWidget> m_widget_under_mouse;
        
    EndPoint m_drag_end_point;
    QPoint m_old_source_pos, m_old_target_pos;
    void startDrag(const EndPoint &end_point, const QPoint &pos);
    void continueDrag(const QPoint &pos);
    void endDrag(const QPoint &pos);
    void adjustHotSopt(const EndPoint &end_point, const QPoint &pos);
    
    void deleteSelected();
    
    Connection *connectionAt(const QPoint &pos) const;
    EndPoint endPointAt(const QPoint &pos) const;
    ConnectionSet m_sel_con_set;

    void paintConnection(QPainter *p, Connection *con,
                            WidgetSet *heavy_highlight_set,
                            WidgetSet *light_highlight_set) const;
    void paintLabel(QPainter *p, EndPoint::Type type, Connection *con);
                            
    QColor m_inactive_color, m_active_color;
                        
    friend class Connection;
    friend class AddConnectionCommand;
    friend class DeleteConnectionsCommand;
};

#endif // CONNECTIONEDIT_H

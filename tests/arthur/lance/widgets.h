#ifndef WIDGETS_H
#define WIDGETS_H

#include "paintcommands.h"

#include <QWidget>
#include <QSettings>
#include <QFileInfo>
#include <QPainter>
#include <QPaintEvent>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>

#include <private/qmath_p.h>

const int CP_RADIUS = 10;

template <class T>
class OnScreenWidget : public T
{
public:
    OnScreenWidget(QWidget *parent = 0) : T(parent)
    {
        QSettings settings("Trolltech", "lance");
        for (int i=0; i<10; ++i) {
            QPointF suggestion(100 + i * 40, 100 + 100 * qSin(i * 3.1415 / 10.0));
            m_controlPoints << settings.value("cp" + QString::number(i), suggestion).toPointF();
        }

        m_currentPoint = -1;
        m_showControlPoints = false;
        m_deviceType = WidgetType;
        m_checkersBackground = true;
        m_verboseMode = false;
    }

    void setVerboseMode(bool v) { m_verboseMode = v; }
    void setCheckersBackground(bool b) { m_checkersBackground = b; }
    void setType(DeviceType t) { m_deviceType = t; }

    ~OnScreenWidget()
    {
        QSettings settings("Trolltech", "lance");
        for (int i=0; i<10; ++i) {
            settings.setValue("cp" + QString::number(i), m_controlPoints.at(i));
        }
        settings.sync();
    }

    void paintEvent(QPaintEvent *)
    {
        QPainter pt(this);
        PaintCommands paintCommands(m_commands, 800, 800);
        paintCommands.setVerboseMode(m_verboseMode);
        paintCommands.setCheckersBackground(m_checkersBackground);
        paintCommands.setType(m_deviceType);
        paintCommands.setPainter(&pt);
        paintCommands.setControlPoints(m_controlPoints);
        paintCommands.setFilePath(QFileInfo(m_filename).absolutePath());
#ifdef DO_QWS_DEBUGGING
        qt_show_painter_debug_output = true;
#endif
        pt.save();
        paintCommands.runCommands();
        pt.restore();
#ifdef DO_QWS_DEBUGGING
        qt_show_painter_debug_output = false;
#endif

        if (m_currentPoint >= 0 || m_showControlPoints) {
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setFont(this->font());
            pt.resetMatrix();
            pt.setPen(QColor(127, 127, 127, 191));
            pt.setBrush(QColor(191, 191, 255, 63));
            for (int i=0; i<m_controlPoints.size(); ++i) {
                if (m_showControlPoints || m_currentPoint == i) {
                    QPointF cp = m_controlPoints.at(i);
                    QRectF rect(cp.x() - CP_RADIUS, cp.y() - CP_RADIUS,
                                CP_RADIUS * 2, CP_RADIUS * 2);
                    pt.drawEllipse(rect);
                    pt.drawText(rect, Qt::AlignCenter, QString::number(i));
                }
            }
        }
    }


    void mouseMoveEvent(QMouseEvent *e)
    {
        if (m_currentPoint == -1) {
            return;
        }

        if (T::rect().contains(e->pos())) {
            m_controlPoints[m_currentPoint] = e->pos();
            T::update();
        }

    }

    void mousePressEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::RightButton) {
            m_showControlPoints = true;
        }

        if (e->button() == Qt::LeftButton) {
            for (int i=0; i<m_controlPoints.size(); ++i) {
                if (QLineF(m_controlPoints.at(i), e->pos()).length() < CP_RADIUS) {
                    m_currentPoint = i;
                    break;
                }
            }
        }
        T::update();
    }

    void mouseReleaseEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton) {
            m_currentPoint = -1;
        }

        if (e->button() == Qt::RightButton) {
            m_showControlPoints = false;
        }
        T::update();
    }

    QSize sizeHint() const { return QSize(800, 800); }

    QVector<QPointF> m_controlPoints;
    int m_currentPoint;
    bool m_showControlPoints;

    QStringList m_commands;
    QString m_filename;

    bool m_verboseMode;
    bool m_checkersBackground;
    DeviceType m_deviceType;
};

#endif

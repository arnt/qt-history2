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
            controlPoints << settings.value("cp" + QString::number(i), suggestion).toPointF();
        }

        currentPoint = -1;
        showControlPoints = false;
        type = WidgetType;
        checkersBackground = true;
        verboseMode = false;
    }
    void setVerboseMode(bool v)
    {
        verboseMode = v;
    }
    void setCheckersBackground(bool b)
    {
        checkersBackground = b;
    }
    void setType(DeviceType t)
    {
        type = t;
    }

    ~OnScreenWidget()
    {
        QSettings settings("Trolltech", "lance");
        for (int i=0; i<10; ++i) {
            settings.setValue("cp" + QString::number(i), controlPoints.at(i));
        }
        settings.sync();
    }

    void paintEvent(QPaintEvent *)
    {
        QPainter pt(this);
        PaintCommands cmd(cmds, 800, 800);
        cmd.setVerboseMode(verboseMode);
        cmd.setCheckersBackground(checkersBackground);
        cmd.setType(type);
        cmd.setPainter(&pt);
        cmd.setControlPoints(controlPoints);
        cmd.setFilePath(QFileInfo(filename).absolutePath());
#ifdef DO_QWS_DEBUGGING
        qt_show_painter_debug_output = true;
#endif
        pt.save();
        cmd.runCommands();
        pt.restore();
#ifdef DO_QWS_DEBUGGING
        qt_show_painter_debug_output = false;
#endif

        if (currentPoint >= 0 || showControlPoints) {
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setFont(this->font());
            pt.resetMatrix();
            pt.setPen(QColor(127, 127, 127, 191));
            pt.setBrush(QColor(191, 191, 255, 63));
            for (int i=0; i<controlPoints.size(); ++i) {
                if (showControlPoints || currentPoint == i) {
                    QPointF cp = controlPoints.at(i);
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
        if (currentPoint == -1) {
            return;
        }

        if (T::rect().contains(e->pos())) {
            controlPoints[currentPoint] = e->pos();
            T::update();
        }

    }

    void mousePressEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::RightButton) {
            showControlPoints = true;
        }

        if (e->button() == Qt::LeftButton) {
            for (int i=0; i<controlPoints.size(); ++i) {
                if (QLineF(controlPoints.at(i), e->pos()).length() < CP_RADIUS) {
                    currentPoint = i;
                    break;
                }
            }
        }
        T::update();
    }

    void mouseReleaseEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton) {
            currentPoint = -1;
        }

        if (e->button() == Qt::RightButton) {
            showControlPoints = false;
        }
        T::update();
    }

    QSize sizeHint() const { return QSize(800, 800); }

    QVector<QPointF> controlPoints;
    int currentPoint;
    bool showControlPoints;

    QStringList cmds;
    QString filename;

    bool verboseMode;
    bool checkersBackground;
    DeviceType type;
};

#endif

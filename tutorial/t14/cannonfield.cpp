/****************************************************************
**
** Implementation CannonField class, Qt tutorial 14
**
****************************************************************/

#include <QDateTime>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

#include <cmath>
#include <cstdlib>

#include "cannonfield.h"

using namespace std;

CannonField::CannonField(QWidget *parent)
    : QWidget(parent)
{
    ang = 45;
    f = 0;
    timerCount = 0;
    autoShootTimer = new QTimer(this);
    connect(autoShootTimer, SIGNAL(timeout()), this, SLOT(moveShot()));
    shoot_ang = 0;
    shoot_f = 0;
    target = QPoint(0, 0);
    gameEnded = false;
    barrelPressed = false;
    setPalette(QPalette(QColor(250, 250, 200)));
    newTarget();
}

void CannonField::setAngle(int angle)
{
    if (angle < 5)
	angle = 5;
    if (angle > 70)
	angle = 70;
    if (ang == angle)
	return;
    ang = angle;
    repaint(cannonRect());
    emit angleChanged(ang);
}

void CannonField::setForce(int force)
{
    if (force < 0)
	force = 0;
    if (f == force)
	return;
    f = force;
    emit forceChanged(f);
}

void CannonField::shoot()
{
    if (isShooting())
	return;
    timerCount = 0;
    shoot_ang = ang;
    shoot_f = f;
    autoShootTimer->start(50);
    emit canShoot(false);
}

void  CannonField::newTarget()
{
    static bool first_time = true;

    if (first_time) {
	first_time = false;
	QTime midnight(0, 0, 0);
	srand(midnight.secsTo(QTime::currentTime()));
    }
    QRegion r(targetRect());
    target = QPoint(200 + rand() % 190, 10 + rand() % 255);
    repaint(r.unite(targetRect()));
}

void CannonField::setGameOver()
{
    if (gameEnded)
	return;
    if (isShooting())
	autoShootTimer->stop();
    gameEnded = true;
    repaint();
}

void CannonField::restartGame()
{
    if (isShooting())
	autoShootTimer->stop();
    gameEnded = false;
    repaint();
    emit canShoot(true);
}

void CannonField::moveShot()
{
    QRegion r(shotRect());
    ++timerCount;

    QRect shotR = shotRect();

    if (shotR.intersects(targetRect())) {
	autoShootTimer->stop();
	emit hit();
	emit canShoot(true);
    } else if (shotR.x() > width() || shotR.y() > height()
               || shotR.intersects(barrierRect())) {
	autoShootTimer->stop();
	emit missed();
	emit canShoot(true);
    } else {
	r = r.unite(QRegion(shotR));
    }

    repaint(r);
}

void CannonField::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
	return;
    if (barrelHit(event->pos()))
	barrelPressed = true;
}

void CannonField::mouseMoveEvent(QMouseEvent *event)
{
    if (!barrelPressed)
	return;
    QPoint pnt = event->pos();
    if (pnt.x() <= 0)
	pnt.setX(1);
    if (pnt.y() >= height())
	pnt.setY(height() - 1);
    double rad = atan(((double)rect().bottom()-pnt.y())/pnt.x());
    setAngle(qRound (rad*180/3.14159265));
}

void CannonField::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
	barrelPressed = false;
}

void CannonField::paintEvent(QPaintEvent *event)
{
    QRect updateR = event->rect();
    QPainter painter(this);

    if (gameEnded) {
	painter.setPen(Qt::black);
	painter.setFont(QFont("Courier", 48, QFont::Bold));
	painter.drawText(rect(), Qt::AlignCenter, "Game Over");
    }
    if (updateR.intersects(cannonRect()))
	paintCannon(painter);
    if (updateR.intersects(barrierRect()))
	paintBarrier(painter);
    if (isShooting() && updateR.intersects(shotRect()))
	paintShot(painter);
    if (!gameEnded && updateR.intersects(targetRect()))
	paintTarget(painter);
}

void CannonField::paintShot(QPainter &painter)
{
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    painter.drawRect(shotRect());
}

void CannonField::paintTarget(QPainter &painter)
{
    painter.setBrush(Qt::red);
    painter.setPen(Qt::black);
    painter.drawRect(targetRect());
}

void CannonField::paintBarrier(QPainter &painter)
{
    painter.setBrush(Qt::yellow);
    painter.setPen(Qt::black);
    painter.drawRect(barrierRect());
}

const QRect barrelRect(33, -4, 15, 8);

void CannonField::paintCannon(QPainter &painter)
{
    QRect rect = cannonRect();
    QPixmap pixmap(rect.size());
    pixmap.fill(this, rect.topLeft());

    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setBrush(Qt::blue);
    pixmapPainter.setPen(Qt::NoPen);

    pixmapPainter.translate(0, pixmap.height() - 1);
    pixmapPainter.drawPie(QRect(-35, -35, 70, 70), 0, 90 * 16);
    pixmapPainter.rotate(-ang);
    pixmapPainter.drawRect(barrelRect);
    pixmapPainter.end();

    painter.drawPixmap(rect.topLeft(), pixmap);
}

QRect CannonField::cannonRect() const
{
    QRect result(0, 0, 50, 50);
    result.moveBottomLeft(rect().bottomLeft());
    return result;
}

QRect CannonField::shotRect() const
{
    const double gravity = 4;

    double time = timerCount / 4.0;
    double velocity = shoot_f;
    double radians = shoot_ang * 3.14159265 / 180;

    double velx = velocity * cos(radians);
    double vely = velocity * sin(radians);
    double x0 = (barrelRect.right() + 5) * cos(radians);
    double y0 = (barrelRect.right() + 5) * sin(radians);
    double x = x0 + velx * time;
    double y = y0 + vely * time - 0.5 * gravity * time * time;

    QRect r = QRect(0, 0, 6, 6);
    r.moveCenter(QPoint(qRound(x), height() - 1 - qRound(y)));
    return r;
}

QRect CannonField::targetRect() const
{
    QRect result(0, 0, 20, 10);
    result.moveCenter(QPoint(target.x(), height() - 1 - target.y()));
    return result;
}

QRect CannonField::barrierRect() const
{
    return QRect(145, height() - 100, 15, 100);
}

bool CannonField::barrelHit(const QPoint &pos) const
{
    QMatrix matrix;
    matrix.translate(0, height() - 1);
    matrix.rotate(-ang);
    matrix = matrix.invert();
    return barrelRect.contains(matrix.map(pos));
}

bool CannonField::isShooting() const
{
    return autoShootTimer->isActive();
}

QSize CannonField::sizeHint() const
{
    return QSize(400, 300);
}

/****************************************************************
**
** Implementation CannonField class, Qt tutorial 13
**
****************************************************************/

#include <QDateTime>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

#include <cmath>
#include <cstdlib>

#include "cannon.h"

using namespace std;

CannonField::CannonField(QWidget *parent)
    : QWidget(parent)
{
    ang = 45;
    f = 0;
    timerCount = 0;
    autoShootTimer = new QTimer(this);
    connect(autoShootTimer, SIGNAL(timeout()),
	    this, SLOT(moveShot()));
    shoot_ang = 0;
    shoot_f = 0;
    target = QPoint(0, 0);
    gameEnded = false;
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

void CannonField::newTarget()
{
    static bool firstTime = true;

    if (firstTime) {
	firstTime = false;
	QTime midnight(0, 0, 0);
	srand(midnight.secsTo(QTime::currentTime()));
    }
    QRegion region(targetRect());
    target = QPoint(200 + rand() % 190, 10 + rand() % 255);
    repaint(region.unite(targetRect()));
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
    QRegion region = shotRect();
    ++timerCount;

    QRect shotR = shotRect();

    if (shotR.intersects(targetRect())) {
	autoShootTimer->stop();
	emit hit();
	emit canShoot(true);
    } else if (shotR.x() > width() || shotR.y() > height()) {
	autoShootTimer->stop();
	emit missed();
	emit canShoot(true);
    } else {
	region = region.unite(shotR);
    }

    repaint(region);
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

const QRect barrelRect(33, -4, 15, 8);

void CannonField::paintCannon(QPainter &painter)
{
    QRect rect = cannonRect();
    QPixmap pixmap(rect.size());
    pixmap.fill(this, rect.topLeft());

    QPainter tmp(&pixmap);
    tmp.setBrush(Qt::blue);
    tmp.setPen(Qt::NoPen);

    tmp.translate(0, pixmap.height() - 1);
    tmp.drawPie(QRect(-35, -35, 70, 70), 0, 90 * 16);
    tmp.rotate(-ang);
    tmp.drawRect(barrelRect);
    tmp.end();

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

    QRect result(0, 0, 6, 6);
    result.moveCenter(QPoint(qRound(x), height() - 1 - qRound(y)));
    return result;
}

QRect CannonField::targetRect() const
{
    QRect result(0, 0, 20, 10);
    result.moveCenter(QPoint(target.x(), height() - 1 - target.y()));
    return result;
}

bool CannonField::isShooting() const
{
    return autoShootTimer->isActive();
}

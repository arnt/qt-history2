#include "qtgradientstopsmodel.h"
#include <QColor>

#include "qdebug.h"

class QtGradientStopPrivate
{
public:
    qreal m_position;
    QColor m_color;
    QtGradientStopsModel *m_model;
};

qreal QtGradientStop::position() const
{
    return d_ptr->m_position;
}

QColor QtGradientStop::color() const
{
    return d_ptr->m_color;
}

QtGradientStopsModel *QtGradientStop::gradientModel() const
{
    return d_ptr->m_model;
}

void QtGradientStop::setColor(const QColor &color)
{
    d_ptr->m_color = color;
}

void QtGradientStop::setPosition(qreal position)
{
    d_ptr->m_position = position;
}

QtGradientStop::QtGradientStop(QtGradientStopsModel *model)
{
    d_ptr = new QtGradientStopPrivate();
    d_ptr->m_position = 0;
    d_ptr->m_color = Qt::white;
    d_ptr->m_model = model;
}

QtGradientStop::~QtGradientStop()
{
    delete d_ptr;
}

class QtGradientStopsModelPrivate
{
    QtGradientStopsModel *q_ptr;
    Q_DECLARE_PUBLIC(QtGradientStopsModel)
public:
    QMap<qreal, QtGradientStop *> m_posToStop;
    QMap<QtGradientStop *, qreal> m_stopToPos;
    QMap<QtGradientStop *, bool> m_selection;
    QtGradientStop *m_current;
};





QtGradientStopsModel::QtGradientStopsModel(QObject *parent)
    : QObject(parent)
{
    d_ptr = new QtGradientStopsModelPrivate;
    d_ptr->q_ptr = this;
    d_ptr->m_current = 0;
}

QtGradientStopsModel::~QtGradientStopsModel()
{
    clear();
    delete d_ptr;
}

QMap<qreal, QtGradientStop *> QtGradientStopsModel::stops() const
{
    return d_ptr->m_posToStop;
}

QtGradientStop *QtGradientStopsModel::at(qreal pos) const
{
    if (d_ptr->m_posToStop.contains(pos))
        return d_ptr->m_posToStop[pos];
    return 0;
}

QColor QtGradientStopsModel::color(qreal pos) const
{
    QMap<double, QtGradientStop *> gradStops = stops();
    if (gradStops.isEmpty())
        return QColor::fromRgbF(pos, pos, pos, 1.0);
    if (gradStops.contains(pos))
        return gradStops[pos]->color();

    gradStops[pos] = 0;
    QMap<double, QtGradientStop *>::ConstIterator itStop = gradStops.find(pos);
    if (itStop == gradStops.constBegin()) {
        itStop++;
        return itStop.value()->color();
    }
    if (itStop == --gradStops.constEnd()) {
        itStop--;
        return itStop.value()->color();
    }
    QMap<double, QtGradientStop *>::ConstIterator itPrev = itStop;
    QMap<double, QtGradientStop *>::ConstIterator itNext = itStop;
    itPrev--;
    itNext++;

    double prevX = itPrev.key();
    double nextX = itNext.key();

    double coefX = (pos - prevX) / (nextX - prevX);
    QColor prevCol = itPrev.value()->color();
    QColor nextCol = itNext.value()->color();

    QColor newColor;
    newColor.setRgbF((nextCol.redF()   - prevCol.redF()  ) * coefX + prevCol.redF(),
                     (nextCol.greenF() - prevCol.greenF()) * coefX + prevCol.greenF(),
                     (nextCol.blueF()  - prevCol.blueF() ) * coefX + prevCol.blueF(),
                     (nextCol.alphaF() - prevCol.alphaF()) * coefX + prevCol.alphaF());
    return newColor;
}

QList<QtGradientStop *> QtGradientStopsModel::selectedStops() const
{
    return d_ptr->m_selection.keys();
}

QtGradientStop *QtGradientStopsModel::currentStop() const
{
    return d_ptr->m_current;
}

bool QtGradientStopsModel::isSelected(QtGradientStop *stop) const
{
    if (d_ptr->m_selection.contains(stop))
        return true;
    return false;
}

QtGradientStop *QtGradientStopsModel::addStop(qreal pos, const QColor &color)
{
    qreal newPos = pos;
    if (pos < 0.0)
        newPos = 0.0;
    if (pos > 1.0)
        newPos = 1.0;
    if (d_ptr->m_posToStop.contains(newPos))
        return 0;
    QtGradientStop *stop = new QtGradientStop();
    stop->setPosition(newPos);
    stop->setColor(color);

    d_ptr->m_posToStop[newPos] = stop;
    d_ptr->m_stopToPos[stop] = newPos;

    emit stopAdded(stop);

    return stop;
}

void QtGradientStopsModel::removeStop(QtGradientStop *stop)
{
    if (!d_ptr->m_stopToPos.contains(stop))
        return;
    if (currentStop() == stop)
        setCurrentStop(0);
    selectStop(stop, false);

    emit stopRemoved(stop);

    qreal pos = d_ptr->m_stopToPos[stop];
    d_ptr->m_stopToPos.remove(stop);
    d_ptr->m_posToStop.remove(pos);
    delete stop;
}

void QtGradientStopsModel::moveStop(QtGradientStop *stop, qreal newPos)
{
    if (!d_ptr->m_stopToPos.contains(stop))
        return;
    if (d_ptr->m_posToStop.contains(newPos))
        return;

    if (newPos > 1.0)
        newPos = 1.0;
    else if (newPos < 0.0)
        newPos = 0.0;

    emit stopMoved(stop, newPos);

    qreal oldPos = stop->position();
    stop->setPosition(newPos);
    d_ptr->m_stopToPos[stop] = newPos;
    d_ptr->m_posToStop.remove(oldPos);
    d_ptr->m_posToStop[newPos] = stop;
}

void QtGradientStopsModel::changeStop(QtGradientStop *stop, const QColor &newColor)
{
    if (!d_ptr->m_stopToPos.contains(stop))
        return;
    if (stop->color() == newColor)
        return;

    emit stopChanged(stop, newColor);

    stop->setColor(newColor);
}

void QtGradientStopsModel::selectStop(QtGradientStop *stop, bool select)
{
    if (!d_ptr->m_stopToPos.contains(stop))
        return;
    bool selected = d_ptr->m_selection.contains(stop);
    if (select == selected)
        return;

    emit stopSelected(stop, select);

    if (select)
        d_ptr->m_selection[stop] = true;
    else
        d_ptr->m_selection.remove(stop);
}

void QtGradientStopsModel::setCurrentStop(QtGradientStop *stop)
{
    if (stop && !d_ptr->m_stopToPos.contains(stop))
        return;
    if (stop == currentStop())
        return;

    emit currentStopChanged(stop);

    d_ptr->m_current = stop;
}

QtGradientStop *QtGradientStopsModel::firstSelected() const
{
    QMap<double, QtGradientStop *> stopList = stops();
    QMap<double, QtGradientStop *>::ConstIterator itStop = stopList.constBegin();
    while (itStop != stopList.constEnd()) {
        QtGradientStop *stop = itStop.value();
        if (isSelected(stop))
            return stop;
        itStop++;
    };
    return 0;
}

QtGradientStop *QtGradientStopsModel::lastSelected() const
{
    QMap<double, QtGradientStop *> stopList = stops();
    QMap<double, QtGradientStop *>::ConstIterator itStop = stopList.constEnd();
    while (itStop != stopList.constBegin()) {
        itStop--;

        QtGradientStop *stop = itStop.value();
        if (isSelected(stop))
            return stop;
    };
    return 0;
}

void QtGradientStopsModel::moveStops(double newPosition)
{
    QtGradientStop *current = currentStop();
    if (!current)
        return;

    double newPos = newPosition;

    if (newPos > 1)
        newPos = 1;
    else if (newPos < 0)
        newPos = 0;

    if (newPos == current->position())
        return;

    double offset = newPos - current->position();

    QtGradientStop *first = firstSelected();
    QtGradientStop *last = lastSelected();

    if (first && last) { // multiselection
        double maxOffset = 1.0 - last->position();
        double minOffset = -first->position();

        if (offset > maxOffset)
            offset = maxOffset;
        else if (offset < minOffset)
            offset = minOffset;

    }

    if (offset == 0)
        return;

    bool forward = (offset > 0) ? false : true;

    QMap<double, QtGradientStop *> stopList;

    QList<QtGradientStop *> selected = selectedStops();
    QListIterator<QtGradientStop *> it(selected);
    while (it.hasNext()) {
        QtGradientStop *stop = it.next();
        stopList[stop->position()] = stop;
    }
    stopList[current->position()] = current;

    QMap<double, QtGradientStop *>::ConstIterator itStop = forward ? stopList.constBegin() : stopList.constEnd();
    while (itStop != (forward ? stopList.constEnd() : stopList.constBegin())) {
        if (!forward)
            itStop--;
        QtGradientStop *stop = itStop.value();
            double pos = stop->position() + offset;
            if (pos > 1)
                pos = 1;
            if (pos < 0)
                pos = 0;

            if (current == stop)
                pos = newPos;

            QtGradientStop *oldStop = at(pos);
            if (oldStop && !stopList.values().contains(oldStop))
                removeStop(oldStop);
            moveStop(stop, pos);

        if (forward)
            itStop++;
    }
}

void QtGradientStopsModel::clear()
{
    QList<QtGradientStop *> stopsList = stops().values();
    QListIterator<QtGradientStop *> it(stopsList);
    while (it.hasNext())
        removeStop(it.next());
}

void QtGradientStopsModel::clearSelection()
{
    QList<QtGradientStop *> stopsList = selectedStops();
    QListIterator<QtGradientStop *> it(stopsList);
    while (it.hasNext())
        selectStop(it.next(), false);
}

void QtGradientStopsModel::selectAll()
{
    QList<QtGradientStop *> stopsList = stops().values();
    QListIterator<QtGradientStop *> it(stopsList);
    while (it.hasNext())
        selectStop(it.next(), true);
}

void QtGradientStopsModel::deleteStops()
{
    QList<QtGradientStop *> selected = selectedStops();
    QListIterator<QtGradientStop *> itSel(selected);
    while (itSel.hasNext()) {
        QtGradientStop *stop = itSel.next();
        removeStop(stop);
    }
    QtGradientStop *current = currentStop();
    if (current)
        removeStop(current);
}


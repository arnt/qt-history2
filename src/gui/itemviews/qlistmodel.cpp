#include "qlistmodel.h"

bool QListModelItem::operator ==(const QListModelItem &other) const
{
    if (values.count() != other.values.count()
	|| edit != other.edit
	|| select != other.select)
	return false;

    for (int i=0; values.count(); ++i)
	if (values.at(i).role != other.values.at(i).role
	    || values.at(i).value != other.values.at(i).value)
	    return false;

    return true;
}

QVariant QListModelItem::data(int role) const
{
    for (int i=0; i<values.count(); ++i) {
	if (values.at(i).role == role)
	    return values.at(i).value;
    }
    return QVariant();
}

void QListModelItem::setData(int role, const QVariant &value)
{
    for (int i=0; i<values.count(); ++i) {
	if (values.at(i).role == role) {
	    values[i].value = value;
	    return;
	}
    }
    values.append(Data(role, value));
    return;
}

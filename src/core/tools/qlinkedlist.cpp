#include "qlinkedlist.h"

QLinkedListData QLinkedListData::shared_null = {
    &QLinkedListData::shared_null, &QLinkedListData::shared_null, Q_ATOMIC_INIT(1), 0
};

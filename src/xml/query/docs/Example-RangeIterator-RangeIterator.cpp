Item::Iterator::Ptr rit(new RangeIterator(1, RangeIterator::Forward, 3));
// rit generates the sequence (1, 2, 3)

rit = Item::Iterator::Ptr(new RangeIterator(-3, RangeIterator::Backward, 0));
// rit generates the sequence (0, -1, -2, -3)

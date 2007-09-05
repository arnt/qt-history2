ItemType::Ptr op1(BuiltinTypes::xsInteger);
const ItemType::Ptr op2(BuiltinTypes::xsString);

op1 = ItemType::Ptr(const_cast<ItemType *>(&(*op1 | *op2)));
ItemType::Ptr simplerOp1(BuiltinTypes::xsInteger);
const ItemType::Ptr simplerOp2(BuiltinTypes::xsString);

simplerOp1 |= simplerOp2;

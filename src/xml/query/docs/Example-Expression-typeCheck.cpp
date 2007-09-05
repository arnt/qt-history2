Expression::Ptr MyExpression::typeCheck(const StaticContext::Ptr &context,
                                        const SequenceType::Ptr &reqType)
{
    // Here we call the super class's typeCheck() function. In this example
    // it is SingleContainer.
    const Expression::Ptr me(EmptyContainer::typeCheck(context, reqType));

    // Do type checks specific to MyExpression

     return me;
}

/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Patternist_QObjectPropertyToAttributeIterator_h
#define Patternist_QObjectPropertyToAttributeIterator_h

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * Remember that the XPath Data Models index starts from 1, while
     * QMetaObject::propertyOffset() starts from 0.
     */
    class QObjectPropertyToAttributeIterator : public Iterator<Item>
    {
    public:
        inline QObjectPropertyToAttributeIterator(const QObjectNodeModel *const nm,
                                                  const QObject *const object) : m_nodeModel(nm)
                                                                               , m_object(object)
                                                                               , m_propertyCount(object->metaObject()->propertyCount())
                                                                               , m_currentPos(0)
        {
        }

        virtual Item next()
        {
            if(m_currentPos == -1 || m_currentPos == m_propertyCount)
            {
                m_currentPos = -1;
                return Item();
            }

            Item retval(m_nodeModel->createNode(m_object, Node::AdditionalData(QObjectNodeModel::IsAttribute | m_currentPos)));
            ++m_currentPos;

            return retval;
        }

        virtual Item current() const
        {
            if(m_currentPos == -1)
                return Item();
            else
                return m_nodeModel->createNode(m_object, QObjectNodeModel::IsAttribute | (m_currentPos - 1));
        }

        virtual xsInteger position() const
        {
            return m_currentPos;
        }

        virtual xsInteger count()
        {
            return m_propertyCount;
        }

        virtual Item::Iterator::Ptr copy() const
        {
            return Item::Iterator::Ptr(new QObjectPropertyToAttributeIterator(m_nodeModel, m_object));
        }

    private:
        const QObjectNodeModel *const   m_nodeModel;
        const QObject *const            m_object;
        const int                       m_propertyCount;
        xsInteger                       m_currentPos;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

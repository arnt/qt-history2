#include <qvaluevector.h>
#include <iostream>

template <class C>
void print( const C& v )
{
    std::cout << "-----> size of container:" << v.size() << std::endl;
    std::cout << "-----> ";
    typename C::const_iterator it = v.begin();
    for ( ; it != v.end(); ++it )
	std::cout << "{" << (*it) << "} ";
    std::cout << endl << endl;
}

int main()
{

    // ctors
    QValueVector<int> v1;
    Q_ASSERT( v1.empty() );
    Q_ASSERT( v1.size() == 0 );
    Q_ASSERT( v1.capacity() >= v1.size() );
    QValueVector<int> v2( v1 );
    Q_ASSERT( v2.empty() );
    Q_ASSERT( v2.size() == 0 );
    Q_ASSERT( v2.capacity() >= v1.size() );
    QValueVector<int> v3( 5 );
    Q_ASSERT( v3.empty() );
    Q_ASSERT( v3.size() == 0 );
    Q_ASSERT( v3.capacity() >= 5 );

    //operator=
    QValueVector<int> v4 = v3;
    Q_ASSERT( v4.empty() );
    Q_ASSERT( v4.size() == 0 );
    Q_ASSERT( v4.capacity() >= 5 );

    // adding elements
    v4.push_back( 1 );
    v4.push_back( 2 );
    v4.push_back( 3 );
    Q_ASSERT( !v4.empty() );
    Q_ASSERT( v3.empty() ); // should have detached
    Q_ASSERT( v4.size() == 3 );
    Q_ASSERT( v4.capacity() >= v4.size() );
    v4.insert( v4.end(), 4 );
    v4.insert( v4.begin(), 0 );
    Q_ASSERT( !v4.empty() );
    Q_ASSERT( v4.size() == 5 );
    Q_ASSERT( v4.capacity() >= v4.size() );

    std::cout << "Should contain 5 elements: 0,1,2,3,4" << std::endl;
    print( v4 );

    // swap
    v4.swap( v3 );
    Q_ASSERT( v4.empty() );
    Q_ASSERT( !v3.empty() );
    Q_ASSERT( v3.size() == 5 );
    Q_ASSERT( v3.capacity() >= v3.size() );
    std::cout << "Should contain 5 elements: 0,1,2,3,4" << std::endl;
    print( v3 );
    std::cout << "Should contain no elements" << std::endl;
    print( v4 );

    // element access
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 1 );
    Q_ASSERT( v3[2] == 2 );
    Q_ASSERT( v3[3] == 3 );
    Q_ASSERT( v3[4] == 4 );
    bool ok = FALSE;
    int& i = v3.at( 1000, &ok );
    Q_ASSERT( !ok );
    int& j = v3.at( 2, &ok );
    Q_ASSERT( ok );

    // iterators
    QValueVector<int>::iterator it = v3.begin();
    int k = 0;
    for ( ; k < 5; ++k, ++it )
	Q_ASSERT( *it == k );
    Q_ASSERT( it == v3.end() );
    --it;
    for ( k = 4; k >= 0; --k, --it )
	Q_ASSERT( *it == k );

    Q_ASSERT( v3.front() == 0 );
    Q_ASSERT( v3.back() == 4 );

    // capacity stuff
    v3.resize( 5 );
    Q_ASSERT( v3.size() == 5 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 1 );
    Q_ASSERT( v3[2] == 2 );
    Q_ASSERT( v3[3] == 3 );
    Q_ASSERT( v3[4] == 4 );
    std::cout << "Should contain 5 elements: 0,1,2,3,4" << std::endl;
    print( v3 );

    v3.resize( 6 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 1 );
    Q_ASSERT( v3[2] == 2 );
    Q_ASSERT( v3[3] == 3 );
    Q_ASSERT( v3[4] == 4 );
    std::cout << "Should contain 6 elements: 0,1,2,3,4,0" << std::endl;
    print( v3 );

    v3.reserve( 1000 );
    Q_ASSERT( v3.size() == 6 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 1 );
    Q_ASSERT( v3[2] == 2 );
    Q_ASSERT( v3[3] == 3 );
    Q_ASSERT( v3[4] == 4 );
    Q_ASSERT( v3.capacity() >= 1000 );
    v3.back() = 5;
    Q_ASSERT( v3.back() == 5 );
    std::cout << "Should contain 6 elements: 0,1,2,3,4,5" << std::endl;
    print( v3 );

    v3.resize( 5 );
    Q_ASSERT( v3.size() == 5 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 1 );
    Q_ASSERT( v3[2] == 2 );
    Q_ASSERT( v3[3] == 3 );
    Q_ASSERT( v3[4] == 4 );
    Q_ASSERT( v3.capacity() >= 1000 );
    std::cout << "Should contain 5 elements: 0,1,2,3,4" << std::endl;
    print( v3 );

    it = v3.end();
    v3.erase( --it );
    Q_ASSERT( v3.size() == 4 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 1 );
    Q_ASSERT( v3[2] == 2 );
    Q_ASSERT( v3[3] == 3 );
    Q_ASSERT( v3.capacity() >= 1000 );
    std::cout << "Should contain 4 elements: 0,1,2,3" << std::endl;
    print( v3 );

    it = v3.begin();
    QValueVector<int>::iterator it2 = v3.end();
    v3.erase( ++it, --it2 );
    Q_ASSERT( v3.size() == 2 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 3 );
    Q_ASSERT( v3.capacity() >= 1000 );
    std::cout << "Should contain 2 elements: 0,3" << std::endl;
    print( v3 );

    it = v3.begin();
    v3.insert( ++it, 9 );
    Q_ASSERT( v3.size() == 3 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 9 );
    Q_ASSERT( v3[2] == 3 );
    Q_ASSERT( v3.capacity() >= 1000 );
    std::cout << "Should contain 3 elements: 0,9,3" << std::endl;
    print( v3 );

    it = v3.begin();
    v3.insert( ++it, 4, 4 );
    Q_ASSERT( v3.size() == 7 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 4 );
    Q_ASSERT( v3[2] == 4 );
    Q_ASSERT( v3[3] == 4 );
    Q_ASSERT( v3[4] == 4 );
    Q_ASSERT( v3[5] == 9 );
    Q_ASSERT( v3[6] == 3 );
    Q_ASSERT( v3.capacity() >= 1000 );
    std::cout << "Should contain 7 elements: 0,4,4,4,4,9,3" << std::endl;
    print( v3 );

    it = v3.begin();
    v3.insert( ++it, 2000, 2 );
    Q_ASSERT( v3.size() == 2007 );
    std::cout << "Should contain 2007 elements: 0,2,2,...2,4,4,4,4,9,3" << std::endl;
    print( v3 );

    it = qFind( v3.begin(), v3.end(), 3 );
    it2 = v3.end();
    Q_ASSERT( it == --it2 );

    v3.resize( 4 );
    Q_ASSERT( v3.size() == 4 );
    Q_ASSERT( v3[0] == 0 );
    Q_ASSERT( v3[1] == 2 );
    Q_ASSERT( v3[2] == 2 );
    Q_ASSERT( v3[3] == 2 );
    Q_ASSERT( v3.capacity() >= 2007 );
    std::cout << "Should contain 4 elements: 0,2,2,2" << std::endl;
    print( v3 );

    it = v3.begin();
    v3.insert( ++it, 2000, 2 );
    v3.push_back( 9 );
    v3.push_back( 3 );
    it = v3.begin();
    it2 = v3.end();
    v3.erase( ++it, ----it2 );
    Q_ASSERT( v3.size() == 3 );
    std::cout << "Should contain 3 elements: 0,9,3" << std::endl;
    print( v3 );

    v3.pop_back();
    Q_ASSERT( v3.size() == 2 );
    std::cout << "Should contain 3 elements: 0,9" << std::endl;
    print( v3 );

    // instantiate other member functions
    QValueVector<int>::const_iterator cit = v3.begin();
    cit = v3.end();
    QValueVector<int>::size_type max_size = v3.max_size();
    std::cout << "max size of vector:" << max_size << std::endl;
    const int& ci = v3.at( 1 );
    const int& ci2 = v3[1];
    const int& ci3 = v3.front();
    const int& ci4 = v3.back();
    v3.clear();

    return 0;
}

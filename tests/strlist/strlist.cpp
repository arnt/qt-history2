
// Exercising QStrList's allocation, deallocation, and copying routines.

// It makes little sense to run this test program outside Purify or equivalent.
// It should have no memory leaks our double deletions.

#include "qstrlist.h"

void test1()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList();

    l1->append( s1 );
    l1->append( s2 );
    l1->append( s3 );

    QStrList* l2 = new QStrList( l1 );

    delete l1;

    delete l2;

    delete[] s1;
    delete[] s2;
    delete[] s3;
}    


void test2()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList(FALSE);

    l1->append( s1 );
    l1->append( s2 );
    l1->append( s3 );

    QStrList* l2 = new QStrList( l1 );

    delete l1;

    delete l2;

    delete[] s1;
    delete[] s2;
    delete[] s3;
}    


void test3()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList();

    l1->append( s1 );
    l1->append( s2 );

    QStrList* l2 = new QStrList( l1 );

    l2->append( s3 );

    delete l1;

    delete l2;

    delete[] s1;
    delete[] s2;
    delete[] s3;
}    


void test4()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList(FALSE);

    l1->append( s1 );
    l1->append( s2 );

    QStrList* l2 = new QStrList( l1 );

    l2->append( s3 );

    delete l1;

    delete l2;

    delete[] s1;
    delete[] s2;
    delete[] s3;
}


void test5()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList(FALSE);

    l1->append( s1 );
    l1->append( s2 );

    QStrList* l2 = new QStrList();

    l2->append( s3 );

    *l2 = *l1;

    delete l1;

    delete l2;

    delete[] s1;
    delete[] s2;
    delete[] s3;
}


void test6()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList(FALSE);
    l1->setAutoDelete( TRUE );
    l1->append( s1 );
    l1->append( s2 );
    l1->append( s3 );

    delete l1;
}


void test7()
{
    QStrList* l1 = new QStrList();
    l1->setAutoDelete( FALSE );
    l1->append( "A-String1" );
    l1->append( "B-String1" );
    l1->append( "C-String1" );

    char* s1 = l1->at(0);
    char* s2 = l1->at(1);
    char* s3 = l1->at(2);

    delete l1;

    delete[] s1;
    delete[] s2;
    delete[] s3;

}


void test8()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList(FALSE);
    l1->setAutoDelete( TRUE );
    l1->append( s1 );
    l1->append( s2 );
    l1->append( s3 );

    QStrList* l2 = new QStrList();
    l2->append( "hei" );
    *l2 = *l1;
    
    delete l1;
    delete l2;
}

void test9()
{
    char* s1 = qstrdup( "A-String1" );
    char* s2 = qstrdup( "B-String1" );
    char* s3 = qstrdup( "C-String1" );

    QStrList* l1 = new QStrList(FALSE);
    l1->setAutoDelete( TRUE );
    l1->append( s1 );
    l1->append( s2 );
    l1->append( s3 );

    QStrList* l2 = new QStrList( l1 );
    l2->append( "hei" );
    
    delete l2;
    delete l1;
}


void test10()
{
    QStrList* l1 = new QStrList();
    l1->setAutoDelete( FALSE );
    l1->append( "A-String1" );
    l1->append( "B-String1" );
    l1->append( "C-String1" );

    char* s1 = l1->at(0);
    char* s2 = l1->at(1);
    char* s3 = l1->at(2);

    QStrList* l2 = new QStrList( l1 );
    l2->append( "hei" );

    delete l1;
    delete l2;

    delete[] s1;
    delete[] s2;
    delete[] s3;

}


void test11()
{
    QStrList* l1 = new QStrList();
    l1->setAutoDelete( FALSE );
    l1->append( "A-String1" );
    l1->append( "B-String1" );
    l1->append( "C-String1" );

    char* s1 = l1->at(0);
    char* s2 = l1->at(1);
    char* s3 = l1->at(2);

    QStrList* l2 = new QStrList();
    l2->append( "hei" );
    *l2 = *l1;

    delete l1;
    delete l2;

    delete[] s1;
    delete[] s2;
    delete[] s3;

}

int main( int, char** )
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
    return 0;
}

	

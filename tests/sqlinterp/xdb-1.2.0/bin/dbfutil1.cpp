/*  dbf utility program  */

#include <xdb/xbase.h>

xbXBase x;

xbDbf d( &x );

class MyClass {
  public:
    MyClass() {};
    int OpenFile();
    int CloseFile();
    int ConvertDatabase();
    int ReindexDatabase();
    int DeleteMemoField();
    int UpdateMemoField();
    int GetRecord();
    int DumpDbtHeader();
    int BlankRecord();
    int PutRecord();
   
    void FindMemoBlocks();
    void GetMemoBlocks();
 
    void FileStats();
    void MainMenu();
    void FileMenu();
    void RecordMenu();
    void FieldMenu();
    void IndexMenu();
    void DebugMenu();
  public:
};
/************************************************************************/
/* stats */
void MyClass::FileStats()
{
  cout << "Number of records = " << d.NoOfRecords() << endl;
}  
/************************************************************************/
/* open database */
int MyClass::OpenFile()
{
  int rc;
  char filename[50];
  cout << "Enter database file name: ";
  cin >> filename;

  rc = d.OpenDatabase( filename );
  cout << "Return Code " << rc << endl;
  return rc;
}
/************************************************************************/
/* close database */
int MyClass::CloseFile()
{
  int rc;
  rc = d.CloseDatabase();
  cout << "Return Code " << rc << endl;
  return rc;
}
/************************************************************************/
/* convefrt database */
int MyClass::ConvertDatabase()
{
  int     rc;
  xbShort FieldNo;
  xbLong  len, FieldCnt;
  xbDbf   d2( &x );
  char    *MemoFieldBuf = NULL;
  xbLong  MemoFieldLen = 0L;

  char filename[50];
  cout << "Enter database file name to convert to: ";
  cin >> filename;

  if(( rc = d2.OpenDatabase( filename )) != XB_NO_ERROR ){
    cout << "Error " << rc << " opening database" << endl;
    return rc;
  }

  rc = d.GetFirstRecord();
  FieldCnt = d.FieldCount();
  while( rc == XB_NO_ERROR )
  {
    for( xbShort i = 0; i < FieldCnt++; i++ )
    {
      if(( FieldNo = d2.GetFieldNo( d.GetFieldName( i ))) == -1 )
        cout << "Error converting field " << d.GetFieldName( i ) << endl;
      else 
      {
        if( d.GetFieldType( i ) != d2.GetFieldType( FieldNo ))
          cout << "Error - different field types " << d.GetFieldName(i);
        else
        {
          if( d.GetFieldType( i ) == 'M' )
          {
#ifdef XB_MEMO_FIELDS
            len = d.GetMemoFieldLen( i ); 
            if( len > MemoFieldLen )
            {
              if( MemoFieldLen > 0 ) 
                delete MemoFieldBuf;
              MemoFieldBuf = new char[len];
              MemoFieldLen = len;
            }
#endif
//          d.GetMemoData( i, len, MemoFieldBuf, F_SETLKW );              
//          d2.PutMemoData( FieldNo, len, MemoFieldBuf, F_SETLKW );
          }
          else
            d2.PutField( FieldNo, d.GetField(i));
        } 
      }
    }
  }
  if( MemoFieldLen > 0 ) 
    delete MemoFieldBuf;
  return 0;
}
/************************************************************************/
/* open database */
int MyClass::ReindexDatabase()
{
  xbNdx n(&d);
  int rc;
  char filename[50];
  cout << "Enter index file name: ";
  cin >> filename;

  if(( rc = n.OpenIndex( filename )) != XB_NO_ERROR ){
    cout << "Error " << rc << " Opening index" << endl;
    return rc;
  }

  if(( rc = n.ReIndex()) != XB_NO_ERROR )
    cout << "Error " << rc << " Reindexing database" << endl;

  return rc;
}
/************************************************************************/
int MyClass::GetRecord()
{
  int rc;
  xbLong RecNo;
  cout << "Enter record number: ";
  cin >> RecNo;

  rc = d.GetRecord( RecNo );
  cout << "Return Code = " << rc << endl;
  return rc;
}
/************************************************************************/
int MyClass::UpdateMemoField()
{
#ifdef XB_MEMO_FIELDS
  xbShort fn,rc;
  xbLong len;
  char bufchar[2];
  char *buf;

  cout << "Enter Field Number: " << endl;
  cin >> fn;
  if( fn < 0 || fn > d.FieldCount()){
    cout << "Invalid Field Number" << endl;
    return 0;
  }
  if( d.GetFieldType( fn ) != 'M' ){
    cout << "Field " << fn << " is not a memo field" << endl;
    cout << "Field Name = " << d.GetFieldName( fn ) << " type= ";
    cout << d.GetFieldType( fn ) << endl;
    return 0;
  }
  cout << "Enter length of memo data:" << endl;
  cin >> len;
 
  cout << "Enter character to populate memo data:" << endl;
  cin >> bufchar;

  buf = new char[len];
  memset( buf, bufchar[0], len );

  rc = d.UpdateMemoData( fn, len, buf, F_SETLKW );
  cout << "Return Code = " << rc << endl;
  return rc;
#else
  cout << "\nXB_MEMO_FIELDS is not compiled in\n";
  return 0;
#endif
}
/************************************************************************/
int MyClass::DeleteMemoField()
{
#ifdef XB_MEMO_FIELDS
  xbShort fn,rc;
  cout << "Enter Field Number: " << endl;
  cin >> fn;
  if( fn < 0 || fn > d.FieldCount()){
    cout << "Invalid Field Number" << endl;
    return 0;
  }
  if( d.GetFieldType( fn ) != 'M' ){
    cout << "Field " << fn << " is not a memo field" << endl;
    cout << "Field Name = " << d.GetFieldName( fn ) << " type= ";
    cout << d.GetFieldType( fn ) << endl;
    return 0;
  }
  rc = d.UpdateMemoData( fn, 0, 0, F_SETLKW );
  cout << "Return Code = " << rc << endl;
  return rc; 
#else
  cout << "\nXB_MEMO_FIELDS is not compiled in\n";
  return 0;
#endif
}
/************************************************************************/
int MyClass::DumpDbtHeader()
{
  int rc = 0;

#ifdef XB_MEMO_FIELDS
#ifdef XBASE_DEBUG
/* FIXME?	Gary, do you mean header or free page list? */
  rc = d.DumpMemoFreeChain();
  cout << "\nFuncion Return Code = " << rc << endl;
#else
  cout << "\nXBASE_DEBUG is not compiled in\n";
#endif
#else
  cout << "\nXB_MEMO_FIELDS is not compiled in\n";
#endif
  return rc;
}
/************************************************************************/
void MyClass::GetMemoBlocks()
{
#ifdef XB_MEMO_FIELDS
  xbLong BlocksNeeded, Location, PrevNode;
  int rc;

  cout << "Enter number of blocks: " << endl;
  cin  >> BlocksNeeded;
  cout << "Enter starting location: " << endl;
  cin  >> Location;
  cout << "Enter previous node: " << endl;
  cin  >> PrevNode;
  rc = d.GetBlockSetFromChain( BlocksNeeded, Location, PrevNode );
  cout << "Return code = " << rc << endl;
#else
  cout << "\nXB_MEMO_FIELDS is not compiled in\n";
#endif
}
/************************************************************************/
void MyClass::FindMemoBlocks()
{
#ifdef XB_MEMO_FIELDS
  xbLong BlocksNeeded, Location, PrevNode;
  int rc;

  cout << "Enter number of blocks: " << endl;
  cin  >> BlocksNeeded;
  rc = d.FindBlockSetInChain( BlocksNeeded, 0, Location, PrevNode );
  cout << "Return code = " << rc << endl;
  cout << "Location = " << Location << endl;
  cout << "Previous Node = " << PrevNode << endl;
#else
  cout << "\nXB_MEMO_FIELDS is not compiled in\n";
#endif
}
/************************************************************************/
void MyClass::IndexMenu()
{
  cout << "not available" << endl;
  return;
}
/************************************************************************/
void MyClass::DebugMenu()
{
  int option = 0;

  while( option != 99 ) {
   cout << endl << endl << "Debug Menu" << endl;
   cout << "1 - Dislay DBT Header" << endl;
   cout << "2 - Find Memo Blocks" << endl;
   cout << "3 - Get Memo Blocks" << endl;
   cout << "99 - Exit Menu" << endl;
   cin >> option;
   switch( option ){
    case 1:  DumpDbtHeader();  break;
    case 2:  FindMemoBlocks(); break;
    case 3:  GetMemoBlocks();  break;
    case 99: break;
    default: cout << "Invalid option" << endl; break;
   }
  }
}
/************************************************************************/
void MyClass::FieldMenu()
{
  int option = 0;

  while( option != 99 ) {
   cout << endl << endl << "Field Menu" << endl;
   cout << "1  - Delete Memo Field" << endl;
   cout << "2  - Update Memo Field" << endl;
   cout << "99 - Exit Menu" << endl;
   cin >> option;
   switch( option ){
    case 1:  DeleteMemoField(); break;
    case 2:  UpdateMemoField(); break;
    case 99: break;
    default: cout << "Function not available" << endl; break;
   }
 }
}
/************************************************************************/
void MyClass::RecordMenu()
{
  int option = 0;
  cout << "File Menu" << endl;
  while( option != 99 ) {
   cout << endl << endl << "Record Menu" << endl;
   cout << "1  - Get Record" << endl;
   cout << "2  - Blank Record" << endl;
   cout << "3  - Append Record" << endl;
   cout << "4  - Put Record" << endl;
   cout << "99 - Exit Menu" << endl;
   cin >> option;
   switch( option ){
    case 1:  GetRecord(); break;
    case 99: break;
    default: cout << "Invalid option" << endl; break;
   }
  }
}
/************************************************************************/
void MyClass::FileMenu()
{
  int option = 0;

  while( option != 99 ) {
   cout << endl << endl << "File Menu" << endl;
   cout << "1  - Open File" << endl;
   cout << "2  - Close File" << endl;
   cout << "3  - File Stats" << endl; 
   cout << "99 - Exit Menu" << endl;
   cin  >> option;

   switch( option ){
    case 1:  OpenFile();  break;
    case 2:  CloseFile(); break;
    case 3:  FileStats(); break;
    case 99: break;
    default: cout << "Invalid Option" << endl;
   }
 }
}
/************************************************************************/
void MyClass::MainMenu()
{
  int option = 0;
  cout << endl<< endl << "XBase Utility Program";
  while( option != 99 ) {
   cout << endl << endl << "Main Menu" << endl;
   cout << "1  - File Menu" << endl;
   cout << "2  - Record Menu" << endl; 
   cout << "3  - Field Menu" << endl;
   cout << "4  - Index Menu" << endl;
   cout << "5  - Debug Menu" << endl;
   cout << "99 - Exit" << endl;
   cin >> option;
   switch( option ){
     case 1:  FileMenu();   break;
     case 2:  RecordMenu(); break;
     case 3:  FieldMenu();  break;
     case 4:  IndexMenu();  break;
     case 5:  DebugMenu();  break;
     case 99: cout << "Bye!! - Thanks for using XBase" << endl; break;
     default: cout << "Invalid function" << endl; break;
   }
  }
}
/************************************************************************/
int main(int ac,char** av)
{
  MyClass m;
  m.MainMenu();
    return 0;
}

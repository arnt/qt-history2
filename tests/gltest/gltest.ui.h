void GLTest::init()
{
    int i;
    QListViewItem *item;
    QStringList list, listItem;
    GLInfo info(this,"info");
    infoView->setText(info.getText());
    list = info.getViewList();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	i = 0;
	item = new QListViewItem(infoList);
	listItem = QStringList::split(" ", (*it).latin1());
	for ( QStringList::Iterator ti = listItem.begin(); ti != listItem.end(); ++ti ) {
	    item->setText(i, (*ti).latin1());
	    i++;
	}
	infoList->insertItem(item);	
    }  
    
#if defined(Q_OS_WIN32)
    infoList->setColumnText(0, "Nr");
    infoList->setColumnText(1, "ColorBits");
    infoList->setColumnText(2, "Draw to");
#endif
    
}

void GLTest::destroy()
{

}


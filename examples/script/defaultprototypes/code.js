listWidget.addItem("Red");
listWidget.addItem("Blue");
listWidget.addItem("Green");
listWidget.addItem("Cyan");
listWidget.addItem("Yellow");
listWidget.addItem("Purple");
listWidget.addItems(["Orange", "Gray"]);

listWidget.itemClicked.connect(
    function(item)
    {
        this.setBackgroundColor(item.text);
    }
);

listWidget.show();

function Calculator(ui)
{
    this.ui = ui;

    this.pendingAdditiveOperator = "";
    this.pendingMultiplicativeOperator = "";
    this.sumInMemory = 0;
    this.sumSoFar = 0;
    this.factorSoFar = 0;
    this.waitingForOperand = true;

    this.abortOperation = function()
    {
        this.clearAll();
        this.ui.display.text = "####";
    }

    this.calculate = function(rightOperand, pendingOperator)
    {
        if (pendingOperator == "+") {
            this.sumSoFar += rightOperand;
        } else if (pendingOperator == "-") {
            this.sumSoFar -= rightOperand;
        } else if (pendingOperator == "*") {
            this.factorSoFar *= rightOperand;
        } else if (pendingOperator == "/") {
            if (rightOperand == 0)
            return false;
            this.factorSoFar /= rightOperand;
        }
        return true;
    }

    this.digitClicked = function()
    {
        var digitValue = this.sender.text - 0;
        if ((digitValue == 0) && (this.ui.display.text == "0"))
            return;
        if (this.waitingForOperand) {
            this.ui.display.clear();
            this.waitingForOperand = false;
        }
        this.ui.display.text += digitValue;
    }

    this.unaryOperatorClicked = function()
    {
        var operand = this.ui.display.text - 0;
        var result = 0;
        if (this.sender.text == "Sqrt") {
            if (operand < 0) {
                this.abortOperation();
                return;
            }
            result = Math.sqrt(operand);
        } else if (this.sender.text == "x^2") {
            result = Math.pow(operand, 2);
        } else if (this.sender.text == "1/x") {
            if (operand == 0.0) {
                this.abortOperation();
                return;
            }
            result = 1 / operand;
        }
        this.ui.display.text = result + "";
        this.waitingForOperand = true;
    }

    this.additiveOperatorClicked = function()
    {
        var operand = this.ui.display.text - 0;

        if (this.pendingMultiplicativeOperator.length != 0) {
            if (!this.calculate(operand, this.pendingMultiplicativeOperator)) {
                this.abortOperation();
            return;
            }
            this.ui.display.text = this.factorSoFar + "";
            operand = this.factorSoFar;
            this.factorSoFar = 0;
            this.pendingMultiplicativeOperator = "";
        }

        if (this.pendingAdditiveOperator.length != 0) {
            if (!this.calculate(operand, this.pendingAdditiveOperator)) {
                this.abortOperation();
                return;
            }
            this.ui.display.text = this.sumSoFar + "";
        } else {
            this.sumSoFar = operand;
        }

        this.pendingAdditiveOperator = this.sender.text;
        this.waitingForOperand = true;
    }

    this.multiplicativeOperatorClicked = function()
    {
        var operand = this.ui.display.text - 0;

        if (this.pendingMultiplicativeOperator.length != 0) {
            if (!this.calculate(operand, this.pendingMultiplicativeOperator)) {
                this.abortOperation();
            return;
            }
            this.ui.display.text = this.factorSoFar + "";
        } else {
            this.factorSoFar = operand;
        }

        this.pendingMultiplicativeOperator = this.sender.text;
        this.waitingForOperand = true;
    }

    this.equalClicked = function()
    {
        operand = this.ui.display.text - 0;

        if (this.pendingMultiplicativeOperator.length != 0) {
            if (!this.calculate(operand, this.pendingMultiplicativeOperator)) {
                this.abortOperation();
                return;
            }
            operand = this.factorSoFar;
            this.factorSoFar = 0.0;
            this.pendingMultiplicativeOperator = "";
        }
        if (this.pendingAdditiveOperator.length != 0) {
            if (!this.calculate(operand, this.pendingAdditiveOperator)) {
                this.abortOperation();
    	    return;
            }
            this.pendingAdditiveOperator = "";
        } else {
            this.sumSoFar = operand;
        }

        this.ui.display.text = this.sumSoFar + "";
        this.sumSoFar = 0.0;
        this.waitingForOperand = true;
    }

    this.pointClicked = function()
    {
        if (this.waitingForOperand)
            this.ui.display.text = "0";
        if (this.ui.display.text.indexOf(".") == -1)
            this.ui.display.text += ".";
        this.waitingForOperand = false;
    }

    this.changeSignClicked = function()
    {
        var text = this.ui.display.text;
        var value = text - 0;

        if (value > 0) {
            text = "-" + text;
        } else if (value < 0) {
            text = text.slice(1);
        }
        this.ui.display.text = text;
    }

    this.backspaceClicked = function()
    {
        if (this.waitingForOperand)
            return;

        var text = this.ui.display.text;
        text = text.slice(0, -1);
        if (text.length == 0) {
            text = "0";
            this.waitingForOperand = true;
        }
        this.ui.display.text = text;
    }

    this.clear = function()
    {
        if (this.waitingForOperand)
            return;

        this.ui.display.text = "0";
        this.waitingForOperand = true;
    }

    this.clearAll = function()
    {
        this.sumSoFar = 0.0;
        this.factorSoFar = 0.0;
        this.pendingAdditiveOperator = "";
        this.pendingMultiplicativeOperator = "";
        this.ui.display.text = "0";
        this.waitingForOperand = true;
    }

    this.clearMemory = function()
    {
        this.sumInMemory = 0.0;
    }

    this.readMemory = function()
    {
        this.ui.display.text = this.sumInMemory + "";
        this.waitingForOperand = true;
    }

    this.setMemory = function()
    {
        this.equalClicked();
        this.sumInMemory = this.ui.display.text - 0;
    }

    this.addToMemory = function()
    {
        this.equalClicked();
        this.sumInMemory += this.ui.display.text - 0;
    }

    with (ui) {
        display.text = "0";

        zeroButton.clicked.connect(this, this.digitClicked);
        oneButton.clicked.connect(this, "digitClicked");
        twoButton.clicked.connect(this, "digitClicked");
        threeButton.clicked.connect(this, "digitClicked");
        fourButton.clicked.connect(this, "digitClicked");
        fiveButton.clicked.connect(this, "digitClicked");
        sixButton.clicked.connect(this, "digitClicked");
        sevenButton.clicked.connect(this, "digitClicked");
        eightButton.clicked.connect(this, "digitClicked");
        nineButton.clicked.connect(this, "digitClicked");

        pointButton.clicked.connect(this, "pointClicked");
        changeSignButton.clicked.connect(this, "changeSignClicked");

        backspaceButton.clicked.connect(this, "backspaceClicked");
        clearButton.clicked.connect(this, "clear");
        clearAllButton.clicked.connect(this, "clearAll");

        clearMemoryButton.clicked.connect(this, "clearMemory");
        readMemoryButton.clicked.connect(this, "readMemory");
        setMemoryButton.clicked.connect(this, "setMemory");
        addToMemoryButton.clicked.connect(this, "addToMemory");
  
        divisionButton.clicked.connect(this, "multiplicativeOperatorClicked");
        timesButton.clicked.connect(this, "multiplicativeOperatorClicked");
        minusButton.clicked.connect(this, "additiveOperatorClicked");
        plusButton.clicked.connect(this, "additiveOperatorClicked");

        squareRootButton.clicked.connect(this, "unaryOperatorClicked");
        powerButton.clicked.connect(this, "unaryOperatorClicked");
        reciprocalButton.clicked.connect(this, "unaryOperatorClicked");
        equalButton.clicked.connect(this, "equalClicked");
    }
}
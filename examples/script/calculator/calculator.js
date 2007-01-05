function bindSlot(fun, obj, sender)
{
    return function() {
        obj.sender = sender;
        fun.apply(obj, arguments);
        obj.sender = undefined;
    }
}

function connect(sender, signal, target, slot)
{
    return sender[signal].connect(bindSlot(target[slot], target, sender));
}

calc.abortOperation = function()
{
    this.clearAll();
    this.display.text = "####";
}

calc.calculate = function(rightOperand, pendingOperator)
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

calc.digitClicked = function()
{
    var digitValue = this.sender.text - 0;
    if ((digitValue == 0) && (this.sender.text == "0"))
        return;
    if (this.waitingForOperand) {
        this.display.clear();
	this.waitingForOperand = false;
    }
    this.display.text += digitValue;
}

calc.unaryOperatorClicked = function()
{
    var operand = this.display.text - 0;
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
    this.display.text = result + "";
    this.waitingForOperand = true;
}

calc.additiveOperatorClicked = function()
{
    var operand = this.display.text - 0;

    if (this.pendingMultiplicativeOperator.length != 0) {
        if (!this.calculate(operand, this.pendingMultiplicativeOperator)) {
            this.abortOperation();
	    return;
        }
        this.display.text = this.factorSoFar + "";
        operand = this.factorSoFar;
        this.factorSoFar = 0;
        this.pendingMultiplicativeOperator = "";
    }

    if (this.pendingAdditiveOperator.length != 0) {
        if (!this.calculate(operand, this.pendingAdditiveOperator)) {
            this.abortOperation();
	    return;
        }
        this.display.text = this.sumSoFar + "";
    } else {
        this.sumSoFar = operand;
    }

    this.pendingAdditiveOperator = this.sender.text;
    this.waitingForOperand = true;
}

calc.multiplicativeOperatorClicked = function()
{
    var operand = this.display.text - 0;

    if (this.pendingMultiplicativeOperator.length != 0) {
        if (!this.calculate(operand, this.pendingMultiplicativeOperator)) {
            this.abortOperation();
	    return;
        }
        this.display.text = this.factorSoFar + "";
    } else {
        this.factorSoFar = operand;
    }

    this.pendingMultiplicativeOperator = this.sender.text;
    this.waitingForOperand = true;
}

calc.equalClicked = function()
{
    operand = this.display.text - 0;

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

    this.display.text = this.sumSoFar + "";
    this.sumSoFar = 0.0;
    this.waitingForOperand = true;
}

calc.pointClicked = function()
{
    if (this.waitingForOperand)
        this.display.text = "0";
    if (this.display.text.indexOf(".") == -1)
        this.display.text += ".";
    this.waitingForOperand = false;
}

calc.changeSignClicked = function()
{
    var text = this.display.text;
    var value = text - 0;

    if (value > 0) {
        text = "-" + text;
    } else if (value < 0) {
        text = text.slice(1);
    }
    this.display.text = text;
}

calc.backspaceClicked = function()
{
    if (this.waitingForOperand)
        return;

    var text = this.display.text;
    text = text.slice(0, -1);
    if (text.length == 0) {
        text = "0";
        this.waitingForOperand = true;
    }
    this.display.text = text;
}

calc.clear = function()
{
    if (this.waitingForOperand)
        return;

    this.display.text = "0";
    this.waitingForOperand = true;
}

calc.clearAll = function()
{
    this.sumSoFar = 0.0;
    this.factorSoFar = 0.0;
    this.pendingAdditiveOperator = "";
    this.pendingMultiplicativeOperator = "";
    this.display.text = "0";
    this.waitingForOperand = true;
}

calc.clearMemory = function()
{
    this.sumInMemory = 0.0;
}

calc.readMemory = function()
{
    this.display.text = this.sumInMemory + "";
    this.waitingForOperand = true;
}

calc.setMemory = function()
{
    this.equalClicked();
    this.sumInMemory = this.display.text - 0;
}

calc.addToMemory = function()
{
    this.equalClicked();
    this.sumInMemory += this.display.text - 0;
}

calc.init = function()
{
    this.pendingAdditiveOperator = "";
    this.pendingMultiplicativeOperator = "";
    this.sumInMemory = 0;
    this.sumSoFar = 0;
    this.factorSoFar = 0;
    this.waitingForOperand = true;

    with (this) {
        display.text = "0";

        connect(zeroButton, "clicked", this, "digitClicked");
        connect(oneButton, "clicked", this, "digitClicked");
        connect(twoButton, "clicked", this, "digitClicked");
        connect(threeButton, "clicked", this, "digitClicked");
        connect(fourButton, "clicked", this, "digitClicked");
        connect(fiveButton, "clicked", this, "digitClicked");
        connect(sixButton, "clicked", this, "digitClicked");
        connect(sevenButton, "clicked", this, "digitClicked");
        connect(eightButton, "clicked", this, "digitClicked");
        connect(nineButton, "clicked", this, "digitClicked");

        connect(pointButton, "clicked", this, "pointClicked");
        connect(changeSignButton, "clicked", this, "changeSignClicked");

        connect(backspaceButton, "clicked", this, "backspaceClicked");
        connect(clearButton, "clicked", this, "clear");
        connect(clearAllButton, "clicked", this, "clearAll");

        connect(clearMemoryButton, "clicked", this, "clearMemory");
        connect(readMemoryButton, "clicked", this, "readMemory");
        connect(setMemoryButton, "clicked", this, "setMemory");
        connect(addToMemoryButton, "clicked", this, "addToMemory");
  
        connect(divisionButton, "clicked", this, "multiplicativeOperatorClicked");
        connect(timesButton, "clicked", this, "multiplicativeOperatorClicked");
        connect(minusButton, "clicked", this, "additiveOperatorClicked");
        connect(plusButton, "clicked", this, "additiveOperatorClicked");

        connect(squareRootButton, "clicked", this, "unaryOperatorClicked");
        connect(powerButton, "clicked", this, "unaryOperatorClicked");
        connect(reciprocalButton, "clicked", this, "unaryOperatorClicked");
        connect(equalButton, "clicked", this, "equalClicked");
    }
}

calc.init();

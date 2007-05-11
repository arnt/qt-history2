__setupPackage__("qt.core");

//if (__cppPlugin__ == undefined) {
//    print("Warning: qt.core.geom: C++ plugin not found; C++ <--> script value conversion won't work");
//}

/////////////////////////////
//////// Point class ////////
/////////////////////////////

qt.core.Point = function(x, y)
{
    this.x = (x == undefined) ? 0 : x - 0;
    this.y = (y == undefined) ? 0 : y - 0;
}

qt.core.Point.prototype = new qt.core.Point();

qt.core.Point.prototype.__defineGetter__(
    "isNull",
     function() { return (this.x == 0) && (this.y == 0); } );

qt.core.Point.prototype.__defineGetter__(
    "manhattanLength",
     function() { return Math.abs(this.x) + Math.abs(this.y); } );

qt.core.Point.prototype.translate = function() {
    if (arguments.length == 2) {
        this.x += arguments[0];
        this.y += arguments[1];
    } else if (arguments.length == 1) {
        this.x += arguments[0].x;
        this.y += arguments[0].y;
    } else {
        throw new Error("Point.prototype.translate: invalid number of arguments");
    }
    return this;
}

qt.core.Point.prototype.clone = function() {
    return new this.constructor(this.x, this.y);
}

qt.core.Point.prototype.toString = function() {
    return "Point(" + this.x + ", " +this.y + ")";
}

////////////////////////////
//////// Size class ////////
////////////////////////////

qt.core.Size = function(width, height)
{
    this.width = (width == undefined) ? -1 : width - 0;
    this.height = (height == undefined) ? -1 : height - 0;
}

qt.core.Size.prototype = new qt.core.Size(0, 0);

qt.core.Size.prototype.__defineGetter__(
    "isEmpty",
    function() {
        return (this.width <= 0) || (this.height <= 0);
    } );

qt.core.Size.prototype.__defineGetter__(
    "isNull",
    function() {
        return (this.width == 0) && (this.height == 0);
    } );

qt.core.Size.prototype.__defineGetter__(
    "isValid",
    function() {
        return (this.width >= 0) && (this.height >= 0);
    } );

qt.core.Size.prototype.boundedTo = function(otherSize) {
    return new this.constructor(Math.min(this.width, otherSize.width),
                                Math.min(this.height, otherSize.height));
}

qt.core.Size.prototype.expandedTo = function(otherSize) {
    return new this.constructor(Math.max(this.width, otherSize.width),
                                Math.max(this.height, otherSize.height));
}

qt.core.Size.prototype.transpose = function() {
    var tmp = this.width;
    this.width = this.height;
    this.height = tmp;
    return this;
}

qt.core.Size.prototype.scale = function() {
    var wd;
    var ht;
    var mode;
    if (arguments.length < 2)
        throw new Error("Size.prototype.scale: too few arguments");
    if (arguments.length == 2) {
        wd = arguments[0].width - 0;
        ht = arguments[0].height - 0;
        mode = arguments[1];
    } else {
        wd = arguments[0] - 0;
        ht = arguments[1] - 0;
        mode = arguments[2];
    }
    if (mode == qt.core.IgnoreAspectRatio) {
        this.width = wd;
        this.height = ht;
    } else {
        var useHeight;
        var rw = ht * this.width / this.height;

        if (mode == qt.core.KeepAspectRatio) {
            useHeight = (rw <= wd);
        } else { // mode == qt.core.KeepAspectRatioByExpanding
            useHeight = (rw >= wd);
        }

        if (useHeight) {
            this.width = rw;
            this.height = ht;
        } else {
            this.height = wd * this.height / this.width;
            this.width = wd;
        }
    }
    return this;
}

qt.core.Size.prototype.clone = function() {
    return new this.constructor(this.width, this.height);
}

qt.core.Size.prototype.toString = function() {
    return "Size(" + this.width + ", " + this.height + ")";
}

////////////////////////////
//////// Line class ////////
////////////////////////////

qt.core.Line = function() {
    if (arguments.length == 4) {
        this.p1 = new qt.core.Point(arguments[0], arguments[1]);
        this.p2 = new qt.core.Point(arguments[2], arguments[3]);
    } else if (arguments.length == 2) {
        this.p1 = arguments[0].clone();
        this.p2 = arguments[1].clone();
    } else if (arguments.length == 0) {
        this.p1 = new qt.core.Point();
        this.p2 = new qt.core.Point();
    } else {
        throw new Error("Line: invalid number of arguments");
    }
}

qt.core.Line.prototype = new qt.core.Line();

qt.core.Line.prototype.__defineGetter__(
    "x1", function() { return this.p1.x; } );

qt.core.Line.prototype.__defineGetter__(
    "y1", function() { return this.p1.y; } );

qt.core.Line.prototype.__defineGetter__(
    "x2", function() { return this.p2.x; } );

qt.core.Line.prototype.__defineGetter__(
    "y2", function() { return this.p2.y; } );

qt.core.Line.prototype.__defineGetter__(
    "dx", function() { return this.p2.x - this.p1.x; } );

qt.core.Line.prototype.__defineGetter__(
    "dy", function() { return this.p2.y - this.p1.y; } );

qt.core.Line.prototype.__defineGetter__(
    "isNull",
    function() {
        return (this.p1.x == this.p2.x) && (this.p1.y == this.p2.y);
    }
);

qt.core.Line.prototype.__defineGetter__(
    "length",
    function() {
        return Math.sqrt(this.dx*this.dx + this.dy*this.dy);
    }
);

qt.core.Line.prototype.__defineSetter__(
    "length",
    function(len) {
        if (this.isNull)
            return;
        var v = this.unitVector;
        this.p2.x = this.p1.x + v.dx * len;
        this.p2.y = this.p1.y + v.dy * len;
    }
);

qt.core.Line.prototype.getAngle = function(line) {
    if (this.isNull || line.isNull)
        return 0;
    var cos_line = (this.dx*line.dx + this.dy*line.dy) / (this.length*line.length);
    var rad = 0;
    // only accept cos_line in the range [-1,1], if it is outside, use 0
    // (we return 0 rather than PI for those cases)
    if (cos_line >= -1.0 && cos_line <= 1.0) rad = Math.acos( cos_line );
    return rad * 360 / (2*Math.PI);
}

qt.core.Line.prototype.getUnitVector = function() {
    return new this.constructor(this.p1.x, this.p1.y,
                   this.p1.x + this.dx/this.length,
                   this.p1.y + this.dy/this.length);
}

qt.core.Line.prototype.getNormalVector = function() {
    return new this.constructor(this.p1.x, this.p1.y,
                   this.p1.x + this.dy, this.p1.y - this.dx);
}

qt.core.Line.prototype.pointAt = function(t) {
    return new this.constructor(this.p1.x + this.dx * t,
                                this.p1.y + this.dy * t);
}

qt.core.Line.prototype.translate = function() {
    if (arguments.length == 2) {
        var dx = arguments[0];
        var dy = arguments[1];
        this.p1.translate(dx, dy);
        this.p2.translate(dx, dy);
    } else if (arguments.length == 1) {
        var p = arguments[0];
        this.p1.translate(p);
        this.p2.translate(p);
    } else {
        throw new Error("Line.prototype.translate: invalid number of arguments");
    }
    return this;
}

qt.core.Line.prototype.clone = function() {
    return new this.constructor(this.x1, this.y1, this.x2, this.y2);
}

qt.core.Line.prototype.toString = function() {
    return "Line(" + this.p1.x + ", " + this.p1.y + ", "
                   + this.p2.x + ", " + this.p2.y + ")";
}

////////////////////////////
//////// Rect class ////////
////////////////////////////

qt.core.Rect = function() {
    if (arguments.length == 4) {
        this.x = arguments[0] - 0;
        this.y = arguments[1] - 0;
        this.width = arguments[2] - 0;
        this.height = arguments[3] - 0;
    } else if (arguments.length == 2) {
        this.x = arguments[0].x - 0;
        this.y = arguments[0].y - 0;
        this.width = arguments[1].width - 0;
        this.height = arguments[1].height - 0;
    } else if (arguments.length == 0) {
        this.x = 0;
        this.y = 0;
        this.width = 0;
        this.height = 0;
    } else {
        throw new Error("Line: invalid number of arguments");
    }
}

qt.core.Rect.prototype = new qt.core.Rect();

qt.core.Rect.prototype.__defineGetter__(
    "isNull",
    function() {
        return (this.width == 0) && (this.height == 0);
    } );

qt.core.Rect.prototype.__defineGetter__(
    "isEmpty",
    function() {
        return (this.width <= 0) || (this.height <= 0);
    } );

qt.core.Rect.prototype.__defineGetter__(
    "isValid",
    function() {
        return (this.width > 0) && (this.height > 0);
    } );

qt.core.Rect.prototype.__defineGetter__(
    "left", function() { return this.x; } );

qt.core.Rect.prototype.__defineSetter__(
    "left",
    function(pos) {
        var diff = pos - this.x;
        this.x += diff;
        this.width -= diff;
    }
);

qt.core.Rect.prototype.__defineGetter__(
    "top", function() { return this.y; } );

qt.core.Rect.prototype.__defineSetter__(
    "top",
    function(pos) {
        var diff = pos - this.y;
        this.y += diff;
        this.height -= diff;
    }
);

qt.core.Rect.prototype.__defineGetter__(
    "right", function() { return this.x + this.width; } );

qt.core.Rect.prototype.__defineSetter__(
    "right", function(pos) { this.width = pos - this.x; } );

qt.core.Rect.prototype.__defineGetter__(
    "bottom", function() { return this.y + this.height; } );

qt.core.Rect.prototype.__defineSetter__(
    "bottom", function(pos) { this.height = pos - this.y; } );

qt.core.Rect.prototype.__defineGetter__(
    "center",
    function() {
        return new qt.core.Point(this.x + this.width/2,
                            this.y + this.height/2);
    } );

qt.core.Rect.prototype.__defineGetter__(
    "size",
    function() {
        return new qt.core.Size(this.width, this.height);
    } );

qt.core.Rect.prototype.__defineSetter__(
    "size",
    function(s) {
        this.width = s.width;
        this.height = s.height;
    } );

qt.core.Rect.prototype.adjust = function(dx1, dy1, dx2, dy2) {
    this.x += dx1;
    this.y += dy1;
    this.width += dx2 - dx1;
    this.height += dy2 - dy1;
    return this;
}

qt.core.Rect.prototype.adjusted = function(dx1, dy1, dx2, dy2) {
    return new this.constructor(this.x + dx1, this.y + dy1,
                                this.width + dx2 - dx1,
                                this.height + dy2 - dy1);
}

qt.core.Rect.prototype.normalized = function() {
    var r = this.clone();
    if (r.width < 0) {
        r.x = r.x + r.width;
        r.width = -r.width;
    }
    if (r.height < 0) {
        r.y = r.y + r.height;
        r.height = -r.height;
    }
    return r;
}

qt.core.Rect.prototype.contains = function(a) {
    if (this.isNull)
        return false;
    if (a instanceof qt.core.Point) {
        var r = this.normalized();
        return (a.x >= r.x) && (a.x <= r.x + r.width)
            && (a.y >= r.y) && (a.y <= r.y + r.height);
    } else {
        if (a.isNull)
            return false;
        var r1 = this.normalized();
        var r2 = a.normalized();
        return (r2.x >= r1.x) && (r2.x + r2.width <= r1.x + r1.width)
            && (r2.y >= r1.y) && (r2.y + r2.height <= r1.y + r1.height);
    }
}

qt.core.Rect.prototype.intersected = function(r) {
    if (this.isNull || r.isNull)
        return new this.constructor();
    var r1 = this.normalized();
    var r2 = r.normalized();
    var tmp = new this.constructor();
    tmp.x = Math.max(r1.x, r2.x);
    tmp.y = Math.max(r1.y, r2.y);
    tmp.width = Math.min(r1.x + r1.width, r2.x + r2.width) - tmp.x;
    tmp.height = Math.min(r1.y + r1.height, r2.y + r2.height) - tmp.y;
    return tmp.isEmpty ? new this.constructor() : tmp;
}

qt.core.Rect.prototype.intersects = function(r) {
    if (this.isNull || r.isNull)
        return false;
    var r1 = this.normalized();
    var r2 = r.normalized();
    return Math.max(r1.x, r2.x) < Math.min(r1.x + r1.width, r2.x + r2.width)
        && Math.max(r1.y, r2.y) < Math.min(r1.y + r1.height, r2.y + r2.height);
}

qt.core.Rect.prototype.translate = function() {
    var dx;
    var dy;
    if (arguments.length == 2) {
        dx = arguments[0];
        dy = arguments[1];
    } else if (arguments.length == 1) {
        dx = arguments[0].x;
        dy = arguments[0].y;
    } else {
        throw new Error("Rect.prototype.translate: invalid number of arguments");
    }
    this.x += dx;
    this.y += dy;
    return this;
}

qt.core.Rect.prototype.translated = function() {
    var dx;
    var dy;
    if (arguments.length == 2) {
        dx = arguments[0];
        dy = arguments[1];
    } else if (arguments.length == 1) {
        dx = arguments[0].x;
        dy = arguments[0].y;
    } else {
        throw new Error("Rect.prototype.translated: invalid number of arguments");
    }
    return new this.constructor(this.x + dx, this.y + dy,
                                this.width, this.height);
}

qt.core.Rect.prototype.united = function(r) {
    if (this.isNull)
        return r;
    if (r.isNull)
        return this.clone();
    var r1 = this.normalized();
    var r2 = r.normalized();
    var tmp = new this.constructor();
    tmp.x = Math.min(r1.x, r2.x);
    tmp.y = Math.min(r1.y, r2.y);
    tmp.width = Math.max(r1.x + r1.width, r2.x + r2.width) - tmp.x;
    tmp.height = Math.max(r1.y + r1.height, r2.y + r2.height) - tmp.y;
    return tmp;
}

qt.core.Rect.prototype.moveTo = function() {
    var x;
    var y;
    if (arguments.length == 2) {
        x = arguments[0];
        y = arguments[1];
    } else if (arguments.length == 1) {
        x = arguments[0].x;
        y = arguments[0].y;
    } else {
        throw new Error("Rect.prototype.moveTo: invalid number of arguments");
    }
    this.x = x;
    this.y = y;
    return this;
}

qt.core.Rect.prototype.clone = function() {
    return new this.constructor(this.x, this.y, this.width, this.height);
}

qt.core.Rect.prototype.toString = function() {
    return "Rect(" + this.x + ", " + this.y + ", "
                   + this.width + ", " + this.height + ")";
}

/****************************************************************************
**
** Implementation of QMap.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmap.h"

QMapData QMapData::shared_null =
{ Q_ATOMIC_INIT(1), 0,
  { &QMapData::shared_null.header, &QMapData::shared_null.header, 0, QMapData::Red, 1 }
};

static void rotateLeft( QMapData::Node * x, QMapData::Node *& root)
{
    register QMapData::Node * y = x->right;
    x->right = y->left;
    if (y->left !=0)
	y->left->parent = x;
    y->parent = x->parent;
    if (x == root)
	root = y;
    else if (x == x->parent->left)
	x->parent->left = y;
    else
	x->parent->right = y;
    y->left = x;
    x->parent = y;
}


static void rotateRight( QMapData::Node * x, QMapData::Node *& root )
{
    register QMapData::Node * y = x->left;
    x->left = y->right;
    if (y->right != 0)
	y->right->parent = x;
    y->parent = x->parent;
    if (x == root)
	root = y;
    else if (x == x->parent->right)
	x->parent->right = y;
    else
	x->parent->left = y;
    y->right = x;
    x->parent = y;
}


void QMapData::rebalance(Node * x)
{
    Node *&root = header.parent;
    x->color = Red;
    while ( x != root && x->parent->color == Red ) {
	if ( x->parent == x->parent->parent->left ) {
	    Node * y = x->parent->parent->right;
	    if (y && y->color == Red) {
		x->parent->color = Black;
		y->color = Black;
		x->parent->parent->color = Red;
		x = x->parent->parent;
	    } else {
		if (x == x->parent->right) {
		    x = x->parent;
		    rotateLeft( x, root );
		}
		x->parent->color = Black;
		x->parent->parent->color = Red;
		rotateRight (x->parent->parent, root );
	    }
	} else {
	    Node * y = x->parent->parent->left;
	    if ( y && y->color == Red ) {
		x->parent->color = Black;
		y->color = Black;
		x->parent->parent->color = Red;
		x = x->parent->parent;
	    } else {
		if (x == x->parent->left) {
		    x = x->parent;
		    rotateRight( x, root );
		}
		x->parent->color = Black;
		x->parent->parent->color = Red;
		rotateLeft( x->parent->parent, root );
	    }
	}
    }
    root->color = Black;
}


QMapData::Node * QMapData::removeAndRebalance(Node * z)
{
    Node *& root = header.parent;
    Node *& leftmost = header.left;
    Node *& rightmost = header.right;

    Node * y = z;
    Node * x;
    Node * x_parent;
    if (y->left == 0) {
	x = y->right;
    } else {
	if (y->right == 0)
	    x = y->left;
	else
	    {
		y = y->right;
		while (y->left != 0)
		    y = y->left;
		x = y->right;
	    }
    }
    if (y != z) {
	z->left->parent = y;
	y->left = z->left;
	if (y != z->right) {
	    x_parent = y->parent;
	    if (x)
		x->parent = y->parent;
	    y->parent->left = x;
	    y->right = z->right;
	    z->right->parent = y;
	} else {
	    x_parent = y;
	}
	if (root == z)
	    root = y;
	else if (z->parent->left == z)
	    z->parent->left = y;
	else
	    z->parent->right = y;
	y->parent = z->parent;
	// Swap the colors
	uint c = y->color;
	y->color = z->color;
	z->color = c;
	y = z;
    } else {
	x_parent = y->parent;
	if (x)
	    x->parent = y->parent;
	if (root == z)
	    root = x;
	else if (z->parent->left == z)
	    z->parent->left = x;
	else
	    z->parent->right = x;
	if ( leftmost == z ) {
	    if (z->right == 0)
		leftmost = z->parent;
	    else
		leftmost = minimum(x);
	}
	if (rightmost == z) {
	    if (z->left == 0)
		rightmost = z->parent;
	    else
		rightmost = maximum(x);
	}
    }
    if (y->color != Red) {
	while (x != root && (x == 0 || x->color == Black)) {
	    if (x == x_parent->left) {
		Node * w = x_parent->right;
		if (w->color == Red) {
		    w->color = Black;
		    x_parent->color = Red;
		    rotateLeft(x_parent, root);
		    w = x_parent->right;
		}
		if ((w->left == 0 || w->left->color == Black) &&
		    (w->right == 0 || w->right->color == Black)) {
		    w->color = Red;
		    x = x_parent;
		    x_parent = x_parent->parent;
		} else {
		    if (w->right == 0 || w->right->color == Black) {
			if (w->left)
			    w->left->color = Black;
			w->color = Red;
			rotateRight(w, root);
			w = x_parent->right;
		    }
		    w->color = x_parent->color;
		    x_parent->color = Black;
		    if (w->right)
			w->right->color = Black;
		    rotateLeft(x_parent, root);
		    break;
		}
	    } else {
		Node * w = x_parent->left;
		if (w->color == Red) {
		    w->color = Black;
		    x_parent->color = Red;
		    rotateRight(x_parent, root);
		    w = x_parent->left;
		}
		if ((w->right == 0 || w->right->color == Black) &&
		    (w->left == 0 || w->left->color == Black)) {
		    w->color = Red;
		    x = x_parent;
		    x_parent = x_parent->parent;
		} else {
		    if (w->left == 0 || w->left->color == Black) {
			if (w->right)
			    w->right->color = Black;
			w->color = Red;
			rotateLeft(w, root);
			w = x_parent->left;
		    }
		    w->color = x_parent->color;
		    x_parent->color = Black;
		    if (w->left)
			w->left->color = Black;
		    rotateRight(x_parent, root);
		    break;
		}
	    }
	}
	if (x)
	    x->color = Black;
    }
    return y;
}

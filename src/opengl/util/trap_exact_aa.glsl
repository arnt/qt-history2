float quad_aa()
{
    float top = min(gl_FragCoord.y + 0.5, gl_TexCoord[0].x);
    float bottom = max(gl_FragCoord.y - 0.5, gl_TexCoord[0].y);

    float topExcluded = (gl_FragCoord.y + 0.5) - top;
    float bottomExcluded = bottom - (gl_FragCoord.y - 0.5);

    float left = gl_FragCoord.x - 0.5;
    float right = gl_FragCoord.x + 0.5;

    vec2 leftLineEq = gl_TexCoord[1].xy;
    vec2 rightLineEq = gl_TexCoord[1].zw;

    float topLeftX = leftLineEq.x * top + leftLineEq.y;
    float topRightX = rightLineEq.x * top + rightLineEq.y;

    float bottomLeftX = leftLineEq.x * bottom + leftLineEq.y;
    float bottomRightX = rightLineEq.x * bottom + rightLineEq.y;

    if (topLeftX < bottomLeftX) {
        float temp = topLeftX;
        topLeftX = bottomLeftX;
        bottomLeftX = temp;
    }

    if (topRightX < bottomRightX) {
        float temp = topRightX;
        topRightX = bottomRightX;
        bottomRightX = temp;
    }

    float area = 1 - topExcluded - bottomExcluded;

    float leftExcluded = 0;

    if (topLeftX > left) {
        float leftIntersectY = bottom + (top - bottom) * (left - bottomLeftX) / (topLeftX - bottomLeftX);
        float rightIntersectY = bottom + (top - bottom) * (right - bottomLeftX) / (topLeftX - bottomLeftX);

        if (bottomLeftX > right) { // right < bottom < top
            leftExcluded = 1;
        } else if (bottomLeftX > left) {
            if (topLeftX > right) { // left < bottom < right < top
                leftExcluded = area - 0.5 * (right - bottomLeftX) * (rightIntersectY - bottom);
            } else { // left < bottom < top < right
                leftExcluded = (bottomLeftX - left + 0.5 * (topLeftX - bottomLeftX)) * (top - bottom);
            }
        } else if (topLeftX > right) { // bottom < left < right < top
            leftExcluded = (top - rightIntersectY + 0.5 * (rightIntersectY - leftIntersectY)) * (right - left);
        } else if (topLeftX > left) { // bottom < left < top < right
            leftExcluded = 0.5 * (top - leftIntersectY) * (topLeftX - left);
        }
    }

    float rightExcluded = 0;

    if (bottomRightX < right) {
        float leftIntersectY = bottom + (top - bottom) * (left - bottomRightX) / (topRightX - bottomRightX);
        float rightIntersectY = bottom + (top - bottom) * (right - bottomRightX) / (topRightX - bottomRightX);;

        if (topRightX < left) { // bottom < top < left
            rightExcluded = 1;
        } else if (topRightX < right) {
            if (bottomRightX < left) { // bottom < left < top < right
                rightExcluded = area - 0.5 * (topRightX - left) * (top - leftIntersectY);
            } else { // left < bottom < top < right
                rightExcluded = (right - topRightX + 0.5 * (topRightX - bottomRightX)) * (top - bottom);
            }
        } else if (bottomRightX < left) { // bottom < left < right < top
            rightExcluded = (leftIntersectY - bottom + 0.5 * (rightIntersectY - leftIntersectY)) * (right - left);
        } else { // left < bottom < right < top
            rightExcluded = 0.5 * (rightIntersectY - bottom) * (right - bottomRightX);
        }
    }

    return area - leftExcluded - rightExcluded;
}

void main()
{
    gl_FragColor = quad_aa();
}


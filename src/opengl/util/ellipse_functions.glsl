float ellipse()
{
    vec2 st = gl_TexCoord[0].st;

    if (dot(st, st) > 1)
        discard;

    return 1.0;
}

// ellipse equation

// x^2/a^2 + y^2/b^2 = 1
//
// f(x,y) = x^2/rx + y^2/ry

// df/dx = 2x/rx
// df/dy = 2y/ry

// f(x,y) = 1 + e ~= 1 + sqrt((df/dx)^2 + (df/dy)^2)*dist

float ellipse_aa()
{
    vec2 st = gl_TexCoord[0].st;
    vec2 r = gl_TexCoord[0].pq;

    vec2 n = st/r;
	
    float eps = 1. - dot(n, n);
    vec2 r2 = r*r;
    vec2 grad = 2.*st/r2;
    float g = inversesqrt(dot(grad, grad));

    return smoothstep(-0.55, 0.55, eps * g);
}

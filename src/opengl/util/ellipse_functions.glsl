void ellipse()
{
    vec2 st = gl_TexCoord[0].st;

    discard (dot(st, st) > 1);
}

float ellipse_aa()
{
    vec2 st = gl_TexCoord[0].st;

    float Fxy = 1 - dot(st, st);
    vec2 gradFxy = vec2(ddx(Fxy), ddy(Fxy));
    float g = inversesqrt(dot(gradFxy, gradFxy));

    return smoothstep(-1.0, 1.0, Fxy * g);
}

void ellipse()
{
    vec2 st = gl_TexCoord[0].st;

    discard (dot(st, st) > 1);
}

uniform vec4 solid_color;

void main()
{
    ellipse();

    gl_FragColor = solid_color;
}

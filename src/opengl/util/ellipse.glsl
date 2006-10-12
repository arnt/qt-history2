uniform vec4 solid_color;

void main()
{
    vec2 st = gl_TexCoord[0].st;
    vec2 r = gl_TexCoord[0].pq;

    vec2 n = st/r;
    float eps = dot(n, n);
    if (eps < 1.)
	gl_FragColor = solid_color;
    else 
	discard;
}

#include "ellipse_functions.glsl"

uniform vec4 solid_color;

void main()
{
    gl_FragColor = solid_color * ellipse_aa();
}

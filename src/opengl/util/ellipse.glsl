#include "ellipse_functions.glsl"

uniform vec4 solid_color;

void main()
{
    ellipse();

    gl_FragColor = solid_color;
}

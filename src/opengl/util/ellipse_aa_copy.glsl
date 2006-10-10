#include "ellipse_functions.glsl"

uniform sampler2D texture;
uniform vec2 inv_texture_size;

void main()
{
    gl_FragColor = ellipse_aa() * texture2D(texture, gl_FragCoord.xy * inv_texture_size);
}

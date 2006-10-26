// fast painter for composition modes which can be implemented with blendfuncs

uniform sampler2D mask_texture;
uniform vec2 inv_buffer_size;

float mask(vec2 tex_coords)
{
    vec4 mask = texture2D(mask_texture, tex_coords);

    return mask.r;
}

void main()
{
    vec2 tex_coords = gl_FragCoord.xy * inv_buffer_size;

    // combine clip and coverage channels
    float mask_alpha = mask(tex_coords);

    //if (mask_alpha < 0.00000000001)
    //    discard;

    gl_FragColor = brush() * mask_alpha;
}

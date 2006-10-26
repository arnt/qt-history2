uniform sampler2D dst_texture;
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

    vec4 dst = texture2D(dst_texture, tex_coords);

    // combine clip and coverage channels
    float mask_alpha = mask(tex_coords);

    //if (mask_alpha < 0.00000000001)
    //    discard;

    gl_FragColor = mix(dst, composite(brush(), dst), mask_alpha);
}

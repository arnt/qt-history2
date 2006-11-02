uniform sampler2D brush_texture;
uniform vec2 inv_brush_texture_size;
uniform vec4 inv_matrix;
uniform vec2 inv_matrix_offset;

vec4 brush()
{
    mat2 mat;

    mat[0] = inv_matrix.xy;
    mat[1] = inv_matrix.zw;

    vec2 coords = gl_FragCoord.xy * mat + inv_matrix_offset;

    coords *= inv_brush_texture_size;

    coords.y = -coords.y;

    return texture2D(brush_texture, coords);
}

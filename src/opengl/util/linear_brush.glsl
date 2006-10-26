uniform sampler1D palette;
uniform vec4 inv_matrix;
uniform vec2 inv_matrix_offset;
uniform vec3 linear;

vec4 brush()
{
    mat2 mat;

    mat[0] = inv_matrix.xy;
    mat[1] = inv_matrix.zw;

    vec2 A = gl_FragCoord.xy * mat + inv_matrix_offset;

    float val = dot(linear.xy, A) * linear.z;

    return texture1D(palette, val);
}


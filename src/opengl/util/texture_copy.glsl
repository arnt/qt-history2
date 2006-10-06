uniform sampler2D texture;
uniform vec2 texture_size;

void main()
{
    gl_FragColor = texture2D(texture, gl_FragCoord.xy / texture_size);
}

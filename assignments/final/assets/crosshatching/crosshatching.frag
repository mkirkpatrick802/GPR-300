#version 450

uniform sampler2D screen_texture;

in vec2 uv;

out vec4 fragment_color;

void main()
{
    fragment_color = texture(screen_texture, uv).rgba;
}
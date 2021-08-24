#version 450 core

in vec2 TextureCoordinate;
in vec4 TransformedPosition;

out vec4 gl_FragColor;

layout (binding = 0) uniform usampler2D VolumeSliceTexture;

void main()
{
    uvec4 TextureColour = texture(VolumeSliceTexture, TextureCoordinate);

    gl_FragColor = vec4(TextureColour.r) / vec4(255.0);
}
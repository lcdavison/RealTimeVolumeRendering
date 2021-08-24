#version 450 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 InTextureCoordinate;

uniform mat4x4 ProjectionMatrix;
uniform mat4x4 ViewMatrix;

uniform mat4x4 Transformation;

out vec2 TextureCoordinate;
out vec4 TransformedPosition;

void main()
{
    TransformedPosition = ProjectionMatrix * ViewMatrix * vec4(VertexPosition, 1.0);
    gl_Position = TransformedPosition;

    TextureCoordinate = InTextureCoordinate;
}
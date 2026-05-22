#version 120

varying vec4 color0;

attribute vec3 position;
attribute vec4 color;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(position, 1.0);
    color0 = color;
}

#version 330
// FilterManager full-screen quad (quad vertices span -1..1 in x/z).
// texpad_tex = (0.5*w/padded_w, 0.5*h/padded_h) compensates for any
// power-of-two padding of the offscreen texture; pass it from Python.
uniform mat4 p3d_ModelViewProjectionMatrix;
uniform vec4 texpad_tex;
in vec4 p3d_Vertex;
out vec2 texcoord;
void main() {
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
  texcoord = p3d_Vertex.xz * texpad_tex.xy + texpad_tex.xy;
}

#version 330
uniform mat4 p3d_ModelViewProjectionMatrix;
uniform mat4 p3d_ModelViewMatrix;
uniform mat3 p3d_NormalMatrix;
in vec4 p3d_Vertex;
in vec3 p3d_Normal;
in vec2 p3d_MultiTexCoord0;
in vec4 p3d_Color;
out vec2 v_uv;
out vec3 v_n;
out vec3 v_view;
out vec4 v_color;
void main() {
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
  vec4 vp = p3d_ModelViewMatrix * p3d_Vertex;
  v_view = -vp.xyz;
  v_n = normalize(p3d_NormalMatrix * p3d_Normal);
  v_uv = p3d_MultiTexCoord0;
  v_color = p3d_Color;
}

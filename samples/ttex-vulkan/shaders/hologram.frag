#version 330
// TTEX "virtual cog" hologram: fresnel rim glow on top of the normal texture.
// Opt-in: np.setShader(Shader.load(Shader.SL_GLSL, "hologram.vert", "hologram.frag"))
// Inputs: rim_color (rgb + strength in alpha), rim_power (falloff, 2-3 looks good).
// The look seen in TTEX QA used rim_color (0.2, 0.8, 1.0, 1.0), rim_power 2.5.
uniform sampler2D p3d_Texture0;
uniform vec4 p3d_ColorScale;
uniform vec4 rim_color;     // set from Python: np.setShaderInput("rim_color", ...)
uniform float rim_power;
in vec2 v_uv;
in vec3 v_n;
in vec3 v_view;
in vec4 v_color;
out vec4 o_color;
void main() {
  vec4 base = texture(p3d_Texture0, v_uv) * v_color * p3d_ColorScale;
  float rim = pow(1.0 - clamp(dot(normalize(v_n), normalize(v_view)), 0.0, 1.0), rim_power);
  o_color = vec4(base.rgb + rim_color.rgb * rim * rim_color.a, base.a);
}

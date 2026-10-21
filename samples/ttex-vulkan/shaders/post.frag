#version 330
// TTEX demo post-process: vignette + slight saturation boost (FilterManager quad).
uniform sampler2D tex;
in vec2 texcoord;
out vec4 o_color;
void main() {
  vec4 c = texture(tex, texcoord);
  float l = dot(c.rgb, vec3(0.299, 0.587, 0.114));
  c.rgb = mix(vec3(l), c.rgb, 1.25);
  float d = distance(texcoord, vec2(0.5));
  c.rgb *= smoothstep(0.85, 0.35, d);
  o_color = vec4(c.rgb, 1.0);
}

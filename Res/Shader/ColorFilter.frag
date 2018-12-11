#version 410

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

// ポストエフェクトデータ.
layout(std140) uniform PostEffectData
{
  mat4x4 matColor; // 色変換行列.

} postEffect;

void main()
{
  fragColor = texture(colorSampler, inTexCoord);
  //fragColor.rgb = vec3(dot(vec3(0.299, 0.587, 0.114), fragColor.rgb));	// モノトーン変換
  fragColor.rgb = (postEffect.matColor * vec4(fragColor.rgb, 1)).rgb;
  //fragColor.rgb = vec3(1 - fragColor.r,1 - fragColor.g,1 - fragColor.b);	// ネガポジ変換
}

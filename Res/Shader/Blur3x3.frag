#version 410

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

void main()
{
#if 1
  // 1/4�k���o�b�t�@
  vec4 ts;
  ts.xy = vec2(0.25) / vec2(textureSize(colorSampler, 0));
  ts.zw = -ts.xy;
  fragColor = texture(colorSampler, inTexCoord + ts.xy);
  fragColor += texture(colorSampler, inTexCoord + ts.zy);
  fragColor += texture(colorSampler, inTexCoord + ts.xw);
  fragColor += texture(colorSampler, inTexCoord + ts.zw);
  fragColor.rgb *= 1.0 / 4.0;
#else
  // 1/2�k���o�b�t�@
  fragColor = texture(colorSampler, inTexCoord);
#endif
  fragColor.a = 1.0;
}

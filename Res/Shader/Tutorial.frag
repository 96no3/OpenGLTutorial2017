#version 410

// in変数.
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inWorldPosition;
//layout(location=3) in vec3 inWorldNormal;
layout(location=3) in mat3 inTBN;

// out変数.
out vec4 fragColor;

// uniform変数.
//uniform sampler2D colorSampler;
uniform sampler2D colorSampler[2];

const int maxLightCount = 4; // ライトの数.

// ライトデータ(点光源).
struct PointLight
{
  vec4 position; //座標(ワールド座標系).
  vec4 color; // 明るさ.
};

// ライティングパラメータ.
layout(std140) uniform LightData
{
  vec4 ambientColor; // 環境光.
  PointLight light[maxLightCount]; // ライトのリスト.
} lightData;


void main()
{
  vec3 normal = texture(colorSampler[1], inTexCoord).xyz * 2.0 - 1.0;
  normal = inTBN * normal;

  //fragColor = inColor * texture(colorSampler, inTexCoord);
  fragColor = inColor * texture(colorSampler[0], inTexCoord);
  vec3 lightColor = lightData.ambientColor.rgb;
  for (int i = 0; i < maxLightCount; ++i) {
    vec3 lightVector = lightData.light[i].position.xyz - inWorldPosition;
    float lightPower = 1.0 / (1 + dot(lightVector, lightVector));
	//float cosTheta = clamp(dot(inWorldNormal, normalize(lightVector)), 0, 1);
	float cosTheta = clamp(dot(normal, normalize(lightVector)), 0, 1);
    lightColor += lightData.light[i].color.rgb * cosTheta * lightPower;
  }

  fragColor.rgb *= lightColor;

//  const float numShades = 3; // 影の段数.
//  fragColor.rgb = ceil(fragColor.rgb * numShades) * (1 / numShades); 

}
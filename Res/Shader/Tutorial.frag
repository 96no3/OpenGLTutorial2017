#version 410

// in�ϐ�.
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inWorldPosition;
//layout(location=3) in vec3 inWorldNormal;
layout(location=3) in mat3 inTBN;
layout(location=6) in vec3 inDepthCoord;

// out�ϐ�.
out vec4 fragColor;

// uniform�ϐ�.
uniform int viewIndex;
//uniform sampler2D colorSampler;
uniform sampler2D colorSampler[2];
uniform sampler2DShadow depthSampler;

const int maxLightCount = 4; // ���C�g�̐�.

// ���C�g�f�[�^(�_����).
struct PointLight
{
  vec4 position; //���W(���[���h���W�n).
  vec4 color; // ���邳.
};

// ���C�e�B���O�p�����[�^.
layout(std140) uniform LightData
{
  vec4 eyePos[4];
  vec4 ambientColor; // ����.
  PointLight light[maxLightCount]; // ���C�g�̃��X�g.
} lightData;

const float shininess = 2;
const float normFactor = (shininess + 2) * (1.0 / (2.0 * 3.1415926));

// �\�t�g�V���h�E
const float softShadowScale = 1.0 / 1600.0;
const vec3 poissonDisk[4] = vec3[](
  vec3( -0.94201624, -0.39906216, 0 ) * softShadowScale,
  vec3( 0.94558609, -0.76890725, 0 ) * softShadowScale,
  vec3( -0.094184101, -0.92938870, 0 ) * softShadowScale,
  vec3( 0.34495938, 0.29387760, 0 ) * softShadowScale
);

// �e�䗦���擾����.
float ShadowRatio(float bias)
{
  vec3 coord = inDepthCoord;
  coord.z -= bias;
  float shadow = 0.0;
  for (int i = 0; i < 4; ++i) {
    shadow += texture(depthSampler, coord + poissonDisk[i]);
  }
  return shadow * (1.0 / 4.0) * 0.5 + 0.5;
}


void main()
{
  vec3 normal = texture(colorSampler[1], inTexCoord).xyz * 2.0 - 1.0;
  normal = inTBN * normal;

  //fragColor = inColor * texture(colorSampler, inTexCoord);
  fragColor = inColor * texture(colorSampler[0], inTexCoord);
  //vec3 lightColor = lightData.ambientColor.rgb;
  vec3 lightColor = vec3(0);
  vec3 specularColor = vec3(0);

  for (int i = 0; i < maxLightCount; ++i) {
    vec3 lightVector = lightData.light[i].position.xyz - inWorldPosition;
    float lightPower = 1.0 / (1 + dot(lightVector, lightVector));
	vec3 normalizedLightVector = normalize(lightVector);
	//float cosTheta = clamp(dot(inWorldNormal, normalize(lightVector)), 0, 1);
	//float cosTheta = clamp(dot(normal, normalize(lightVector)), 0, 1);
	float cosTheta = clamp(dot(normal, normalizedLightVector), 0, 1);
    lightColor += lightData.light[i].color.rgb * cosTheta * lightPower;
	vec3 eyeVector = normalize(lightData.eyePos[viewIndex].xyz - lightData.light[i].position.xyz);
    specularColor += lightData.light[i].color.rgb * pow(max(dot(eyeVector, reflect(normalizedLightVector, normal)), 0), shininess) * lightPower * 0.25;
  }

//  fragColor.rgb *= lightColor;
//  fragColor.rgb += specularColor * normFactor;
  float cosTheta = clamp(dot(normal, normalize(lightData.light[0].position.xyz - inWorldPosition)), 0, 1);
  float depthBias = 0.005 * tan(acos(cosTheta));
  depthBias = clamp(depthBias, 0, 0.01);
  //float shadow = texture(depthSampler, inDepthCoord + vec3(0, 0, -depthBias)) * 0.5 + 0.5;
  float shadow = ShadowRatio(depthBias);
  fragColor.rgb *= lightColor * shadow + lightData.ambientColor.rgb;
  fragColor.rgb += specularColor * normFactor * shadow;

//  const float numShades = 3; // �e�̒i��.
//  fragColor.rgb = ceil(fragColor.rgb * numShades) * (1 / numShades);
}
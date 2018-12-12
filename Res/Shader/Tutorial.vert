#version 410

// in�ϐ�.
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec4 vTangent;

// out�ϐ�.
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outWorldPosition;
//layout(location=3) out vec3 outWorldNormal;
layout(location=3) out mat3 outTBN;

/**
* ���_�V�F�[�_�̃p�����[�^.
*/
layout(std140) uniform VertexData
{
  mat4 matMVP;
  mat4 matModel;
  mat3x4 matNormal;
  vec4 color;

} vertexData;


void main()
{
//  outColor = vColor;
//  outTexCoord = vTexCoord;
//  outWorldPosition = vPosition;
//  outWorldNormal = vNormal;

	outColor = vertexData.color * vColor;
	outTexCoord = vTexCoord;
	outWorldPosition = (vertexData.matModel * vec4(vPosition, 1.0)).xyz;
	//outWorldNormal = mat3(vertexData.matNormal) * vNormal;
	mat3 matNormal = mat3(vertexData.matNormal);
	vec3 t = matNormal * vTangent.xyz;
	vec3 n = matNormal * vNormal;
	vec3 b = normalize(cross(n, t)) * vTangent.w;
	outTBN = mat3(t, b, n);

  gl_Position = vertexData.matMVP * vec4(vPosition, 1);	
}
/**
* @file standard.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition;		 // ���[���h���W
layout(location=1) in vec2 inTexcoord;		 // �e�N�X�`�����W
layout(location=2) in mat3 inTBN;			 // �@���ϊ��s��
layout(location=5) in vec3 inShadowTexcoord; // �V���h�E�e�N�X�`�����W

// �e�N�X�`���T���v��
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texEmission;
layout(binding=2) uniform sampler2DShadow texShadow; // �e�p�̐[�x�e�N�X�`��
layout(binding=3) uniform sampler2D texNormal; // �@���e�N�X�`��
layout(binding=4) uniform samplerCube texEnvironment; // ���L���[�u�}�b�v

layout(location=4) uniform vec3 cameraPosition; // �J�������W

// �v���O��������̓���
layout(location=100) uniform vec4 color;		// ���̂̐F
layout(location=101) uniform vec4 emission;		// ���̂̔����F
layout(location=102) uniform float alphaCutoff; // �t���O�����g��j�����鋫�E�l

// x = ���̕\�ʂ̑e���Ay = �������ǂ���(0 = ������A1 = ����)
layout(location=103) uniform vec2 roughnessAndMetallic;

// ����
layout(location=107) uniform vec3 ambientLight;

// ���s����
struct DirectionalLight
{
	vec3 color;		// �F�Ɩ��邳
	vec3 direction; // ���̌���
};
layout(location=108) uniform DirectionalLight directionalLight;

// �o�͂���F�f�[�^
out vec4 outColor;

// ���C�g
struct Light
{
	vec4 colorAndFalloffAngle[16];	// �F�Ɩ��邳�A�����J�n�p�x
	vec4 positionAndRadius[16];		// ���W�Ɣ��a
	vec4 directionAndConeAngle[16]; // �����A�ő�Ǝˊp�x
};
layout(location=110) uniform int lightCount;
layout(location=111) uniform Light pointLight;

// �@�����z��
float NormalDistributionGGX(vec3 N, vec3 H, float roughness)
{
	// ���������������ɔ��˂���������ʂ̔䗦�����߂�
	float NdotH = max(dot(N, H), 0);
	float NdotH2 = NdotH * NdotH;
	
	// dot(N, H)���e������䗦�����߂�
	float r2 = roughness * roughness;
	float r4 = r2 * r2;
	float denom = (NdotH2 * (r4 - 1.0) + 1.0);
	denom = 3.14159265 * denom * denom;
	
	return r4 / denom;
}

// �􉽌�����
float GeometricAttenuationSchlick(float NdotL, float NdotV, float roughness)
{
	float k = (roughness + 1) * (roughness + 1) * 0.125;
	
	// �����������猩���􉽊w�I���������v�Z����
	float g0 = NdotL / (NdotL * (1 - k) + k);
	
	// ���_�������猩���􉽊w�I���������v�Z����
	float g1 = NdotV / (NdotV * (1 - k) + k);
	
	return g0 * g1;
}

// �t���l����
vec3 FresnelSchlick(vec3 f0, float VdotH)
{
	// �V�����b�N�̋ߎ���
	float vh1 = 1 - VdotH;
	float vh2 = vh1 * vh1;
	return f0 + (1 - f0) * (vh2 * vh2 * vh1);
}

// cos�Ƃ̍ŏ��l
const float minCosTheta = 0.000001;

// ���ʔ���BRDF
vec3 SpecularBRDF(
  vec3 normal, vec3 H, float roughness, vec3 cameraVector, float NdotL, vec3 F)
{
  // �@�����z�����v�Z����
  float D = NormalDistributionGGX(normal, H, roughness);

  // �􉽊w�I���������v�Z����
  float NdotV = max(dot(normal, cameraVector), minCosTheta);
  float G = GeometricAttenuationSchlick(NdotL, NdotV, roughness);

  // ���ʔ���BRDF���v�Z����
  float denom = 4 * NdotL * NdotV;
  return (D * G * F) * (1 / denom);
}

// CalcBRDF�̌v�Z����
struct BRDFResult
{
  vec3 diffuse;  // �g�U����
  vec3 specular; // ���ʔ���
};

// ���ʔ���BRDF�Ɗg�U����BRDF���܂Ƃ߂Čv�Z����
BRDFResult CalcBRDF(vec3 normal, vec3 f0,
  vec3 cameraVector, vec3 direction, vec3 color)
{
	// �t���l�����v�Z����
	vec3 H = normalize(direction + cameraVector);
	vec3 F = FresnelSchlick(f0, max(dot(cameraVector, H), 0));

	// GGX�ŋ��ʔ��˂��v�Z����
	float NdotL = max(dot(normal, direction), minCosTheta);
	vec3 specularBRDF = SpecularBRDF(
	  normal, H, roughnessAndMetallic.x, cameraVector, NdotL, F);
	vec3 specular = color * specularBRDF * NdotL;

	// ���K�������o�[�g�Ŋg�U���˂��v�Z����
	float diffuseBRDF = NdotL / 3.14159265;
	vec3 diffuse = color * diffuseBRDF * (1 - F) * (1 - roughnessAndMetallic.y);

	return BRDFResult(diffuse, specular);
}

// �G���g���[�|�C���g
void main()
{
	vec4 c = texture(texColor, inTexcoord);

	// �J�b�g�I�t�l���w�肳��Ă���ꍇ�A�A���t�@�����̒l�����̏ꍇ�͕`����L�����Z������
    if (alphaCutoff > 0)
	{
		if (c.a < alphaCutoff)
			// �t���O�����g��j��(�L�����Z��)����
			discard;

		// �j������Ȃ�������s�����ɂ���
		c.a = 1;
	}

	// �e�N�X�`���̃K���}�␳����������
	const float crtGamma = 2.2; // CRT���j�^�[�̃K���}�l
	c.rgb = pow(c.rgb, vec3(crtGamma));

	outColor = c * color;

	// �@���̒�����0�łȂ���΁A�@���e�N�X�`�����ݒ肳��Ă���Ɣ��f����
	vec3 normal = texture(texNormal, inTexcoord).xyz;
	if (dot(normal, normal) > 0.0001)
	{
	  // 8bit�l�ł��邱�Ƃ��l������0�`1��-1�`+1�ɕϊ�
	  // ���̎����g����8bit��128�����傤��0�ɂȂ�
	  normal = normal * (255.0 / 127.0) - (128.0 / 127.0);

	  // XY��������Z�����𕜌�(�@���̒����͏��1�Ȃ���)
	  normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);

	  // �^���W�F���g���W�n���烏�[���h���W�n�ɕϊ�
	  // ���`��Ԃɂ����outTBN�̒�����1�ł͂Ȃ��Ȃ��Ă���̂ŁA���K�����Ē�����1�ɕ�������
	  normal = normalize(inTBN * normal);
	}
	else
	  // �@���e�N�X�`�������ݒ�̏ꍇ�͒��_�@�����g��
	  normal = normalize(inTBN[2]);

	 // �����x�N�g��
	vec3 cameraVector = normalize(cameraPosition - inPosition);

	// �p�x0�̃t���l���l
	vec3 f0 = mix(vec3(0.04), outColor.rgb, roughnessAndMetallic.y);

	vec3 diffuse=vec3(0); // �g�U���̖��邳�̍��v
	vec3 specular = vec3(0);// ���ʔ��ˌ��̖��邳�̍��v
	for(int i = 0; i < lightCount; ++i)
	{
		// �|�C���g���C�g�̕����𑪂�
		vec3 direction = pointLight.positionAndRadius[i].xyz - inPosition;

		// �����܂ł̋����𑪂�
		float sqrDistance = dot(direction,direction);
		float distance = sqrt(sqrDistance);

		// �����𐳋K�����Ē�����1�ɂ���
		direction = normalize(direction);

	    float illuminance = 1;

		// �Ǝˊp�x��0���傫����΃X�|�b�g���C�g�Ƃ݂Ȃ�
		const float coneAngle = pointLight.directionAndConeAngle[i].w;
		if(coneAngle > 0)
		{
			// �u���C�g����t���O�����g�֌������x�N�g���v�Ɓu�X�|�b�g���C�g�̃x�N�g���v�̂Ȃ��p���v�Z����
			// �p�x��coneAngle�ȏ�Ȃ�͈͊O
			float angle = acos(dot(-direction, pointLight.directionAndConeAngle[i].xyz));
			if(angle >= coneAngle)
				continue; // �Ǝ˔͈͊O

			// �ő�Ǝˊp�x�̂Ƃ�0,�����p�x�̎�1�ɂȂ�悤�ɕ�Ԃ���
			const float falloffAngle = pointLight.colorAndFalloffAngle[i].w;
			const float a = min((coneAngle - angle) / (coneAngle - falloffAngle), 1);
			illuminance *= a;
		}

		// ���C�g�̍ő勗���𐧌�����
		const float radius = pointLight.positionAndRadius[i].w;
		const float smoothFactor = clamp(1 - pow(distance / radius, 4), 0, 1);
		illuminance *= smoothFactor * smoothFactor;

		// �t2��̖@���ɂ���Ė��邳������������
		// 0���Z���N���Ȃ��悤��1�𑫂��Ă���
		illuminance /= sqrDistance + 1;

		// �g�U���˂Ƌ��ʔ��˂��v�Z����
		vec3 color = pointLight.colorAndFalloffAngle[i].xyz * illuminance;
		BRDFResult result = CalcBRDF(normal, f0, cameraVector, direction, color);
		diffuse += result.diffuse;
		specular += result.specular;
	} // for lightCount

	// �e���v�Z����
	float shadow = texture(texShadow, inShadowTexcoord).r;

	// ���s�����̖��邳���v�Z����
	BRDFResult result = CalcBRDF(normal, f0, cameraVector,
	  -directionalLight.direction, directionalLight.color);
	specular += result.specular * shadow;
	diffuse += result.diffuse * shadow;

    // �A���r�G���g���C�g�̖��邳���v�Z����
	vec3 Fa = f0 + (1 - f0) * 0.0021555; // �p�x45���̃t���l���l
	specular += ambientLight * Fa;
	diffuse += ambientLight * (1 - Fa) * (1 - roughnessAndMetallic.y);

	// ���L���[�u�}�b�v�ɂ�郉�C�e�B���O
	{
		// �J�����x�N�g���Ɩ@�����甽�˃x�N�g�����v�Z����
		vec3 d = dot(cameraVector, normal) * normal;
		vec3 reflectionVector = 2 * d - cameraVector;
	
		// �X�y�L�������˗����v�Z����
		float NdotV = max(dot(normal, cameraVector), 0.01);
		float G = GeometricAttenuationSchlick(NdotV, NdotV, roughnessAndMetallic.x);
		vec3 F = FresnelSchlick(f0, max(dot(cameraVector, normal), 0));

		// �����p�x�̔��˂������Ȃ肷���Ȃ��悤�ɐ�������l
		const float angleThreshold = cos(radians(60));
		float denom = mix(angleThreshold, 1, NdotV);
		vec3 specularRatio = F * (G / denom);

		// �L���[�u�}�b�v���狾�ʔ��ː������擾����
		float maxMipmapLevel = textureQueryLevels(texEnvironment) - 1;
		float levelCap = textureQueryLod(texEnvironment, reflectionVector).x;
		float level = max(maxMipmapLevel * pow(roughnessAndMetallic.x, 0.5), levelCap);
		vec3 ambientSpecular = textureLod(texEnvironment, reflectionVector, level).rgb;
		ambientSpecular = pow(ambientSpecular, vec3(2.2)); // �K���}�␳������
		specular += ambientSpecular * specularRatio;

		// �L���[�u�}�b�v����g�U���ː������擾
		vec3 ambientDiffuse = textureLod(texEnvironment, normal, maxMipmapLevel).rgb;
		ambientDiffuse = pow(ambientDiffuse, vec3(2.2)); // �K���}�␳������
		diffuse += ambientDiffuse * (1 - specularRatio) * (1 - roughnessAndMetallic.y);
	}

	// �g�U���̉e���𔽉f����
	outColor.rgb *= diffuse;

	// ���ʔ��˂̉e���𔽉f����
	outColor.rgb += specular;

	// �����F�𔽉f����
	if(emission.w > 0)
		outColor.rgb += texture(texEmission, inTexcoord).rgb * emission.rgb;
	else
		outColor.rgb += emission.rgb;

  // �K���}�␳��image_processing.frag�ōs���̂ŁA�����ł͎��s���Ȃ��B
}
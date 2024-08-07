/**
* @file Engine.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Engine.h"
#include "ProgramObject.h"
#include "EasyAudio/EasyAudio.h"
#include "AabbCollider.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Debug.h"
#include "SkeletalMeshRenderer.h"

// �}�`�f�[�^
#include "../../Res/MeshData/crystal_mesh.h"
#include "../../Res/MeshData/plane_xy_mesh.h"

/// <summary>
/// OpenGL����̃��b�Z�[�W����������R�[���o�b�N�֐�
/// </summary>
/// <param name="source">���b�Z�[�W�̔��M��(OpenGL�AWindows�A�V�F�[�_�Ȃ�)</param>
/// <param name="type">���b�Z�[�W�̎��(�G���[�A�x���Ȃ�)</param>
/// <param name="id">���b�Z�[�W����ʂɎ��ʂ���l</param>
/// <param name="severity">���b�Z�[�W�̏d�v�x(���A���A��A�Œ�)</param>
/// <param name="length">���b�Z�[�W�̕�����.�����Ȃ烁�b�Z�[�W��0�I�[����Ă���</param>
/// <param name="message">���b�Z�[�W�{��</param>
/// <param name="userParam">�R�[���o�b�N�ݒ莞�Ɏw�肵���|�C���^</param>
void APIENTRY DebugCallback
(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	std::string s;
	// string.assign : ������̏�������(first��second��)
	(length < 0) ? s = message : s.assign(message, message + length);
	
	s += '\n'; // ���b�Z�[�W�ɂ͉��s���Ȃ��̂Œǉ�����
	// string.c_str() : string�^��char�^�ɕϊ�����֐�
	LOG(s.c_str());
}

/// <summary>
/// �Q�[���G���W�������s����
/// </summary>
/// <returns>0 = ����Ɏ��s�����������A0�ȊO = �G���[����������</returns>
int Engine::Run()
{
	const int result = Initialize();
	if(result)
		return result;
	// �������C�u����������������
	if (!EasyAudio::Initialize())
		return 1; // ���������s
	
	// �I�����������Ă��Ȃ����
	while(!glfwWindowShouldClose(window))
	{
		// �Q�[�������s����
		Update();
		Render();
		RemoveDestroyedGameObject();

		// �������C�u�������X�V����
		EasyAudio::Update();
	}

	// �������C�u�������I������
	EasyAudio::Finalize();

	// GLFW���I������
	glfwTerminate();
	return 0;
}

/// <summary>
/// �Q�[���I�u�W�F�N�g�z�u�t�@�C����ǂݍ���
/// </summary>
/// <param name="filename">�t�@�C����</param>
/// <param name="filepathMap"></param>
/// <returns>��̃I�u�W�F�N�g = �ǂݍ��ݎ��s�A�Q�[���I�u�W�F�N�g�z�� = �ǂݍ��ݐ���</returns>
GameObjectList Engine::LoadGameObjectMap
(
	const char* filename,
	const FilepathMap& filepathMap
)
{
	// �t�@�C�����J��
	std::ifstream ifs(filename);
	if (!ifs)
	{
		LOG_WARNING("%s���J���܂���", filename);
		return {};
	}

	struct PARENT_DATA
	{
		std::string parent;
		GameObjectPtr go;
	};
	std::vector<PARENT_DATA> objectMap;

	GameObjectList gameObjectList;
	gameObjectList.reserve(1000); // �K���Ȑ���\��

	// �t�@�C����ǂݍ���
	while (!ifs.eof())
	{
		std::string line;
		std::getline(ifs, line);
		const char* p = line.c_str();
		int readByte = 0; // �ǂݍ��񂾃o�C�g��

		// �Q�[���I�u�W�F�N�g��`�̊J�n����(�擪�ɂ͕K��name�v�f������͂�)
		char name[256] = { 0 }; // �Q�[���I�u�W�F�N�g��
		if (sscanf(p, R"( { "name" : "%255[^"]" %n)", name, &readByte) != 1)
			continue;  // ��`�ł͂Ȃ�
		
		p += readByte; // �ǂݎ��ʒu���X�V

		/* ���O�ȊO�̗v�f��ǂݍ��� */

		char parentName[256] = { 0 }; // �e�I�u�W�F�N�g��
		if (sscanf(p, R"(, "parent" : "%255[^"]" %n)", parentName, &readByte) == 1)
			p += readByte;

		char meshName[256] = { 0 };	  // ���b�V����
		if (sscanf(p, R"(, "mesh" : "%255[^"]" %n)", meshName, &readByte) == 1)
			p += readByte;

		char renderType[256] = { 0 }; // �`��^�C�v
		if (sscanf(p, R"(, "renderType" : "%255[^"]" %n)", renderType, &readByte) == 1)
			p += readByte;

		vec3 t(0); // ���s�ړ�
		if (sscanf(p, R"(, "translate" : [ %f, %f, %f ] %n)",
			&t.x, &t.y, &t.z, &readByte) == 3)
			p += readByte;

		vec3 r(0); // ��]
		if (sscanf(p, R"(, "rotate" : [ %f, %f, %f ] %n)",
			&r.x, &r.y, &r.z, &readByte) == 3)
			p += readByte;

		vec3 s(1); // �g�嗦
		if (sscanf(p, R"(, "scale" : [ %f, %f, %f ] %n)",
			&s.x, &s.y, &s.z, &readByte) == 3)
			p += readByte;

		// �Q�[���I�u�W�F�N�g���쐬����
		// Unity�͍�����W�n�Ȃ̂ŁA���s�ړ��Ɖ�]�������E����W�n�ɕϊ�����
		auto gameObject = Create<GameObject>(name);
		gameObject->position = vec3(t.x, t.y, -t.z);
		gameObject->rotation =
			vec3(radians(-r.x), radians(-r.y), radians(r.z));
		gameObject->scale = s;

		// ���b�V����ǂݍ���
		if (meshName[0])
		{
			// ���b�V�����ɑΉ�����t�@�C���p�X������
			// �Ή�����t�@�C���p�X��������΃��b�V����ǂݍ���
			const auto itr = filepathMap.find(meshName);
			if (itr != filepathMap.end())
			{
				const auto& path = itr->second.c_str();
				gameObject->staticMesh = meshBuffer->LoadOBJ(path);
			}
		} // if meshName[0]

		// �{�b�N�X�R���C�_�[���쐬����
		const char strBoxCollider[] = R"(, "BoxCollider" : [)";
		if (strncmp(p, strBoxCollider, sizeof(strBoxCollider) - 1) == 0)
		{
			p += sizeof(strBoxCollider) - 1;
			BOX box = { vec3(0), { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }, vec3(1) };
			for (;;)
			{
				if (sscanf(p, R"( { "center" : [ %f, %f, %f ], "size" : [ %f, %f, %f ] } %n)",
					&box.position.x, &box.position.y, &box.position.z,
					&box.scale.x, &box.scale.y, &box.scale.z, &readByte) != 6)
					break;
				
				auto collider = gameObject->AddComponent<BoxCollider>();
				collider->box = box;
				collider->box.position.z *= -1; // ������W�n���E����W�n�ɕϊ�
				collider->box.scale *= 0.5f;	// Unity��BoxCollider��1x1m(0.5m�l��)
				collider->isStatic = true;
				p += readByte;

				// �����ɃJ���}������ꍇ�͔�΂�
				if (*p == ',')
					++p;
			}
		} // if strBoxCollider

		// �Q�[���I�u�W�F�N�g�����X�g�ɒǉ�����
		gameObjectList.push_back(gameObject);
		objectMap.push_back({ parentName, gameObject });
	} // while

	for (auto& e : objectMap)
	{
		auto itr =
			std::find_if
			(
				gameObjectList.begin(),
				gameObjectList.end(),
				[&e](const GameObjectPtr& go)
				{
					return e.parent == go->name;
				}
			);
		if (itr != gameObjectList.end())
			e.go->SetParent(*itr);
	}

	// �쐬�����Q�[���I�u�W�F�N�g�z���Ԃ�
	LOG("�Q�[���I�u�W�F�N�g�z�u�t�@�C��%s��ǂݍ��݂܂���", filename);
	return gameObjectList;
}

/// <summary>
/// �Q�[���G���W������S�ẴQ�[���I�u�W�F�N�g��j������
/// </summary>
void Engine::ClearGameObjectAll()
{
	for (auto& e : gameObjects)
		e->OnDestroy();

	gameObjects.clear();
}

/// <summary>
/// ��������p��ݒ肷��
/// </summary>
/// <param name="fovY">�ݒ肷�鐂������p(�x���@)</param>
void Engine::SetFovY(float fovY)
{
	degFovY = fovY;
	radFovY = degFovY * 3.1415926535f / 180; // �ʓx�@�ɕϊ�
	fovScale = 1 / tan(radFovY / 2);		 // ����p�ɂ��g�嗦
}

/// <summary>
/// �}�E�X�J�[�\���̍��W���擾����
/// </summary>
/// <returns>�J�������W�n�̃J�[�\�����W</returns>
vec2 Engine::GetMousePosition() const
{
	// �X�N���[�����W�n�̃J�[�\�����W���擾����
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// �߂�^�ɍ��킹�邽�߂�float�ɃL���X�g
	const vec2 pos = { static_cast<float>(x),static_cast<float>(y) };

	// UILayer�̍��W�n�ɍ��킹�邽�߂ɁA�X�N���[�����W�n����J�������W�n(Z=-1)�ɕϊ�����
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	const vec2 framebufferSize = { static_cast<float>(w),static_cast<float>(h) };
	const float aspectRatio = framebufferSize.x / framebufferSize.y;
	return
	{
		(pos.x / framebufferSize.x * 2 - 1) * aspectRatio,
		(pos.y / framebufferSize.y * 2 - 1) * -1
	};
}

/// <summary>
/// �}�E�X�{�^���̏�Ԃ��擾����
/// </summary>
/// <param name="button">���͂��ꂽ�}�E�X�{�^���������}�N���萔(GLFW_MOUSE_BUTTON_LEFT��)</param>
/// <returns>true = ������Ă���Afalse = ������Ă��Ȃ�</returns>
bool Engine::GetMouseButton(int button) const
{
	return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

/// <summary>
/// �}�E�X�{�^���̃N���b�N��Ԃ��擾����
/// </summary>
/// <param name="button">���͂��ꂽ�}�E�X�{�^��(�{�^���������}�N���萔(GLFW_MOUSE_BUTTON_LEFT��))</param>
/// <returns>false = ������Ă��Ȃ��Afalse�ȊO = �����ꂽ</returns>
bool Engine::GetMouseClick(int button) const
{
	// �͈͊O�̃{�^���ԍ��͖���
	if (button<GLFW_MOUSE_BUTTON_LEFT
		|| button>GLFW_MOUSE_BUTTON_MIDDLE)
		return false;
	
	return mouseButtons[button].click;
}

/// <summary>
/// �t���[���o�b�t�@�̑傫�����擾����
/// </summary>
/// <returns>�t���[���o�b�t�@�̏c�Ɖ��̃T�C�Y</returns>
vec2 Engine::GetFramebufferSize() const
{
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	return { static_cast<float>(w),static_cast<float>(h) };
}

/// <summary>
/// �t���[���o�b�t�@�̃A�X�y�N�g����擾����
/// </summary>
/// <returns>�t���[���o�b�t�@�̃A�X�y�N�g��</returns>
float Engine::GetAspectRatio() const
{
	const vec2 size = GetFramebufferSize();
	return size.x / size.y;
}

/// <summary>
/// �e�N�X�`�����擾����
/// </summary>
/// <param name="name">�e�N�X�`���t�@�C����</param>
/// <returns>���O��name�ƈ�v����e�N�X�`��</returns>
TexturePtr Engine::GetTexture(const char* name)
{
	// �L���b�V���ɂ���΁A�L���b�V�����ꂽ�e�N�X�`����Ԃ�
	auto itr = textureCache.find(name);
	if (itr != textureCache.end())
		return itr->second; // �L���b�V�����ꂽ�e�N�X�`����Ԃ�

	// �L���b�V���ɂȂ���΁A�e�N�X�`�����쐬���ăL���b�V���ɓo�^
	auto tex = std::make_shared<TEX_HELPER>(name);
	textureCache.emplace(name, tex);
	return tex; // �쐬�����e�N�X�`����Ԃ�
}

/// <summary>
/// �L���[�u�}�b�v�e�N�X�`����ǂݍ���
/// </summary>
/// <param name="name">�e�N�X�`����</param>
/// <param name="cubemapFilenames">�L���[�u�}�b�v�p�̉摜�t�@�C�����̔z��</param>
/// <returns>���O��name�ƈ�v����e�N�X�`��</returns>
TexturePtr Engine::LoadCubemapTexture
(
	const char* name,
	const char* const cubemapFilenames[6]
)
{
	// �L���b�V���ɂ���΁A�L���V�����ꂽ�e�N�X�`����Ԃ�
	auto itr = textureCache.find(name);
	if (itr != textureCache.end())
		return itr->second; // �L���b�V�����ꂽ�e�N�X�`����Ԃ�

	// �L���b�V���ɂȂ���΁A�e�N�X�`�����쐬���ăL���b�V���ɓo�^
	auto tex = std::make_shared<TEX_HELPER>(name, cubemapFilenames);
	textureCache.emplace(name, tex);
	return tex; // �쐬�����e�N�X�`����Ԃ�
}

// ���C�g�z�������������
void Engine::InitializeLight()
{
	// �w�肳�ꂽ���̃��C�g�𐶐�����
	lights.resize(lightResizeCount);
	usedLights.reserve(lightResizeCount);

	// ���ׂẴ��C�g�𖢎g�p���C�g�z��Ɂu�t���v�Œǉ�����
	freeLights.resize(lightResizeCount);
	for (int i = 0; i < lightResizeCount; ++i)
		freeLights[i] = static_cast<int>(lightResizeCount - i - 1);
}

/// <summary>
/// �V�������C�g���擾����
/// </summary>
/// <returns>���C�g�̃C���f�b�N�X</returns>
int Engine::AllocateLight()
{
	// ���g�p�̃��C�g���Ȃ���΃��C�g�z����g������
	if (freeLights.empty())
	{
		const size_t oldSize = lights.size();
		lights.resize(oldSize + lightResizeCount);
		// �g���������C�g�𖢎g�p���C�g�z��Ɂu�t���v�Œǉ�����
		for (size_t i = lights.size()-1; i >= oldSize; --i)
			freeLights.push_back(static_cast<int>(i));
	}
	
	// ���g�p���C�g�z��̖�������C���f�b�N�X�����o��
	const int index = freeLights.back();
	freeLights.pop_back();

	// ���o�����C���f�b�N�X���g�p�����C�g�z��ɒǉ�����
	usedLights.push_back(index);

	// ���C�g�̏�Ԃ��u�g�p���v�ɂ���
	lights[index].isUsed = true;

	// ���o�����C���f�b�N�X��Ԃ�
	return index;
}

/// <summary>
/// ���C�g���������
/// </summary>
/// <param name="index">������郉�C�g�̃C���f�b�N�X</param>
void Engine::DeallocateLight(int index)
{
	if (index >= 0 && index < lights.size())
	{
		// �C���f�b�N�X�𖢎g�p���C�g�z��ɒǉ�����
		freeLights.push_back(index);

		// ���C�g�̏�Ԃ��u���g�p�v�ɂ���
		lights[index].isUsed = false;
	}
}

/// <summary>
/// �C���f�b�N�X�ɑΉ����郉�C�g�f�[�^���擾����
/// </summary>
/// <param name="index">���C�g�̃C���f�b�N�X</param>
/// <returns>nullptr�ȊO = index�ɑΉ����郉�C�g�f�[�^�̃A�h���X�Anullptr = �Ή����郉�C�g�f�[�^�����݂��Ȃ�</returns>
LIGHT_DATA* Engine::GetLight(int index)
{
	if (index >= 0 && index < lights.size() && lights[index].isUsed)
		return &lights[index];

	return nullptr;
}

/// <summary>
/// �C���f�b�N�X�ɑΉ����郉�C�g�f�[�^���擾����
/// </summary>
/// <param name="index">���C�g�̃C���f�b�N�X</param>
/// <returns>nullptr�ȊO = index�ɑΉ����郉�C�g�f�[�^�̃A�h���X�Anullptr = �Ή����郉�C�g�f�[�^�����݂��Ȃ�</returns>
const LIGHT_DATA* Engine::GetLight(int index) const
{
	return const_cast<Engine*>(this)->GetLight(index);
}

// �}�E�X���W���甭�˂����������擾����
RAY Engine::GetRayFromMousePosition() const
{
	// ���W�n�̃}�E�X�J�[�\�����W���擾����
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// �X�N���[�����W�n����NDC���W�n�ɕϊ�����
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	vec3 nearPos = {
		static_cast<float>(x / w * 2 - 1),
		-static_cast<float>(y / h * 2 - 1),
		-1
	};
	vec3 farPos = { nearPos.x,nearPos.y,1 };

	// �[�x�l�̌v�Z���ʂ�-1~+1�ɂȂ�悤�ȃp�����[�^A,B���v�Z����
	// ���_�V�F�[�_�̒l����v�����邱��
	const float near = 0.35f;
	const float far = 1000;
	const float A = -2 * far * near / (far - near);
	const float B = (far + near) / (far - near);

	// NDC���W�n����N���b�v���W�n�ɕϊ�����
	nearPos *= near;
	farPos *= far;
	nearPos.z=(nearPos.z - A) / B;
	farPos.z = (farPos.z - A) / B;

	// �N���b�v���W�n����r���[���W�n�ɕϊ�����
	const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
	const float invFovScale = 1.0f/GetFovScale();
	nearPos.x *= invFovScale * aspectRatio;
	nearPos.y *= invFovScale;
	farPos.x *= invFovScale * aspectRatio;
	farPos.y *= invFovScale;

	// �r���[���W�n���烏�[���h���W�n�ɕϊ�����
	const float cameraSinY = std::sin(camera.rotation.y);
	const float cameraCosY = std::cos(camera.rotation.y);
	nearPos = 
	{
		nearPos.x * cameraCosY - near * cameraSinY,
		nearPos.y,
		nearPos.x * -cameraSinY - near * cameraCosY
	};
	nearPos += camera.position;

	farPos = 
	{
		farPos.x * cameraCosY - far * cameraSinY,
		farPos.y,
		farPos.x * -cameraSinY - far * cameraCosY
	};
	farPos += camera.position;

	// �ߕ��ʂ̍��W�Ɖ����ʂ̍��W��������̌����x�N�g�������߂�
	vec3 direction = farPos - nearPos;
	const float length =
		sqrt
		(
			direction.x * direction.x +
			direction.y * direction.y +
			direction.z * direction.z
		);
	direction *= 1.0f / length;

	return RAY{ nearPos,direction };
}

/// <summary>
/// �����ƃR���C�_�[�̌�������
/// </summary>
/// <param name="ray">����</param>
/// <param name="hitInfo">�����ƍŏ��Ɍ��������R���C�_�[�̏��</param>
/// <param name="pred">����������s���R���C�_�[��I�ʂ���q��</param>
/// <returns>true = �R���C�_�[�ƌ��������Afalse = �ǂ̃R���C�_�[�Ƃ��������Ȃ�����</returns>
bool Engine::Raycast
(
	const RAY& ray,
	RAYCAST_HIT& hitInfo,
	const RaycastPredicate& pred
) const
{
	// ��_�̏���������
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	for (const auto& go : gameObjects)
		for (const auto& collider : go->colliders)
		{
			// �R���C�_�[�����[���h���W�n�ɕϊ�����
			const auto worldCollider =
				collider->GetTransformedCollider(go->GetTransformMatrix());

			// �����Ƃ̌�������
			float d;
			bool hit = false;
			switch (collider->GetType())
			{
			case Collider::TYPE_SHAPE::AABB:
				hit = Intersect(static_cast<AabbCollider&>(*worldCollider).aabb, ray, d);
				break;
			case Collider::TYPE_SHAPE::SPHERE:
				hit = Intersect(static_cast<SphereCollider&>(*worldCollider).sphere, ray, d);
				break;
			case Collider::TYPE_SHAPE::BOX:
				hit = Intersect(static_cast<BoxCollider&>(*worldCollider).box, ray, d);
				break;
			}
			if (!hit)
				continue;

			// ��������̑ΏۂłȂ���Δ�΂�
			if (!pred(collider, d))
				continue;

			// ��蔭�˓_�ɋ߂���_�����R���C�_�[��I��
			if (d < hitInfo.distance)
			{
				hitInfo.collider = collider;
				hitInfo.distance = d;
			}
		} // for colliders

	// ��������R���C�_�[�������true�A�Ȃ����false��Ԃ�
	if (hitInfo.collider)
	{
		//��_�̍��W���v�Z����
		hitInfo.point 
			= ray.start + ray.direction * hitInfo.distance;
		return true;
	}

	return false;
}

/// <summary>
/// �Q�[���G���W��������������
/// </summary>
/// <returns>0 = ����ɏ��������ꂽ�A0�ȊO = �G���[����������</returns>
int Engine::Initialize()
{
	// GLFW���C�u����������������
	if (glfwInit() != GLFW_TRUE)
		return 1; // ���������s

	// �`��E�C���h�E���쐬����
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	window =
		glfwCreateWindow
		(
			1280,
			720,
			title.c_str(),
			nullptr,
			nullptr
		);
	if (!window)
	{
		glfwTerminate();
		return 1; //�E�C���h�E�쐬���s
	}

	// OpenGL�R���e�L�X�g���쐬����
	glfwMakeContextCurrent(window);

	// �L���X�g(�^�ϊ�)�ɂ���
	// const_cast�E�E�Econst�̕t���O��
	// static_cast�E�E�E�|�C���^�������^�ϊ�
	// reinterpret_cast�E�E�E�|�C���^�̌^�ϊ�
	// dynamic_cast�E�E�E���N���X����h���N���X�ւ̃L���X�g

	// OpenGL�֐��̃A�h���X���擾����
	// ���C�u�����Ԃ̕s�������������邽�߂�reinterpret_cast���g��
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		glfwTerminate();
		return 1; // �A�h���X�擾���s
	}

	// ���b�Z�[�W�R�[���o�b�N��ݒ肷��
	glDebugMessageCallback(DebugCallback, nullptr);

	// ���C�g�f�[�^������������
	InitializeLight();

	// �V�F�[�_��ǂݍ���ŃR���p�C��
	prog3D =
		std::make_shared<ProgramObject>
		(
			"Res/standard.vert",
			"Res/standard.frag"
		);

	// �X�P���^�����b�V���E�V�F�[�_���쐬����
	progSkeletal =
		std::make_shared<ProgramObject>
		(
			"Res/skeletal.vert",
			"Res/standard.frag"
		);

	progShadowSkeletal =
		std::make_shared<ProgramObject>
		(
			"Res/shadow_skeletal.vert",
			"Res/shadow.frag"
		);

	progHighPassFilter =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/high_pass_filter.frag"
		);
	progDownSampling =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/down_sampling.frag"
		);
	progUpSampling =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/up_sampling.frag"
		);

	progImageProcessing =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/image_processing.frag"
		);

	progSAORestoreDepth = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao_restore_depth.frag");
	progSAODownSampling = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao_down_sampling.frag");
	progSAO = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao.frag");
	progSAOBlur = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao_blur.frag");

	// �e�N�X�`����`�悷�邾���̃V�F�[�_
	progSimple = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/simple.frag");

	// �V���h�E�E�V�F�[�_���쐬
	progShadow =std::make_shared<ProgramObject>(
		"Res/shadow.vert","Res/shadow.frag"	);

	// �J�b�g�I�t�l��ݒ肵�Ă���
	glProgramUniform1f(*progShadow, locAlphaCutoff, 0.5f);
	glProgramUniform1f(*progShadowSkeletal, locAlphaCutoff, 0.5f);

	// FBO���쐬����
	CreateMainFBO(1920, 1080);
	auto texShadow =
		std::make_shared<TEX_HELPER>
		(
			"FBO(depth)",
			2048,
			2048,
			GL_DEPTH_COMPONENT32
		);
	fboShadow = 
		std::make_shared<FramebufferObject>
		(
			nullptr,
			texShadow
		);

	// �A�����b�g�E�V�F�[�_���쐬����
	progUnlit =
		std::make_shared<ProgramObject>
		(
			"Res/unlit.vert",
			"Res/unlit.frag"
		);

	// �֐ߗp��SSBO���쐬����
	for (size_t i = 0; i < std::size(ssboJointMatrices); ++i)
		ssboJointMatrices[i] = BufferObject::Create(jointMatricesBytes);
	jointMatrixIndex = 0;

	// ���_�f�[�^��GPU�������ɃR�s�[
	const VERTEX vertexData[] =
	{
		// +Z(��O�̖�)
		{{-1,-1,1},{0,0}},
		{{1,-1,1},{1,0}},
		{{1,1,1},{1,1}},
		{{-1,1,1},{0,1}},

		// -Z(���̖�)
		{{1,-1,-1},{0,0}},
		{{-1,-1,-1},{1,0}},
		{{-1,1,-1},{1,1}},
		{{1,1,-1},{0,1}},

		// +Y(��̖�)
		{{1,1,1},{0.0}},
		{{1,1,-1},{1,0}},
		{{-1,1,-1},{1,1}},
		{{-1,1,1},{0,1}},

		// -Y(���̖�)
		{{1,-1,-1},{0,0}},
		{{1,-1,1},{1,0}},
		{{-1,-1,1},{1,1}},
		{{-1,-1,-1},{0,1}},

		// +X
		{{1,-1,1},{0,0}},
		{{1,-1,-1},{1,0}},
		{{1,1,-1},{1,1}},
		{{1,1,1},{0,1}},

		// -X
		{{-1,-1,1},{0,0}},
		{{-1,-1,-1},{1,0}},
		{{-1,1,-1},{1,1}},
		{{-1,1,1},{0,1}},
	};

	const GLushort indexData[] =
	{
		0,1,2,2,3,0,
		4,5,6,6,7,4,
		8,9,10,10,11,8,
		12,13,14,14,15,12,
		16,17,18,18,19,16,
		20,21,22,22,23,20,
	};

	//�}�`�f�[�^�̏��
	struct MESH_DATA
	{
		const char* name;            // ���b�V����
		const char* textureFilename; // �e�N�X�`���t�@�C����
		size_t vertexSize;			 // ���_�f�[�^�̃o�C�g��
		size_t indexSize;			 // �C���f�b�N�X�f�[�^�̃o�C�g��
		const void* vertexData;		 // ���_�f�[�^�̃A�h���X
		const void* indexData;		 // �C���f�b�N�X�f�[�^�̃A�h���X
	};
	const MESH_DATA meshes[] =
	{
		{ 
			"crystal",
			nullptr,
			sizeof(crystal_vertices),
			sizeof(crystal_indices),
			crystal_vertices,
			crystal_indices,
		},
		{
			"plane_xy",
			nullptr,
			sizeof(plane_xy_vertices),
			sizeof(plane_xy_indices),
			plane_xy_vertices,
			plane_xy_indices
		},
	};

	// �}�`�f�[�^����`��p�����[�^���쐬���A�f�[�^��GPU�������ɃR�s�[����
	meshBuffer = MeshBuffer::Create(32'000'000);
	meshBuffer->SetTextureCallback([this](const char* filename)
		{ return GetTexture(filename); });
	for (const auto& e : meshes)
	{
		// �@����ݒ肷�邽�߂ɐ}�`�f�[�^�̃R�s�[�����
		auto pVertex = static_cast<const VERTEX*>(e.vertexData);
		auto pIndex = static_cast<const uint16_t*>(e.indexData);
		std::vector<VERTEX> v(pVertex, pVertex + e.vertexSize / sizeof(VERTEX));

		// �R�s�[�����}�`�f�[�^�ɖ@����ݒ肷��
		for (auto& e : v)
			e.normal = { 0,0,0 };
		FillMissingNormals(v.data(), v.size(), pIndex, e.indexSize / sizeof(uint16_t));

		// �@����ݒ肵���}�`�f�[�^��GPU�������ɃR�s�[
		meshBuffer->AddVertexData(v.data(), e.vertexSize, pIndex, e.indexSize);

		// ���O��AddVertexData�ō쐬�����`��p�����[�^���擾����
		const DRAW_PARAMS& drawParams =
			meshBuffer->GetDrawParams(meshBuffer->GetDrawParamsCount() - 1);

		// �e�N�X�`�����쐬����
		TexturePtr texBaseColor;
		if (e.textureFilename)
			texBaseColor = GetTexture(e.textureFilename);

		// �`��p�����[�^�ƃe�N�X�`������X�^�e�B�b�N���b�V�����쐬����
		meshBuffer->CreateStaticMesh(e.name, drawParams, texBaseColor);
	}

	// OBJ�t�@�C����ǂݍ���
	skySphere = meshBuffer->LoadOBJ("Res/MeshData/sky_sphere/sky_sphere.obj");

	// �h���S��
	{
		MeshBuffer::SKELETON skeleton;
		skeleton.bones.resize(9);
		skeleton.bones[0] = { { 0.00f, 0.00f, 0.00f }, { 0.00f, 0.38f, 0.87f },-1, 0.7f }; // ����
		skeleton.bones[1] = { { 0.00f, 0.86f, 2.86f }, { 0.00f, 0.31f, 2.79f }, 0, 0.5f }; // ��
		skeleton.bones[2] = { {-0.40f, 0.50f, 1.80f }, {-1.51f, 0.61f, 1.48f }, 0, 0.5f }; // �E�r
		skeleton.bones[3] = { { 0.40f, 0.50f, 1.80f }, {-1.51f, 0.61f, 1.48f }, 0, 0.5f }; // ���r
		skeleton.bones[4] = { {-0.34f, 0.22f, 1.23f }, {-0.48f,-0.25f, 0.17f }, 0, 0.3f }; // �E��
		skeleton.bones[5] = { { 0.34f, 0.22f, 1.23f }, {-0.48f,-0.25f, 0.17f }, 0, 0.3f }; // ����
		skeleton.bones[6] = { {-0.60f,-0.03f, 2.21f }, {-2.08f,-0.21f, 2.50f }, 0, 0.3f }; // �E��
		skeleton.bones[7] = { { 0.60f,-0.03f, 2.21f }, { 2.08f,-0.21f, 2.50f }, 0, 0.3f }; // ����
		skeleton.bones[8] = { { 0.00f,-0.66f, 0.93f }, { 0.00f,-3.45f, 0.35f }, 0, 0.5f }; // ��
		meshBuffer->LoadOBJ("Res/MeshData/free_rocks/Dragon.obj", skeleton);
	}
	// �I�[�N
	{
		MeshBuffer::SKELETON skeleton;
		skeleton.bones.resize(6);
		skeleton.bones[0] = { { 0.0f, 1.00f,-0.2f }, { 0.0f, 1.80f,-0.2f },-1, 0.7f }; // ����
		skeleton.bones[1] = { { 0.0f, 1.70f, 0.1f }, { 0.0f, 1.70f, 0.3f }, 0, 0.5f }; // ��
		skeleton.bones[2] = { {-0.4f, 1.65f,-0.2f }, {-1.6f, 1.65f,-0.2f }, 0, 0.5f }; // �E�r
		skeleton.bones[3] = { { 0.4f, 1.65f,-0.2f }, { 1.6f, 1.65f,-0.2f }, 0, 0.5f }; // ���r
		skeleton.bones[4] = { {-0.2f, 0.90f,-0.2f }, {-0.2f, 0.00f,-0.2f }, 0, 0.3f }; // �E��
		skeleton.bones[5] = { { 0.2f, 0.90f,-0.2f }, { 0.2f, 0.00f,-0.2f }, 0, 0.3f }; // ����
		meshBuffer->LoadOBJ("Res/MeshData/orcshaman/orcshaman_fighter.obj",skeleton);
		meshBuffer->LoadOBJ("Res/MeshData/orcshaman/orcshaman_magician.obj",skeleton);
	}
	meshBuffer->LoadOBJ("Res/MeshData/arm_and_hand/arm_and_hand_grab.obj");
	meshBuffer->LoadOBJ("Res/MeshData/arm_and_hand/sword.obj");

	// �Q�[���I�u�W�F�N�g�z��̗e�ʂ�\�񂷂�
	gameObjects.reserve(1000);

	// �J�����̏����ݒ������
	camera.position = { 3,1,3 };
	camera.rotation.y = 3.14159265f;

	return 0;
}

/// <summary>
/// FBO���쐬����
/// </summary>
/// <param name="width">���C��FBO�̕�(�s�N�Z���P��)</param>
/// <param name="height">���C��FBO�̍���(�s�N�Z���P��)</param>
void Engine::CreateMainFBO
(
	int width,
	int height
)
{
	// �ȑO��FBO���폜����
	fboMain.reset();

	// �V����FBO���쐬����
	auto texMainColor =
		std::make_shared<TEX_HELPER>
		(
			"FBO(main color)",
			width,
			height,
			GL_RGBA16F
		);
	auto texMainDepth =
		std::make_shared<TEX_HELPER>
		(
			"FBO(main depth)",
			width,
			height,
			GL_DEPTH_COMPONENT32
		);
	fboMain = 
		std::make_shared<FramebufferObject>
		(
			texMainColor,
			texMainDepth
		);

	// SAO�pFBO���쐬����
	const int maxMipLevel = static_cast<int>(std::size(fboSAODepth));
	auto texSAODepth = std::make_shared<TEX_HELPER>(
		"FBO(sao depth)", width / 2, height / 2, GL_R32F, maxMipLevel);
	for (int level = 0; level < maxMipLevel; ++level)
		fboSAODepth[level] = std::make_shared<FramebufferObject>(texSAODepth, nullptr, level, 0);

	auto texSAO = std::make_shared<TEX_HELPER>(
		"FBO(sao)", width / 2, height / 2, GL_R8);
	fboSAO = std::make_shared<FramebufferObject>(texSAO, nullptr);
	auto texSAOBlur = std::make_shared<TEX_HELPER>(
		"FBO(sao blur)", width / 2, height / 2, GL_R8);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_A, GL_RED);
	fboSAOBlur = std::make_shared<FramebufferObject>(texSAOBlur, nullptr);

	// �u���[���pFBO���쐬����
	fboBloom.clear();
	fboBloom.resize(6);
	width /= 4; // 0�Ԃ̃u���[���pFBO�͏c��1/4�ɏk��
	height /= 4;
	char name[] = "FBO(bloom[0])";
	for (int i = 0; i < fboBloom.size(); ++i)
	{
		name[10] = '0' + i;
		auto texColor = 
			std::make_shared<TEX_HELPER>
			(
				name,
				width,
				height,
				GL_RGBA16F
			);
		fboBloom[i] = 
			std::make_shared<FramebufferObject>
			(
				texColor,
				nullptr
			);
		width /= 2; // 1�Ԉȍ~��FBO�͑O�i�̏c��1/2�ɏk��
		height /= 2;
	} // for i
}

/// <summary>
/// �Q�[���G���W���̏�Ԃ��X�V����
/// </summary>
void Engine::Update()
{
	// �f���^�^�C��(�O��̍X�V����̌o�ߎ���)���v�Z
	const double currentTime = glfwGetTime(); // ���ݎ���
	deltaTime = static_cast<float>(currentTime - previousTime);
	previousTime = currentTime;

	// �o�ߎ��Ԃ���������ꍇ�͓K���ɒZ������(��Ƀf�o�b�O�΍�)
	if (deltaTime >= 0.5f)
		deltaTime = 1.0f / 60.0f;

	// �}�E�X�{�^���̏�Ԃ��擾
	for (int i = 0; i < std::size(mouseButtons); ++i)
	{
		MOUSEBUTTON& e = mouseButtons[i];
		e.previous = e.current;
		e.current = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT + i);

		// ������Ă��鎞�Ԃ��X�V
		(e.previous) ? e.timer += deltaTime : e.timer = 0;

		// �ȉ��̏��������ׂĖ������ꍇ�̓N���b�N�Ɣ���
		// -�O�t���[���ŉ�����Ă���
		// -���݃t���[���ŉ�����Ă��Ȃ�
		// -������Ă��鎞�Ԃ�0.3�b�ȉ�
		(e.previous && !e.current && e.timer <= mouseClickSpeed) ? e.click = true : e.click = false;
	} // for i

	// �}�E�X���W���擾����
	oldMousePosition = mousePosition;
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	mousePosition.x = static_cast<float>(x);
	mousePosition.y = static_cast<float>(y);
	mouseMovement = mousePosition - oldMousePosition;

	// �V�[����؂�ւ���
	if (nextScene)
	{
		if (scene)
			scene->Finalize(*this);
		
		nextScene->Initialize(*this);
		scene = std::move(nextScene);
	}

	// �V�[�����X�V����
	if (scene)
		scene->Update(*this, deltaTime);

	UpdateGameObject(deltaTime);
	UpdateJointMatrix();
	HandleGameObjectCollision();
}

/// <summary>
/// �Q�[���G���W���̏�Ԃ�`�悷��
/// </summary>
void Engine::Render()
{
	// �Q�[���I�u�W�F�N�g�������_�[�L���[���ɕ��בւ���
	std::stable_sort
	(
		gameObjects.begin(),
		gameObjects.end(),
		[](const GameObjectPtr& a, const GameObjectPtr& b)
		{
			return a->render_queue < b->render_queue;
		}
	);

	// transparent�L���[�̐擪����������
	const auto transparentBegin = std::lower_bound
	(
		gameObjects.begin(),
		gameObjects.end(),
		RENDER_QUEUE_TRANSPARENT,
		[](const GameObjectPtr& e, int value)
		{
			return e->render_queue < value;
		}
	);

	// �t���[���o�b�t�@�̑傫�����擾����
	int fbWidth, fbHeight;
	glfwGetFramebufferSize
	(
		window,
		&fbWidth,
		&fbHeight
	);

	// �u���[���G�t�F�N�g��`�悷��
	DrawBloomEffect();

	// �`�����f�t�H���g�t���[���o�b�t�@�ɖ߂�
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, fbWidth, fbHeight);

	// 3D�`��̌��ʂ��摜�������ăf�t�H���g�t���[���o�b�t�@�ɏ�������
	DrawMainImageToDefaultFramebuffer();

	// overlay�L���[�̐擪����������
	const auto overlayBegin = std::lower_bound
	(
		transparentBegin,
		gameObjects.end(),
		RENDER_QUEUE_OVERLAY,
		[](const GameObjectPtr& e, int value)
		{
			return e->render_queue < value;
		}
	);

	// �f�v�X�V���h�E�}�b�v���쐬����
	CreateShadowMap(gameObjects.begin(), transparentBegin);

	// �`����3D�`��pFBO�ɕύX����
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMain);
	glViewport(0, 0, fboMain->GetWidth(), fboMain->GetHeight());

	// �o�b�N�o�b�t�@���N���A
	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const GLuint programs[] = { *prog3D,*progUnlit,*progSkeletal };
	for (auto prog : programs)
	{
		// �A�X�y�N�g��Ǝ���p��ݒ肷��
		const float aspectRatio =
			static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
		glProgramUniform2f(prog, 3, fovScale / aspectRatio, fovScale);

		// �J�����p�����[�^��ݒ肷��
		glProgramUniform3fv(prog, 4, 1, &camera.position.x);
		glProgramUniform2f(prog, 5, sin(-camera.rotation.y), cos(-camera.rotation.y));
		glProgramUniform2f(prog, 6, sin(-camera.rotation.x), cos(-camera.rotation.x));
	}
	// �[�x�e�X�g��L��������
	glEnable(GL_DEPTH_TEST);

	// ������������L��������
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	UpdateShaderLight();

	// ���}�b�s���O�p�L���[�u�}�b�v��ݒ�
	if (texEnvironment)
	{
		const GLuint tex = *texEnvironment;
		glBindTextures(4, 1, &tex);
	}

	glUseProgram(*prog3D);

	// transparent�ȑO�̃L���[��`�悷��
	glProgramUniform1f(*prog3D, locAlphaCutoff, 0.5f);
	DrawGameObject(*prog3D, gameObjects.begin(), transparentBegin);
	glProgramUniform1f(*prog3D, locAlphaCutoff, 0);

	// �A���r�G���g�I�N���[�W������`�悷��
	DrawAmbientOcclusion();

	// �X�J�C�X�t�B�A��`�悷��
	DrawSkySphere();

	// transparent����overlay�܂ł̃L���[��`�悷��
	glDepthMask(GL_FALSE); // �[�x�o�b�t�@�ւ̏������݂��֎~
	DrawGameObject(*prog3D, transparentBegin, overlayBegin);
	glDepthMask(GL_TRUE);  // �[�x�o�b�t�@�ւ̏������݂�����

	// overlay�ȍ~�̃L���[��`�悷��
	glDisable(GL_DEPTH_TEST); // �[�x�e�X�g�𖳌���
	glUseProgram(*progUnlit);
	DrawGameObject(*progUnlit, overlayBegin, gameObjects.end());

	// �`�抮���҂��̂��߁A�����I�u�W�F�N�g���쐬����
	syncJointMatrix[jointMatrixIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	// �g���v���o�b�t�@�̃C���f�b�N�X���X�V����
	jointMatrixIndex = (jointMatrixIndex + 1) % std::size(ssboJointMatrices);

#if 0
	// �V���h�E�}�b�v�̃`�F�b�N�p
	{
		glBindVertexArray(*meshBuffer->GetVAO());
		MaterialList materials(1, std::make_shared<MATERIAL>());
		materials[0]->texBaseColor = fboShadow->GetDepthTexture();
		mat4 m = GetTransformMatrix(vec3(1), vec3(0), vec3(0, 1.5f, -4));
		glProgramUniformMatrix4fv(*progUnlit, locTransformMatrix, 1, GL_FALSE, &m[0].x);
		Draw(*meshBuffer->GetStaticMesh("plane_xy"), *progUnlit, materials);
		glBindVertexArray(0);
	}
#endif

	glfwSwapBuffers(window);
	glfwPollEvents();
}

/// <summary>
/// �Q�[���I�u�W�F�N�g�z���`�悷��
/// </summary>
/// <param name="prog">�`��Ɏg���v���O�����p�C�v���C���I�u�W�F�N�g
/// <param name="begin">�`�悷��Q�[���I�u�W�F�N�g�͈͂̐擪</param>
/// <param name="end">�`�悷��Q�[���I�u�W�F�N�g�͈͂̏I�[</param>
void Engine::DrawGameObject
(
	ProgramObject& prog,
	GameObjectList::iterator begin,
	GameObjectList::iterator end
)
{
	glBindVertexArray(*meshBuffer->GetVAO());
	for (GameObjectList::iterator i = begin; i != end; ++i)
	{
		const auto& e = *i;

		// ���b�V�����S���ݒ肳��Ă��Ȃ��ꍇ�͕`�悵�Ȃ�
		if (!(e->staticMesh || e->renderer))
			continue;

		// ���j�t�H�[���ϐ��Ƀf�[�^���R�s�[
		glProgramUniform4fv(prog, 100, 1, &e->color.x);

		// ���W�ϊ��x�N�g���̔z���GPU�������ɃR�s�[
		glProgramUniformMatrix4fv(prog, 0, 1, GL_FALSE, &e->GetTransformMatrix()[0].x);
		if (prog != *progUnlit)
			glProgramUniformMatrix3fv(prog, 1, 1, GL_FALSE, &e->GetNormalMatrix()[0].x);

		// �X�^�e�B�b�N���b�V����`�悷��
		if (e->staticMesh)
			if (e->materials.empty())
				Draw(*e->staticMesh, prog, e->staticMesh->materials);
			else
				Draw(*e->staticMesh, prog, e->materials);

		// �X�P���^�����b�V����`�悷��
		if (e->renderer)
		{
			// �X�P���^�����b�V���p�̃V�F�[�_��VAO�����蓖�Ă�
			glUseProgram(*progSkeletal);
			glBindVertexArray(*meshBuffer->GetVAOSkeletal());

			e->renderer->Draw(*e, *progSkeletal,
				*ssboJointMatrices[jointMatrixIndex], bpJointMatrices);

			// ���̃V�F�[�_��VAO�ɖ߂�
			glBindVertexArray(*meshBuffer->GetVAO());
			glUseProgram(prog);
		} // if e->renderer
	}
	glBindVertexArray(0);
}

/// <summary>
/// �Q�[���I�u�W�F�N�g�̏�Ԃ��X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Engine::UpdateGameObject(float deltaTime)
{
	// �v�f�̒ǉ��ɑΉ����邽�ߓY����I������
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* e = gameObjects[i].get();
		if (!e->IsDestroyed())
		{
			e->Start();
			e->Update(deltaTime);
		}
	}

	// ���[�J�����W�ϊ��s����v�Z����
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* e = gameObjects[i].get();
		e->matrix_transform = GetTransformMatrix(e->scale, e->rotation, e->position);
		e->matrix_normal = GetRotationMatrix(e->rotation);
	}

	// ���[���h���W�ϊ��s����v�Z����
	std::vector<mat4> worldTransforms(gameObjects.size());
	std::vector<mat3> worldNormals(gameObjects.size());
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* e = gameObjects[i].get();
		mat4 m = e->matrix_transform;
		mat3 n = e->matrix_normal;
		int parentJointNo = e->number_joint_parent; // �e�̊֐ߔԍ�

		// ���ׂĂ̑c��̍��W�ϊ��s�����Z����
		for (e = e->parent; e; e = e->parent)
		{
			// �e�̊֐ߔԍ�������ꍇ�A��Ɋ֐ߍs�����Z����
			if (parentJointNo >= 0 && e->renderer)
			{
				const mat4& matJoint = e->renderer->GetJointMatrix(parentJointNo);

				// �֐ߍs�񂩂��]���������o��
				vec3 t, s; // �ʒu�Ɗg�嗦�͎g��Ȃ�
				mat3 matJointNormal;
				Decompose(matJoint, t, s, matJointNormal);

				// �֐ߍs�����Z����
				m = matJoint * m;
				n = matJointNormal * n;

				// ���̐e�̊֐ߔԍ����擾����
				parentJointNo = e->number_joint_parent;
			}      m = e->matrix_transform * m;
			n = e->matrix_normal * n;
		}
		worldTransforms[i] = m;
		worldNormals[i] = n;
	}

	// ���[���h���W�ϊ��s����Q�[���I�u�W�F�N�g�ɐݒ肷��
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		gameObjects[i]->matrix_transform = worldTransforms[i];
		gameObjects[i]->matrix_normal = worldNormals[i];
	}
} // UpdateGameObject

/// <summary>
/// �֐ߍs����X�V����
/// </summary>
void Engine::UpdateJointMatrix()
{
	const size_t jointMatrixSize = jointMatricesBytes / sizeof(mat4);
	std::shared_ptr<mat4[]> jointMatrices(new mat4[jointMatrixSize]);
	size_t totalJointCount = 0; // �R�s�[�����֐ߍs��̐�

	// �������ݏ�����1��ōς܂��邽�߂ɁA�S�f�[�^���o�b�t�@�ɒ~��
	for (const auto& go : gameObjects)
	{
		if (!go->renderer)
			continue;

		// �f�[�^�T�C�Y��256�o�C�g���E(mat4���Z��4��)�ɐ؂�グ��
		const size_t jointCount =
			((go->renderer->GetJointMatrixCount() + 3) / 4) * 4;

		// ���W�ϊ��s����v�Z���ăo�b�t�@�ɐݒ肷��
		go->renderer->CalculateJointMatrix(&jointMatrices[totalJointCount]);

		go->renderer->gpuOffset = totalJointCount * sizeof(mat4); //�ݒ肵���ʒu���L�^����
		totalJointCount += jointCount; // �֐ߍs��̑������X�V����
	}

	// �`�抮����҂�
	if (syncJointMatrix[jointMatrixIndex])
	{
		const GLuint64 timeout = 16'700'000; // 16.7�~���b(��1/60�b)
		glClientWaitSync(syncJointMatrix[jointMatrixIndex],
			GL_SYNC_FLUSH_COMMANDS_BIT, timeout);

		// �����I�u�W�F�N�g���폜����
		glDeleteSync(syncJointMatrix[jointMatrixIndex]);
		syncJointMatrix[jointMatrixIndex] = 0;
	}

	// �֐ߍs���GPU�������ɃR�s�[
	if (totalJointCount > 0)
	{
		const size_t size = sizeof(mat4) * totalJointCount;
		ssboJointMatrices[jointMatrixIndex]->CopyData(0, jointMatrices.get(), size);
	}
} // UpdateJointMatrix

/// <summary>
/// �Q�[���I�u�W�F�N�g�Ԃ̏Փ˂���������
/// </summary>
void Engine::HandleGameObjectCollision()
{
	// ���[���h���W�n�̏Փ˔�����쐬����
	std::vector<WorldColliderList> colliders;
	colliders.reserve(gameObjects.size());
	for (const auto& e : gameObjects)
	{
		if (e->colliders.empty())
			continue;

		// �u�ڒn���Ă��Ȃ��v��Ԃɂ���
		e->isGrounded = false;

		// �Փ˔�����쐬
		WorldColliderList list(e->colliders.size());
		for (int i = 0; i < e->colliders.size(); ++i)
		{
			// �I���W�i���̃R���C�_�[���R�s�[
			list[i].origin = e->colliders[i];

			// �R���C�_�[�̍��W�����[���h���W�ɕϊ�
			list[i].world = e->colliders[i]->GetTransformedCollider(e->GetTransformMatrix());
		}
		colliders.push_back(list);
	}

	if (colliders.size() >= 2)
	{
		// �Q�[���I�u�W�F�N�g�P�ʂ̏Փ˔���
		for (auto a = colliders.begin(); a != colliders.end() - 1; ++a)
		{
			const GameObject* goA = a->at(0).origin->GetOwner();
			if (goA->IsDestroyed())
				continue; // �폜�ς݂Ȃ̂Ŕ�΂�
			for (auto b = a + 1; b != colliders.end(); ++b)
			{
				const GameObject* goB = b->at(0).origin->GetOwner();
				if (goB->IsDestroyed())
					continue; //�폜�ς݂Ȃ̂Ŕ�΂�
				HandleWorldColliderCollision(&*a, &*b);	// �R���C�_�[�P�ʂ̏Փ˔���
			} // for b
		} // for a
	}
}

// 

/// <summary>
/// �^�ɂ���Č�������֐����Ăѕ����邽�߂̊֐��e���v���[�g
/// </summary>
template<typename T, typename U>
bool CallIntersect
(
	const ColliderPtr& a,
	const ColliderPtr& b,
	vec3& p
)
{
	return
		Intersect
		(
			static_cast<T&>(*a).GetShape(),
			static_cast<U&>(*b).GetShape(),
			p
		);
}


/// <summary>
/// �^�ɂ���Č�������֐����Ăѕ����邽�߂̊֐��e���v���[�g
/// ��������֐��ɓn���������t�ɂ���o�[�W����
/// </summary>
template<typename T, typename U>
bool CallIntersectReverse
(
	const ColliderPtr& a,
	const ColliderPtr& b,
	vec3& p
)
{
	if (Intersect(static_cast<U&>(*b).GetShape(),
		static_cast<T&>(*a).GetShape(), p))
	{
		p *= -1; // �ђʃx�N�g�����t�����ɂ���
		return true;
	}
	return false;
}


/// <summary>
/// ���false��Ԃ��֐��e���v���[�g
/// �������܂��͎����\��̂Ȃ��g�ݍ��킹�p
/// </summary>
template<typename T, typename U>
bool NotImplemented
(
	const ColliderPtr& a,
	const ColliderPtr& b,
	vec3& p
)
{
	return false;
}

/// <summary>
/// �ђʃx�N�g�����Q�[���I�u�W�F�N�g�ɔ��f����
/// </summary>
/// <param name="worldColliders">
/// �Փ˔���Ɏg�p���ꂽ���[���h���W�n�̃R���C�_�[���X�g�ւ̃|�C���^
/// </param>
/// <param name="gameObject">
/// �Փ˔���̑ΏۂƂȂ����Q�[���I�u�W�F�N�g�ւ̃|�C���^
/// </param>
/// <param name="penetration">�Փˌ�̃I�u�W�F�N�g�̊ђʗ�</param>
void Engine::ApplyPenetration
(
	WorldColliderList* worldColliders,
	GameObject* gameObject,
	const vec3& penetration
)
{
	// �ڒn����
	// �Փ˃x�N�g���������ɋ߂��ꍇ�ɁA���ɐG�ꂽ�Ƃ݂Ȃ�
	static const float cosGround = cos(radians(30)); // ���Ƃ݂Ȃ��p�x
	if (penetration.y > 0)
	{
		// �Ώۂ��P�ʐ����x�N�g���ł��邱�Ƃ𗘗p���āA���ςɂ��p�x�̔�r��P����
		const float d = length(penetration);
		if (penetration.y >= d * cosGround)
			gameObject->isGrounded = true; // �ڒn����
	} // if penetration.y

	// �Q�[���I�u�W�F�N�g���ړ�
	gameObject->position += penetration;

	// �S�Ẵ��[���h�R���C�_�[���ړ�
	for (auto& e : *worldColliders)
		e.world->AddPosition(penetration);
}

/// <summary>
/// �R���C�_�[�P�ʂ̏Փ˔���
/// </summary>
/// <param name="a">����Ώۂ̃��[���h�R���C�_�[�z�񂻂�1</param>
/// <param name="b">����Ώۂ̃��[���h�R���C�_�[�z�񂻂�2</param>
void Engine::HandleWorldColliderCollision(WorldColliderList* a, WorldColliderList* b)
{
	// �֐��|�C���^�^���`����
	using FuncType = bool(*)(const ColliderPtr&, const ColliderPtr&, vec3&);

	// �g�ݍ��킹�ɑΉ������������֐���I�Ԃ��߂̔z��
	static const FuncType funcList[3][3] =
	{
		{
			CallIntersect<AabbCollider, AabbCollider>,
			CallIntersect<AabbCollider, SphereCollider>,
			NotImplemented<AabbCollider, BoxCollider>,
		},
		{
			CallIntersectReverse<SphereCollider, AabbCollider>,
			CallIntersect<SphereCollider, SphereCollider>,
			CallIntersectReverse<SphereCollider, BoxCollider>,
		},
		{
			NotImplemented<BoxCollider, AabbCollider>,
			CallIntersect<BoxCollider, SphereCollider>,
			NotImplemented<BoxCollider, BoxCollider>,
		},
	};

	// �R���C�_�[�P�ʂ̏Փ˔���
	for (auto& colA : *a)
	{
		for (auto& colB : *b)
		{
			// �X�^�e�B�b�N�R���C�_�[���m�͏Փ˂��Ȃ�
			if (colA.origin->isStatic && colB.origin->isStatic)
				continue;

			// �Փ˔�����s��
			vec3 penetration;
			const int typeA = static_cast<int>(colA.origin->GetType());
			const int typeB = static_cast<int>(colB.origin->GetType());
			if (funcList[typeA][typeB](colA.world, colB.world, penetration))
			{
				GameObject* goA = colA.origin->GetOwner();
				GameObject* goB = colB.origin->GetOwner();

				// �R���C�_�[���d�Ȃ�Ȃ��悤�ɍ��W�𒲐�
				if (!colA.origin->isTrigger && !colB.origin->isTrigger)
					if (colA.origin->isStatic)
						//A�͓����Ȃ��̂�B���ړ�������
						ApplyPenetration(b, goB, penetration);
					else if (colB.origin->isStatic)
						//B�͓����Ȃ��̂�A���ړ�������
						ApplyPenetration(a, goA, -penetration);
					else
					{
						//A��B���ϓ��Ɉړ�������
						ApplyPenetration(b, goB, penetration * 0.5f);
						ApplyPenetration(a, goA, penetration * -0.5f);
					}

				//�C�x���g�֐��̌Ăяo��
				goA->OnCollision(colA.origin, colB.origin);
				goB->OnCollision(colB.origin, colA.origin);

				//�C�x���g�̌��ʁA�ǂ��炩�̃Q�[���I�u�W�F�N�g���j�����ꂽ�烋�[�v�I��
				if (goA->IsDestroyed() || goB->IsDestroyed())
					return;	// �֐����I������
			} // if (funcList[typeA][typeB](colA.world, colB.world, penetration))
		} // for colB
	} // for colA
} // HandleGameObjectCollision

/// <summary>
/// �j���\��̃Q�[���I�u�W�F�N�g���폜����
/// </summary>
void Engine::RemoveDestroyedGameObject()
{
	if (gameObjects.empty())
		return;	// �Q�[���I�u�W�F�N�g�������Ă��Ȃ���Ή������Ȃ�

	// �j���\��̗L���ŃQ�[���I�u�W�F�N�g�𕪂���
	const auto iter =
		std::partition
		(
			gameObjects.begin(),
			gameObjects.end(),
			[](const GameObjectPtr& e) { return !e->IsDestroyed(); }
	);

	// �j���\��̃I�u�W�F�N�g��ʂ̔z��Ɉړ�
	GameObjectList destroyList
	(
		std::make_move_iterator(iter),
		std::make_move_iterator(gameObjects.end())
	);

	// �z�񂩂�ړ��ς݃I�u�W�F�N�g���폜
	gameObjects.erase(iter, gameObjects.end());

	// �j���\��̃I�u�W�F�N�g��OnDestroy�����s
	for (auto& e : destroyList)
		e->OnDestroy();

	// �����Ŏ��ۂɃQ�[���I�u�W�F�N�g���폜�����(destroyList�̎������I��邽��)
}

/// <summary>
/// �J�����ɋ߂����C�g��I���GPU�������ɃR�s�[����
/// </summary>
void Engine::UpdateShaderLight()
{
	const GLuint programs[] = { *prog3D, *progSkeletal };
	for (auto prog : programs)
	{
		// ������GPU�������ɃR�s�[
		glProgramUniform3fv(prog, locAmbientLight, 1, &ambientLight.x);

		// ���s�����̃p�����[�^��GPU�������ɃR�s�[
		const vec3 color = directionalLight.color * directionalLight.intensity;
		glProgramUniform3fv(prog, locDirectionalLightColor, 1, &color.x);
		glProgramUniform3fv(prog, locDirectionalLightDirection,
			1, &directionalLight.direction.x);
	} // for programs

	// �R�s�[���郉�C�g���Ȃ���΃��C�g����0�ɐݒ肷��
	if (usedLights.empty())
	{
		for (auto prog : programs)
			glProgramUniform1i(prog, locLightCount, 0);
		
		return;
	}

	// �g�p�����C�g�̔z�񂩂�A���g�p�ɂȂ������C�g�����O����
	const auto itrUnused = std::remove_if(usedLights.begin(), usedLights.end(),
		[&](int i) { return !lights[i].isUsed; });
	usedLights.erase(itrUnused, usedLights.end());

	// �d������ԍ�����������
	std::sort(usedLights.begin(), usedLights.end());
	auto itr = std::unique(usedLights.begin(), usedLights.end());
	usedLights.erase(itr, usedLights.end());

	// �J�����̐��ʃx�N�g�����v�Z����
	const vec3 front = { -sin(camera.rotation.y),0,-cos(camera.rotation.y) };

	// �J�������烉�C�g�܂ł̋������v�Z����
	struct DISTANCE
	{
		float distance;		 // �J�������烉�C�g�̔��a�܂ł̋���
		const LIGHT_DATA* p; // ���C�g�̃A�h���X
	};
	std::vector<DISTANCE> distanceList;
	distanceList.reserve(lights.size());
	for (auto index : usedLights)
	{
		const auto& light = lights[index];
		const vec3 v = light.position - camera.position;
		// �J�����̌��ɂ��郉�C�g�����O����
		if (dot(front, v) <= -light.radius)
			continue;
		
		const float d = length(v) - light.radius; // �J�������烉�C�g�̔��a�܂ł̋���
		distanceList.push_back({ d,&light });
	}

	// ��ʂɉe�����郉�C�g���Ȃ���΃��C�g����0�ɐݒ肷��
	if (distanceList.empty())
	{
		for (auto prog : programs)
			glProgramUniform1i(prog, locLightCount, 0);

		return;
	}

	// �J�����ɋ߂����C�g��D�悷��
	std::stable_sort(distanceList.begin(), distanceList.end(),
		[&](const auto& a, const auto& b) { return a.distance < b.distance; });

	// ���C�g�f�[�^��GPU�������ɃR�s�[
	const int lightCount = static_cast<int>(
		std::min(distanceList.size(), maxShaderLightCount)); // �R�s�[���郉�C�g��
	std::vector<vec4> colorAndFalloffAngle(lightCount);
	std::vector<vec4> positionAndRadius(lightCount);
	std::vector<vec4> directionAndConeAngle(lightCount);
	for (int i = 0; i < lightCount; ++i)
	{
		const LIGHT_DATA* p = distanceList[i].p;
		const vec3 color = p->color * p->intensity;
		colorAndFalloffAngle[i] =
		{
			color.x,color.y,color.z,p->falloffAngle
		};
		positionAndRadius[i] =
		{
			p->position.x,p->position.y,p->position.z,p->radius
		};
		directionAndConeAngle[i] =
		{
			p->direction.x,p->direction.y,p->direction.z,p->coneAngle
		};
	}
	for (auto prog : programs)
	{
		glProgramUniform4fv(prog, locLightColorAndFalloffAngle, lightCount, &colorAndFalloffAngle[0].x);
		glProgramUniform4fv(prog, locLightPositionAndRadius, lightCount, &positionAndRadius[0].x);
		glProgramUniform4fv(prog, locLightDirectionAndConeAngle, lightCount, &directionAndConeAngle[0].x);
		glProgramUniform1i(prog, locLightCount, lightCount);
	} // for program
}

/// <summary>
/// �X�J�C�X�t�B�A��`�悷��
/// </summary>
void Engine::DrawSkySphere()
{
	// �V�[���ɃX�J�C�X�t�B�A���ݒ肳��Ă��Ȃ��ꍇ�͕`�悵�Ȃ�
	if (!skySphere || !scene->material_skysphere)
		return;

	// �A�����b�g�V�F�[�_�ŕ`��
	glUseProgram(*progUnlit);
	glBindVertexArray(*meshBuffer->GetVAO());
	glDepthMask(GL_FALSE); // �[�x�o�b�t�@�ւ̏������݂��֎~����

	// �X�J�C�X�t�B�A���f���̔��a��0.5m�Ɖ��肵�A�`��͈͂�95%�̑傫���Ɋg�傷��
	static const float far = 1000; // �`��͈͂̍ő�l
	static const float scale = far * 0.95f / 0.5f;
	static const mat4 transformMatrix =
	{
	  vec4(scale, 0,    0,  0),
	  vec4(0, scale,    0,  0),
	  vec4(0,     0, scale, 0),
	  vec4(0,     0,    0,  1),
	};
	glProgramUniformMatrix4fv(*progUnlit,
		locTransformMatrix, 1, GL_FALSE, &transformMatrix[0].x);

	// �F�̓}�e���A���J���[�Œ�������̂ŃI�u�W�F�N�g�J���[�͔��ɐݒ肷��
	static const vec4 color = { 1, 1, 1, 1 };
	glProgramUniform4fv(*progUnlit, locColor, 1, &color.x);

	// �X�J�C�X�t�B�A�̓J���������_�Ƃ��ĕ`�悷��
	static const vec3 skySphereCameraPosition = { 0, 0, 0 };
	glProgramUniform3fv(*progUnlit, 4, 1, &skySphereCameraPosition.x);

	// �X�J�C�X�t�B�A��`�悷��
	const MaterialList materials(1, scene->material_skysphere);
	Draw(*skySphere, *progUnlit, materials);

	glProgramUniform3fv(*progUnlit, 4, 1, &camera.position.x); // �J�������W�����ɖ߂�
	glDepthMask(GL_TRUE);  // �[�x�o�b�t�@�ւ̏������݂�������
	glUseProgram(*prog3D); // 3D�`��p�V�F�[�_�ɖ߂�
}

/// <summary>
/// �f�v�X�V���h�E�}�b�v���쐬����
/// </summary>
void Engine::CreateShadowMap
(
	GameObjectList::iterator begin,
	GameObjectList::iterator end
)
{
	// �`���t���[���o�b�t�@��ύX����
	glBindFramebuffer(GL_FRAMEBUFFER, *fboShadow);

	// �r���[�|�[�g���t���[���o�b�t�@�̃T�C�Y�ɍ��킹��
	const auto& texShadow = fboShadow->GetTextureDepth();
	glViewport(0, 0, texShadow->GetWidth(), texShadow->GetHeight());

	glEnable(GL_DEPTH_TEST); // �[�x�e�X�g��L����
	glDisable(GL_BLEND);     // �����������𖳌���

	// �[�x�o�b�t�@���N���A
	glClear(GL_DEPTH_BUFFER_BIT);

	// VAO�ƃV�F�[�_��OpenGL�R���e�L�X�g�Ɋ��蓖�Ă�
	glBindVertexArray(*meshBuffer->GetVAO());
	glUseProgram(*progShadow);

	// �e�̕`��p�����[�^
	const float shadowAreaSize = 100;  // �e�̕`��͈�(XY����)
	const float shadowNearZ = 1;       // �e�̕`��͈�(��Z����)
	const float shadowFarZ = 200;      // �e�̕`��͈�(��Z����)
	const float shadowCenterZ = (shadowNearZ + shadowFarZ) * 0.5f; // �`��͈͂̒��S
	const vec3 target = { 0, 0, 50 }; // �J�����̒����_
	const vec3 eye =
		target - directionalLight.direction * shadowCenterZ; // �J�����̈ʒu

	// �r���[�v���W�F�N�V�����s����v�Z����
	const mat4 matShadowView = LookAt(eye, target, vec3(0, 1, 0));
	const mat4 matShadowProj = Orthogonal(
		-shadowAreaSize / 2, shadowAreaSize / 2,
		-shadowAreaSize / 2, shadowAreaSize / 2,
		shadowNearZ, shadowFarZ);
	const mat4 matShadow = matShadowProj * matShadowView;

	// �r���[�v���W�F�N�V�����s���GPU�������ɃR�s�[
	glProgramUniformMatrix4fv(*progShadow,
		locViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);
	glProgramUniformMatrix4fv(*progShadowSkeletal,
		locViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);

	// ���b�V����`�悷��
	std::vector<const GameObject*> skeletalMeshList;
	skeletalMeshList.reserve(end - begin);
	for (GameObjectList::iterator i = begin; i != end; ++i)
	{
		const auto& e = *i;

		if (!(int(e->type_lighting) & int(GameObject::TYPE_LIGHTING::SHADOW)))
			continue;

		// �X�P���^�����b�V�����L�^����
		if (e->renderer)
			skeletalMeshList.push_back(e.get());

		if (!e->staticMesh)
			continue;

		glProgramUniformMatrix4fv(*progShadow,
			locTransformMatrix, 1, GL_FALSE, &e->GetTransformMatrix()[0].x);
		if (e->materials.empty())
			Draw(*e->staticMesh, *progShadow, e->staticMesh->materials);
		else
			Draw(*e->staticMesh, *progShadow, e->materials);

	} // for

	 // �X�P���^�����b�V����`�悷��
	glUseProgram(*progShadowSkeletal);
	glBindVertexArray(*meshBuffer->GetVAOSkeletal());
	for (const GameObject* e : skeletalMeshList)
		e->renderer->Draw(*e, *progShadowSkeletal, *ssboJointMatrices[jointMatrixIndex], bpJointMatrices);

	// �[�x�e�N�X�`�������蓖�Ă�
	glBindTextureUnit(2, *texShadow);

	// �[�x�e�N�X�`�����W�ւ̕ϊ��s����쐬����
	static const mat4 matTexture = {
	  { 0.5f, 0.0f, 0.0f, 0.0f },
	  { 0.0f, 0.5f, 0.0f, 0.0f },
	  { 0.0f, 0.0f, 0.5f, 0.0f },
	  { 0.5f, 0.5f, 0.5f, 1.0f } };

	// �V���h�E�e�N�X�`���s���GPU�������ɃR�s�[
	const mat4 matShadowTexture = matTexture * matShadowProj * matShadowView;
	glProgramUniformMatrix4fv(*prog3D,
		locShadowTextureMatrix, 1, GL_FALSE, &matShadowTexture[0].x);

	// �@�������̕␳�l��ݒ肷��
	const float texelSize = shadowAreaSize / static_cast<float>(texShadow->GetWidth());
	glProgramUniform1f(*prog3D, locShadowNormalOffset, texelSize);
}

/// <summary>
/// �A���r�G���g�I�N���[�W������`�悷��
/// </summary>
void Engine::DrawAmbientOcclusion()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glBindVertexArray(*meshBuffer->GetVAO());
	const auto& drawParams =
		meshBuffer->GetStaticMesh("plane_xy")->drawParamsList[0];

	// �[�x�l����`�����ɕϊ�(fboMain -> fboSAODepth[0])
	glUseProgram(*progSAORestoreDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboSAODepth[0]);
	glViewport(0, 0, fboSAODepth[0]->GetWidth(), fboSAODepth[0]->GetHeight());
	GLuint tex0 = *fboMain->GetTextureDepth();
	glBindTextures(0, 1, &tex0);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// �k���o�b�t�@���쐬(fboSAODepth[i - 1] -> fboSAODepth[i])
	glUseProgram(*progSAODownSampling);
	tex0 = *fboSAODepth[0]->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	for (int i = 1; i < std::size(fboSAODepth); ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *fboSAODepth[i]);
		glViewport(0, 0, fboSAODepth[i]->GetWidth(), fboSAODepth[i]->GetHeight());
		glProgramUniform1i(*progSAODownSampling, 200, i - 1);
		glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
			GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);
	}

	// SAO����p�p�����[�^��ݒ肷��
	const float radius = 1.0f; // ���[���h���W�n�ɂ�����AO�̃T���v�����O���a(�P��=m)
	const float bias = 0.012f; // ���ʂƂ݂Ȃ��Ė�������p�x�̃R�T�C��
	const float intensity = 5; // AO���ʂ̋���
	glProgramUniform4f(*progSAO, 200, radius * radius,
		0.5f * radius * GetFovScale(), bias, intensity / pow(radius, 6.0f));

	// NDC���W�����_���W�ɕϊ�����p�����[�^��ݒ肷��
	const float w = static_cast<float>(fboSAODepth[0]->GetWidth());
	const float h = static_cast<float>(fboSAODepth[0]->GetHeight());
	const float aspectRatio = w / h;
	const float invFovScale = 1.0f / GetFovScale();
	glProgramUniform2f(*progSAO, 201, invFovScale * aspectRatio, invFovScale);

	// SAO���v�Z����
	glUseProgram(*progSAO);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboSAO);
	tex0 = *fboSAODepth[0]->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// SAO�̌��ʂ��ڂ���
	glUseProgram(*progSAOBlur);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboSAOBlur);
	tex0 = *fboSAO->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	GLuint tex1 = *fboSAODepth[0]->GetTextureColor();
	glBindTextures(1, 1, &tex1);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// �A���r�G���g�I�N���[�W����������
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(*progSimple);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMain);
	glViewport(0, 0, fboMain->GetWidth(), fboMain->GetHeight());
	tex0 = *fboSAOBlur->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

/// <summary>
/// �u���[���G�t�F�N�g��`�悷��
/// </summary>
void Engine::DrawBloomEffect()
{
	// �V�F�[�_���ʂ̐ݒ�
	const int bufferCount = static_cast<int>(fboBloom.size());
	const auto& drawParams =
		meshBuffer->GetStaticMesh("plane_xy")->drawParamsList[0];
	glBindVertexArray(*meshBuffer->GetVAO());
	glDisable(GL_DEPTH_TEST);

	// ���邢�����𒊏o����
	glDisable(GL_BLEND);
	glUseProgram(*progHighPassFilter);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboBloom[0]);
	glViewport(0, 0, fboBloom[0]->GetWidth(), fboBloom[0]->GetHeight());
	const GLuint tex0 = *fboMain->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	glProgramUniform2f(*progHighPassFilter,
		locHighPassFilter, bloomThreshold, bloomIntensity);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// �k���ڂ���
	glUseProgram(*progDownSampling);
	for (int i = 1; i < bufferCount; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *fboBloom[i]);
		glViewport(0, 0, fboBloom[i]->GetWidth(), fboBloom[i]->GetHeight());
		const GLuint tex = *fboBloom[i - 1]->GetTextureColor();
		glBindTextures(0, 1, &tex);
		glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
			GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);
	}

	// �g��ڂ���
	glEnable(GL_BLEND); // ������������L����
	glBlendFunc(GL_ONE, GL_ONE); // 1:1�̔䗦�ŉ��Z����
	glUseProgram(*progUpSampling);
	for (int i = bufferCount - 2; i >= 0; --i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *fboBloom[i]);
		glViewport(0, 0, fboBloom[i]->GetWidth(), fboBloom[i]->GetHeight());
		const GLuint tex = *fboBloom[i + 1]->GetTextureColor();
		glBindTextures(0, 1, &tex);
		glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
			GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);
	}

	// ���C��FBO�Ƀu���[���G�t�F�N�g�𔽉f����
	glBlendFunc(GL_CONSTANT_COLOR, GL_ONE); // �萔:1�̔䗦�ŉ��Z����
	const float c = 1.0f / static_cast<float>(bufferCount); // �萔
	glBlendColor(c, c, c, c); // �萔��ݒ肷��
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMain);
	glViewport(0, 0, fboMain->GetWidth(), fboMain->GetHeight());
	const GLuint tex1 = *fboBloom[0]->GetTextureColor();
	glBindTextures(0, 1, &tex1);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// �u�����h�������ɖ߂�
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


/// <summary>
/// 3D�`��̌��ʂ��摜�������ăf�t�H���g�t���[���o�b�t�@�ɏ�������
/// </summary>
void Engine::DrawMainImageToDefaultFramebuffer()
{
	// �t���[���o�b�t�@�̓��e����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST); // �[�x�e�X�g�𖳌���
	glDisable(GL_BLEND);	  // �����������𖳌���

	glUseProgram(*progImageProcessing);
	glBindVertexArray(*meshBuffer->GetVAO());

	// OpenGL�R���e�L�X�g�Ƀe�N�X�`����ݒ肷��
	const GLuint tex[] = { *fboMain->GetTextureColor() };
	glBindTextures(0, GLsizei(std::size(tex)), tex);

	//// �O���[�X�P�[���ɂ���F�ϊ��s���ݒ�
	//colorMatrix = {
	//{ 0.2126f, 0.2126f, 0.2126f },
	//{ 0.7152f, 0.7152f, 0.7152f },
	//{ 0.0722f, 0.0722f, 0.0722f }
	// };

	//// �Z�s�A���ɂ���F�ϊ��s���ݒ�
	//colorMatrix = {
	//{ 0.346f, 0.278f, 0.196f },
	//{ 0.678f, 0.546f, 0.385f },
	//{ 0.151f, 0.124f, 0.089f },
	//};

	// �F�ϊ��s���GPU�������ɃR�s�[
	glProgramUniformMatrix3fv(*progImageProcessing,
		locColorMatrix, 1, GL_FALSE, &colorMatrix[0].x);

	//// ���ˏ�u���[�̃p�����[�^��GPU�������ɃR�s�[
	//glProgramUniform4f(*progImageProcessing, locRadialBlur,
	//    radialBlurCenter.x, radialBlurCenter.y, radialBlurLength, radialBlurStart);

	// ��ʑS�̂Ɏl�p�`��`�悷��
	const auto& drawParams =
		meshBuffer->GetStaticMesh("plane_xy")->drawParamsList[0];
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	glEnable(GL_BLEND); // ������������L����
}
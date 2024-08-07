/**
* @file Scene.h
*/
#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED
#include <memory>

// ��s�錾
class Engine;
struct MATERIAL;
using MaterialPtr = std::shared_ptr<MATERIAL>;

/// <summary>
/// �V�[���̊��N���X
/// </summary>
class Scene
{
public:
	// �f�t�H���g�R���X�g���N�^
	inline Scene() = default;
	// �f�t�H���g�f�X�g���N�^
	inline virtual ~Scene() = default;

	// ���ƂȂ�Initialize
	inline virtual bool Initialize(Engine& engine) { return true; }
	// ���ƂȂ�Update
	inline virtual void Update(Engine& engine,float deltaTime){}
	// ���ƂȂ�Finalize
	inline virtual void Finalize(Engine& engine){}

	MaterialPtr material_skysphere; // �X�J�C�X�t�B�A�p�̃}�e���A��
};

using ScenePtr = std::shared_ptr<Scene>; // �V�[���|�C���^�^

#endif // !SCENE_H_INCLUDED
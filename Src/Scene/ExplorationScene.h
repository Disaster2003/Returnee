/**
* @file ExplorationScene.h
*/
#ifndef EXPLORATIONSCENE_H_INCLUDED
#define EXPLORATIONSCENE_H_INCLUDED
#include "../Engine/Scene.h"

// ��s�錾
class PlayerComponent;
class Dragon;

/// <summary>
/// �t�B�[���h�T���V�[��
/// </summary>
class ExplorationScene
	: public Scene
{
public:
	// �f�t�H���g�R���X�g���N�^
	ExplorationScene() = default;
	// �f�t�H���g�f�X�g���N�^
	virtual ~ExplorationScene() = default;

	// �V�[��������������
	virtual bool Initialize(Engine& engine) override;
	// ��ɃQ�[���I�[�o�[�ɂȂ��ĂȂ����Ď�����
	virtual void Update(Engine& engine, float deltaTime)override;
	// �V�[�����I������
	virtual void Finalize(Engine& engine) override;

private:
	std::shared_ptr<PlayerComponent> playerComponent; // �v���C���[
	std::shared_ptr<Dragon> boss;					  // �{�X
};

#endif // EXPLORATIONSCENE_H_INCLUDED
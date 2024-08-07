/**
* @file ProgramObject.h
*/
#ifndef PROGRAMPIPELINEOBJECT_H_INCLUDED
#define PROGRAMPIPELINEOBJECT_H_INCLUDED
#include "glad/glad.h"
#include <string>
#include <memory>

// ��s�錾
class ProgramObject;
using ProgramObjectPtr = std::shared_ptr<ProgramObject>;

/// <summary>
/// �v���O�����E�I�u�W�F�N�g���Ǘ�����N���X
/// </summary>
class ProgramObject
{
public:
	// �R���X�g���N�^
	ProgramObject(const char* filenameVS, const char* filenameFS);
	// �f�X�g���N�^
	virtual ~ProgramObject();

	// �R�s�[�Ƒ�����֎~����
	ProgramObject(const ProgramObject&) = delete;
	ProgramObject& operator=(const ProgramObject&) = delete;

	// �Ǘ��ԍ����擾����
	operator GLuint() const { return prog; }

	// ���P�[�V�����ԍ����擾����
	GLint ColorLocation() const { return locColor; }
	GLint RoughnessAndMetallicLocation() const { return locRoughnessAndMetallic; }

private:
	GLuint vs = 0;          // ���_�V�F�[�_
	GLuint fs = 0;          // �t���O�����g�V�F�[�_
	GLuint prog = 0;        // �v���O�����I�u�W�F�N�g
	std::string filenameVS; // ���_�V�F�[�_�t�@�C����
	std::string filenameFS; // �t���O�����g�V�F�[�_�t�@�C����
	GLint locColor = -1;
	GLint locRoughnessAndMetallic = -1;
};

#endif // PROGRAMPIPELINEOBJECT_H_INCLUDED
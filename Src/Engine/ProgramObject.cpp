/**
* @file ProgramObject.cpp
*/
#include "ProgramObject.h"
#include "Debug.h"
#include <fstream>
#include <filesystem>

/// <summary>
/// �V�F�[�_�t�@�C����ǂݍ���ŃR���p�C������
/// </summary>
/// <param name="type">�V�F�[�_�̎��</param>
/// <param name="filename">�V�F�[�_�t�@�C����</param>
/// <returns>�V�F�[�_�̊Ǘ��ԍ�</returns>
GLuint CompileShader
(
	GLenum type,
	const char* filename
)
{
	std::ifstream file(filename, std::ios::binary);	//�t�@�C�����J��
	if (!file)
	{
		LOG_ERROR("%s���J���܂���", filename);
		return 0;
	}

	// �t�@�C����ǂݍ���
	const size_t filesize = std::filesystem::file_size(filename);
	std::vector<char> buffer(filesize);
	file.read(buffer.data(), filesize);
	file.close();

	// �\�[�X�R�[�h��ݒ肵�ăR���p�C��
	const char* source[] = { buffer.data() };
	const GLint length[] = { int(buffer.size()) };
	const GLuint object = glCreateShader(type);
	glShaderSource(object, 1, source, length);
	glCompileShader(object);

	// �R���p�C�����ʂ��`�F�b�N
	GLint status = 0;
	glGetShaderiv(object, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		LOG_ERROR("�V�F�[�_�̃R���p�C���Ɏ��s(%s)", filename);
		glDeleteShader(object);
		return 0;
	}

	return object;
}

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="filenameVS">�o�[�e�b�N�X�V�F�[�_�̃t�@�C����</param>
/// <param name="filenameFS">�t���O�����g�V�F�[�_�̃t�@�C����</param>
ProgramObject::ProgramObject
(
	const char* filenameVS,
	const char* filenameFS
) : filenameVS(filenameVS)
  , filenameFS(filenameFS)
{
	// �V�F�[�_��ǂݍ���ŃR���p�C��
	vs = CompileShader(GL_VERTEX_SHADER, filenameVS);
	fs = CompileShader(GL_FRAGMENT_SHADER, filenameFS);

	if (vs == 0 || fs == 0)
	{
		return;
	}

	// 2�̃V�F�[�_�������N
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	// �����N���ʂ��`�F�b�N
	GLint status = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		LOG_ERROR("�V�F�[�_�̃����N�Ɏ��s(vs=%s, fs=%s)", filenameVS, filenameFS);
		return;
	}
	// ���P�[�V�����ԍ����擾
	locColor = glGetUniformLocation(prog, "color");
	locRoughnessAndMetallic = glGetUniformLocation(prog, "roughnessAndMetallic");

	LOG("�V�F�[�_���쐬(vs=%s, fs=%s)", filenameVS, filenameFS);
}

/// <summary>
/// �f�X�g���N�^
/// </summary>
ProgramObject::~ProgramObject()
{
	glDeleteProgram(prog);
	glDeleteShader(fs);
	glDeleteShader(vs);
}
/**
* @file Debug.h
*/
#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

// �f�o�b�N�p�̋@�\���i�[���閼�O���
namespace Debug
{
	// ���b�Z�[�W�̎��
	enum class TYPE
	{
		ERROR,	 // �G���[
		WARNING, // �x��
		INFO,	 // ���̑��̏��
	};

	void Log(TYPE type, const char* func, const char* format, ...);
}// namespace Debug

#ifndef NDEBUG
#   define ENABLE_DEBUG_LOG // ���̃}�N�����`����ƃf�o�b�O�o�͂��L���ɂȂ�
#endif // NDEBUG

// �f�o�b�O�o�̓}�N��
#ifdef ENABLE_DEBUG_LOG
#   define LOG_ERROR(...)   Debug::Log(Debug::TYPE::ERROR, __func__, __VA_ARGS__)
#   define LOG_WARNING(...) Debug::Log(Debug::TYPE::WARNING, __func__, __VA_ARGS__)
#   define LOG(...)         Debug::Log(Debug::TYPE::INFO, __func__, __VA_ARGS__)
#else
#   define LOG_ERROR(...)   ((void)0)
#   define LOG_WARNING(...) ((void)0)
#   define LOG(...)         ((void)0)
#endif // !ENABLE_DEBUG_LOG

#endif // !DEBUG_H_INCLUDED
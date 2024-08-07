/**
* @file Debug.h
*/
#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

// デバック用の機能を格納する名前空間
namespace Debug
{
	// メッセージの種類
	enum class TYPE
	{
		ERROR,	 // エラー
		WARNING, // 警告
		INFO,	 // その他の情報
	};

	void Log(TYPE type, const char* func, const char* format, ...);
}// namespace Debug

#ifndef NDEBUG
#   define ENABLE_DEBUG_LOG // このマクロを定義するとデバッグ出力が有効になる
#endif // NDEBUG

// デバッグ出力マクロ
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
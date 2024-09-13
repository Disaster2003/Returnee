/**
* @file ProgramObject.cpp
*/
#include "ProgramObject.h"
#include "Debug.h"
#include <fstream>
#include <filesystem>

/// <summary>
/// シェーダファイルを読み込んでコンパイルする
/// </summary>
/// <param name="type">シェーダの種類</param>
/// <param name="filename">シェーダファイル名</param>
/// <returns>シェーダの管理番号</returns>
GLuint CompileShader
(
	GLenum type,
	const char* filename
)
{
	std::ifstream file(filename, std::ios::binary);	//ファイルを開く
	if (!file)
	{
		LOG_ERROR("%sを開けません", filename);
		return 0;
	}

	// ファイルを読み込む
	const size_t filesize = std::filesystem::file_size(filename);
	std::vector<char> buffer(filesize);
	file.read(buffer.data(), filesize);
	file.close();

	// ソースコードを設定してコンパイル
	const char* source[] = { buffer.data() };
	const GLint length[] = { int(buffer.size()) };
	const GLuint object = glCreateShader(type);
	glShaderSource(object, 1, source, length);
	glCompileShader(object);

	// コンパイル結果をチェック
	GLint status = 0;
	glGetShaderiv(object, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		LOG_ERROR("シェーダのコンパイルに失敗(%s)", filename);
		glDeleteShader(object);
		return 0;
	}

	return object;
}

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="filenameVS">バーテックスシェーダのファイル名</param>
/// <param name="filenameFS">フラグメントシェーダのファイル名</param>
ProgramObject::ProgramObject
(
	const char* filenameVS,
	const char* filenameFS
) : filenameVS(filenameVS)
  , filenameFS(filenameFS)
{
	// シェーダを読み込んでコンパイル
	vs = CompileShader(GL_VERTEX_SHADER, filenameVS);
	fs = CompileShader(GL_FRAGMENT_SHADER, filenameFS);

	if (vs == 0 || fs == 0)
	{
		return;
	}

	// 2つのシェーダをリンク
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	// リンク結果をチェック
	GLint status = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		LOG_ERROR("シェーダのリンクに失敗(vs=%s, fs=%s)", filenameVS, filenameFS);
		return;
	}
	// ロケーション番号を取得
	locColor = glGetUniformLocation(prog, "color");
	locRoughnessAndMetallic = glGetUniformLocation(prog, "roughnessAndMetallic");

	LOG("シェーダを作成(vs=%s, fs=%s)", filenameVS, filenameFS);
}

/// <summary>
/// デストラクタ
/// </summary>
ProgramObject::~ProgramObject()
{
	glDeleteProgram(prog);
	glDeleteShader(fs);
	glDeleteShader(vs);
}
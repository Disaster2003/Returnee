/**
* @file ProgramObject.h
*/
#ifndef PROGRAMPIPELINEOBJECT_H_INCLUDED
#define PROGRAMPIPELINEOBJECT_H_INCLUDED
#include "glad/glad.h"
#include <string>
#include <memory>

// 先行宣言
class ProgramObject;
using ProgramObjectPtr = std::shared_ptr<ProgramObject>;

/// <summary>
/// プログラム・オブジェクトを管理するクラス
/// </summary>
class ProgramObject
{
public:
	// コンストラクタ
	ProgramObject(const char* filenameVS, const char* filenameFS);
	// デストラクタ
	virtual ~ProgramObject();

	// コピーと代入を禁止する
	ProgramObject(const ProgramObject&) = delete;
	ProgramObject& operator=(const ProgramObject&) = delete;

	// 管理番号を取得する
	operator GLuint() const { return prog; }

	// ロケーション番号を取得する
	GLint ColorLocation() const { return locColor; }
	GLint RoughnessAndMetallicLocation() const { return locRoughnessAndMetallic; }

private:
	GLuint vs = 0;          // 頂点シェーダ
	GLuint fs = 0;          // フラグメントシェーダ
	GLuint prog = 0;        // プログラムオブジェクト
	std::string filenameVS; // 頂点シェーダファイル名
	std::string filenameFS; // フラグメントシェーダファイル名
	GLint locColor = -1;
	GLint locRoughnessAndMetallic = -1;
};

#endif // PROGRAMPIPELINEOBJECT_H_INCLUDED
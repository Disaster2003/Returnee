/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "dds.h"
#include "Debug.h"
#include <vector>
#include <fstream>
#include <filesystem>

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="filename">テクスチャファイル名</param>
Texture::Texture(const char* filename)
{
	// 拡張子がddsの場合はDDSファイルとみなす
	const size_t n = strlen(filename);
	if (n >= 4 && filename[n - 4] == '.' &&
		tolower(filename[n - 3]) == 'd' &&
		tolower(filename[n - 2]) == 'd' &&
		tolower(filename[n - 1]) == 's')
	{
		id = LoadDDS(filename);
		if (id)
		{
			glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
			glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
			name = filename;
			LOG("%sを読み込みました", filename);
		}
		return;
	}

	std::ifstream file(filename, std::ios::binary);
	if (!file)
	{
		LOG_ERROR("%sを開けません", filename);
		return;
	}

	// ファイルを読み込む
	const size_t filesize = std::filesystem::file_size(filename);
	std::vector<uint8_t> buffer(filesize);
	file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
	file.close();

	// ヘッダから情報を取得
	const size_t tgaHeaderSize = 18;				   // ヘッダ情報のバイト数
	const int imageOffset = tgaHeaderSize + buffer[0]; // 画像データの位置
	const uint8_t imageType = buffer[2];               // 画像形式
	const int pixelBytes = (buffer[16] + 7) / 8;       // 1ピクセルのバイト数
	width = buffer[12] + buffer[13] * 256;
	height = buffer[14] + buffer[15] * 256;

	// 圧縮形式の場合は展開する
	//  0(0b0000) : 画像なし
	//  1(0b0001) : インデックス(無圧縮)
	//  2(0b0010) : トゥルーカラー(無圧縮)
	//  3(0b0011) : 白黒(無圧縮)
	//  9(0b1001) : インデックス(RLE)
	// 10(0b1010) : トゥルーカラー(RLE)
	// 11(0b1011) : 白黒(RLE)
	if (imageType & 0b1000)
	{
		// 展開用のバッファを用意
		const int imageBytes = width * height * pixelBytes; // 展開後のバイト数
		std::vector<uint8_t> tmp(imageOffset + imageBytes);

		// swapに備えて、画像データより前の部分をコピー
		std::copy_n(buffer.begin(), imageOffset, tmp.begin());

		// 画像データを展開
		const uint8_t* src = buffer.data() + imageOffset; // 圧縮データのアドレス
		uint8_t* dest = tmp.data() + imageOffset;         // データ展開先アドレス
		const uint8_t* const destEnd = dest + imageBytes; // 展開終了アドレス

		while (dest != destEnd)
		{
			// パケットヘッダからIDとデータ数を取得
			const int isRLE = *src & 0x80;
			const int count = (*src & 0x7f) + 1;
			++src; // アドレスをパケットデータの位置に進める

			if (isRLE)
			{
				// 圧縮データの場合、パケットデータを指定回数コピー
				for (int a = 0; a < count; ++a)
				{
					memcpy(dest, src, pixelBytes);
					dest += pixelBytes;
				}
				src += pixelBytes;
			}
			else
			{
				// 無圧縮データの場合、パケットデータ全体をコピー
				const int dataBytes = pixelBytes * count;
				memcpy(dest, src, dataBytes);
				dest += dataBytes;
				src += dataBytes;
			}
		} // while
		// bufferとtmpの内容を交換
		buffer.swap(tmp);
	}

	// 格納方向が「上から下」の場合、データを上下反転
	const bool topToBottom = buffer[17] & 0b0010'0000;
	if (topToBottom)
	{
		const int pixelDepth = buffer[16];					 // 1ピクセルのビット数
		const int lineByteSize = width * pixelDepth / 8;	 // 1行のバイト数
		uint8_t* top = buffer.data() + tgaHeaderSize;		 // 上の行の位置
		uint8_t* bottom = top + lineByteSize * (height - 1); // 下の行の位置
		std::vector<uint8_t>tmp(lineByteSize);				 // 上下入れ替え用のバッファ

		// 行単位で上下反転
		// 上下の行の位置が逆転するまで繰り返す
		while (top < bottom)
		{
			std::copy_n(top, lineByteSize, tmp.data());		// 「上の行のコピー」を作る
			std::copy_n(bottom, lineByteSize, top);			// 下の行を、上の行に上書き
			std::copy_n(tmp.data(), lineByteSize, bottom);	// 「上の行のコピー」を下の行に上書き
			top += lineByteSize;							// 上の行の位置を1行下に移動
			bottom -= lineByteSize;							// 下の行の位置を1行上に移動
		}
	} // if topToBottom

	  // 1ピクセルのビット数に対応する形式のセットを検索
	struct FORMAT
	{
		int pixelDepth;     // TGAの1ピクセルのビット数
		GLenum imageFormat; // TGAのデータ形式
		GLenum imageType;   // TGAのデータ型
		GLenum gpuFormat;   // GPU側のデータ形式
	};
	constexpr FORMAT formatList[] =
	{
	  { 32, GL_BGRA, GL_UNSIGNED_BYTE,              GL_RGBA8 },
	  { 24, GL_BGR,  GL_UNSIGNED_BYTE,              GL_RGB8 },
	  { 16, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_RGB5_A1 },
	  { 15, GL_BGR,  GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_RGB5 },
	  {  8, GL_RED,  GL_UNSIGNED_BYTE,              GL_R8 },
	};

	const FORMAT* format = std::find_if(formatList, std::end(formatList) - 1,
		[pixelDepth = buffer[16]](const FORMAT& e) { return e.pixelDepth == pixelDepth; });

	// 現在のアラインメントを記録
	GLint alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

	// 画像のアラインメントを決定
	// 1行のバイト数が4で割り切れるときは4、2で割り切れるときは2、それ以外は1に設定
	constexpr int alignmentList[] = { 4, 1, 2, 1 };
	const int lineByteSize = width * pixelBytes; // 1行のバイト数
	const int imageAlignment = alignmentList[lineByteSize % 4];

	// アラインメントを変更
	if (alignment != imageAlignment)
		glPixelStorei(GL_UNPACK_ALIGNMENT, imageAlignment);

	// テクスチャを作成
	GLuint object = 0; // テクスチャの管理番号
	glCreateTextures(GL_TEXTURE_2D, 1, &object);
	glTextureStorage2D(object, 1, format->gpuFormat, width, height);
	glTextureSubImage2D(object, 0, 0, 0, width, height,
		format->imageFormat, format->imageType, buffer.data() + tgaHeaderSize);

	id = object;
	name = filename;

	// アラインメントを元に戻す
	if (alignment != imageAlignment)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	}

	// グレースケールテクスチャの場合、赤成分を緑と青にコピーするように設定する
	if (format->imageFormat == GL_RED)
	{
		glTextureParameteri(object, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTextureParameteri(object, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTextureParameteri(object, GL_TEXTURE_SWIZZLE_B, GL_RED);
	}
}

/// <summary>
/// キューブマップを作成するコンストラクタ
/// </summary>
/// <param name="name">テクスチャ識別用の名前</param>
/// <param name="cubemap_filenames">キューブマップを構成する6枚の画像ファイル名</param>
Texture::Texture
(
	const char* name,
	const char* const cubemap_filenames[6]
)
{
	// 画像ファイルを読み込む
	Texture t[6] =
	{
	  Texture(cubemap_filenames[0]),
	  Texture(cubemap_filenames[1]),
	  Texture(cubemap_filenames[2]),
	  Texture(cubemap_filenames[3]),
	  Texture(cubemap_filenames[4]),
	  Texture(cubemap_filenames[5]),
	};

	// 1枚でもテクスチャの読み込みに失敗していたら作成しない
	for (const auto& e : t)
	{
		if (!e)
		{
			LOG_ERROR("キューブマップ%sの画像の読み込みに失敗", name);
			return; // 6枚そろっていないと作成できない
		}
	}

	// 画像サイズを取得する
	const int w = t[0].GetWidth();
	const int h = t[0].GetHeight();
	if (w != h)
	{
		LOG_ERROR("キューブマップ%sが正方形ではありません(%dx%d)", name, w, h);
		return; // 縦と横が同じサイズでないと作成できない
	}
	for (const auto& e : t)
	{
		if (e.GetWidth() != w || e.GetHeight() != h)
		{
			LOG_ERROR("キューブマップ%sの画像サイズが一致しません", name);
			return; // すべてのサイズが等しくないと作成できない
		}
	}

	// 画像形式を取得する
	GLint gpuFormat;
	glGetTextureLevelParameteriv(t[0], 0, GL_TEXTURE_INTERNAL_FORMAT, &gpuFormat);
	for (const auto& e : t)
	{
		GLint tmp;
		glGetTextureLevelParameteriv(e, 0, GL_TEXTURE_INTERNAL_FORMAT, &tmp);
		if (tmp != gpuFormat)
		{
			LOG_ERROR("キューブマップ%sの画像形式が一致しません", name);
			return; // すべての画像形式が一致しないと作成できない
		}
	} // for t

	// 画像サイズからミップマップ数を計算する
	const int levels = static_cast<int>(log2(w)) + 1;

	// キューブマップを作成する
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);
	glTextureStorage2D(id, levels, gpuFormat, w, h);
	for (int i = 0; i < 6; ++i)
	{
		glCopyImageSubData(t[i], GL_TEXTURE_2D, 0, 0, 0, 0,
			id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, i, w, h, 1);
	}

	// ミップマップを生成する
	glGenerateTextureMipmap(id);

	// テクスチャパラメータを設定する
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

/// <summary>
/// 空のテクスチャを作成するコンストラクタ
/// </summary>
/// <param name="name">テクスチャ識別用の名前</param>
/// <param name="width">テクスチャの幅(ピクセル数)</param>
/// <param name="height">テクスチャの高さ(ピクセル数)</param>
/// <param name="format_gpu">データ形式</param>
/// <param name="mode_wrap">ラップモード</param>
/// <param name="levels">ミップマップテクスチャのレベル数</param>
Texture::Texture
(
	const char* name,
	int width,
	int height,
	GLenum format_gpu,
	GLenum mode_wrap,
	int levels
) : name(name), width(width), height(height)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &id);
	glTextureStorage2D(id, levels, format_gpu, width, height);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, mode_wrap);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, mode_wrap);

	// 深度テクスチャの場合、シャドウマッピングのために比較モードを設定する
	switch (format_gpu)
	{
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH_COMPONENT32:
	case GL_DEPTH_COMPONENT32F:
		glTextureParameteri(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		break;
	default:
		break;
	}
}

/// <summary>
/// デストラクタ
/// </summary>
Texture::~Texture()
{
	glDeleteTextures(1,&id);
}
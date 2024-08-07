/**
* @file dds.cpp
*/
#include "dds.h"
#include "Debug.h"
#include <filesystem>
#include <fstream>
#include <memory>

// GL_EXT_texture_compression_s3tc
constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;
constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;

namespace
{
	/// <summary>
	/// バイト列から数値を復元する
	/// </summary>
	/// <param name="buf">バイト列へのポインタ</param>
	/// <returns>復元した数値</returns>
	uint32_t Get(const char* buf)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
		return p[0] + (p[1] << 8UL) + (p[2] << 16UL) + (p[3] << 24UL);
	}
	
	/// <summary>
	/// FOURCCを作成する
	/// </summary>
	/// <param name="a">FOURCCの1文字目</param>
	/// <param name="b">FOURCCの2文字目</param>
	/// <param name="c">FOURCCの3文字目</param>
	/// <param name="d">FOURCCの4文字目</param>
	/// <returns>a, b, c, dから計算したFOURCC</returns>
	constexpr uint32_t MakeFourCC
	(
		char a,
		char b,
		char c,
		char d
	)
	{
		return a + (b << 8UL) + (c << 16UL) + (d << 24UL);
	}

	/// <summary>
	/// バッファからピクセル形式を読み出す
	/// </summary>
	/// <param name="buf">読み出し元バッファ</param>
	/// <returns>ピクセル形式を表すDDS_PIXEL_FORMAT構造体</returns>
	DDS_PIXEL_FORMAT ReadDDSPixelFormat(const char* buf)
	{
		DDS_PIXEL_FORMAT tmp = {};
		tmp.size = Get(buf + 0);
		tmp.flags = Get(buf + 4);
		tmp.fourCC = Get(buf + 8);
		tmp.rgbBitCount = Get(buf + 12);
		tmp.redBitMask = Get(buf + 16);
		tmp.greenBitMask = Get(buf + 20);
		tmp.blueBitMask = Get(buf + 24);
		tmp.alphaBitMask = Get(buf + 28);
		return tmp;
	}

	/// <summary>
	/// バッファからDDSヘッダを読み出す
	/// </summary>
	/// <param name="buf">読み出し元バッファ</param>
	/// <returns>DDSヘッダを表すDDS_HEADER構造体</returns>
	DDS_HEADER ReadDDSHeader(const char* buf)
	{
		DDS_HEADER tmp = {};
		tmp.size = Get(buf + 0);
		tmp.flags = Get(buf + 4);
		tmp.height = Get(buf + 8);
		tmp.width = Get(buf + 12);
		tmp.pitchOrLinearSize = Get(buf + 16);
		tmp.depth = Get(buf + 20);
		tmp.mipmapCount = Get(buf + 24);
		for (int i = 0; i < 11; ++i)
			tmp.reserved1[i] = Get(buf + 28 + i * 4);
		tmp.ddspf = ReadDDSPixelFormat(buf + 72);
		for (int i = 0; i < 4; ++i)
			tmp.caps[i] = Get(buf + 104 + i * 4);
		tmp.reserved2 = Get(buf + 120);
		return tmp;
	}

	/// <summary>
	/// バッファからDX10拡張ヘッダ画像情報を読み出す
	/// </summary>
	/// <param name="buf">読み出し元バッファ</param>
	/// <returns>DX10拡張ヘッダを表すDDSHeaderDXT10構造体</returns>
	DDS_HEADER_DXT10 ReadDX10Hader(const char* buf)
	{
		DDS_HEADER_DXT10 tmp = {};
		tmp.dxgiFormat = Get(buf + 0);
		tmp.resourceDimension = Get(buf + 4);
		tmp.miscFlag = Get(buf + 8);
		tmp.arraySize = Get(buf + 12);
		tmp.miscFlags2 = Get(buf + 16);
		return tmp;
	}
} // unnamed namespace

/// <summary>
/// DDSファイルからテクスチャを作成する
/// </summary>
/// <param name="filename">DDSファイル名</param>
/// <returns>0 = 作成失敗、0以外 = 作成したテクスチャの管理番号</returns>
GLuint LoadDDS(const char* filename)
{
	// ファイルを開く
	std::ifstream ifs(filename, std::ios_base::binary);
	if (!ifs)
	{
		LOG_ERROR("%sを開けません", filename);
		return 0; // 作成失敗
	}

	// ファイルの先頭部分を読み込む
	char buf[128];
	ifs.read(buf, 128);
	if (buf[0] != 'D' || buf[1] != 'D' || buf[2] != 'S' || buf[3] != ' ')
	{
		LOG_ERROR("%sはDDSファイルではありません", filename);
		return 0; // 作成失敗
	}

	// DDSヘッダを取り出す
	const DDS_HEADER header = ReadDDSHeader(buf + 4);
	if (header.size != 124)
	{
		LOG_ERROR("%sは未対応のDDSファイルです", filename);
		return 0; // 作成失敗
	}

	// DDSピクセル形式に対応するOpenGLピクセル形式を選択する
	GLenum imageFormat;      // 画像データのピクセル形式
	GLenum gpuFormat;        // GPU側のピクセル形式
	uint32_t blockSize = 16; // 1ブロックのバイト数
	if (header.ddspf.flags & DDPF_FOURCC)
	{
		// ピクセル形式がFourCCで定義されている場合
		constexpr struct
		{
			uint32_t fourCC;
			uint32_t blockSize;
			GLenum imageFormat;
		}
		formatList[] =
		{
		  { MakeFourCC('D', 'X', 'T', '1'),  8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT },
		  { MakeFourCC('D', 'X', 'T', '3'), 16, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT },
		  { MakeFourCC('D', 'X', 'T', '5'), 16, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT },
		  { MakeFourCC('B','C','5','U'), 16, GL_COMPRESSED_RG_RGTC2 },
		};
		bool supported = false;
		for (const auto& e : formatList)
			if (header.ddspf.fourCC == e.fourCC)
			{
				imageFormat = e.imageFormat;
				blockSize = e.blockSize;
				supported = true;
				break;
			}

		// ピクセル形式がDX10追加データで定義されている場合
		if (!supported && header.ddspf.fourCC == MakeFourCC('D', 'X', '1', '0'))
		{
			// DX10の追加データを読み取る
			char buf[sizeof(DDS_HEADER_DXT10)];
			ifs.read(buf, sizeof(DDS_HEADER_DXT10));
			const DDS_HEADER_DXT10 dx10header = ReadDX10Hader(buf);

			// DX10追加データのピクセル形式とOpenGLのピクセル形式の対応表
			constexpr struct
			{
				uint32_t dxgiFormat;
				uint32_t blockSize;
				GLenum imageFormat;
			}
			formatList[] =
			{
			  { DXGI_FORMAT_BC1_UNORM,  8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT },
			  { DXGI_FORMAT_BC2_UNORM, 16, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT },
			  { DXGI_FORMAT_BC3_UNORM, 16, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT },
			  { DXGI_FORMAT_BC4_UNORM,  8, GL_COMPRESSED_RED_RGTC1 },
			  { DXGI_FORMAT_BC4_SNORM,  8, GL_COMPRESSED_SIGNED_RED_RGTC1 },
			  { DXGI_FORMAT_BC5_UNORM, 16, GL_COMPRESSED_RG_RGTC2 },
			  { DXGI_FORMAT_BC5_SNORM, 16, GL_COMPRESSED_SIGNED_RG_RGTC2 },
			  { DXGI_FORMAT_BC6H_UF16, 16, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT },
			  { DXGI_FORMAT_BC6H_SF16, 16, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT },
			  { DXGI_FORMAT_BC7_UNORM, 16, GL_COMPRESSED_RGBA_BPTC_UNORM },
			};

			// 対応表から一致するデータを取得する
			for (const auto& e : formatList)
				if (dx10header.dxgiFormat == e.dxgiFormat)
				{
					imageFormat = e.imageFormat;
					blockSize = e.blockSize;
					supported = true;
					break;
				}
		} // if fourCC == DX10

		if (!supported)
		{
			LOG_ERROR("%sは未対応のDDSファイルです", filename);
			return 0; // 作成失敗
		}

		// 圧縮テクスチャでは画像とGPUのピクセル形式を合わせる必要がある
		gpuFormat = imageFormat;
	}
	else
	{
		LOG_ERROR("%sは圧縮DDSではありません", filename);
		return 0; // 作成失敗
	}

	// テクスチャを作成してGPUメモリを確保する
	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, header.mipmapCount, gpuFormat, header.width, header.height);

	// CPU側のメモリを確保する
	// DXT1=0.5バイト/ピクセル, DXT1以外=1バイト/ピクセル
	auto p = std::make_unique<char[]>(header.width * header.height);

	// 画像を読み込む
	glBindTexture(GL_TEXTURE_2D, tex); // テクスチャをOpenGLコンテキストにバインド
	uint32_t width = header.width;
	uint32_t height = header.height;
	for (uint32_t mipLevel = 0; mipLevel < header.mipmapCount; ++mipLevel)
	{
		// 圧縮テクスチャのデータはブロック単位で記録されるので、
		// 縦横のブロック数からバイト数を計算する
		const uint32_t w = (width + 3) / 4;  // 横のブロック数
		const uint32_t h = (height + 3) / 4; // 縦のブロック数
		const uint32_t imageBytes = w * h * blockSize; // 画像のバイト数

		// データを読み込んでテクスチャを作成する
		ifs.read(p.get(), imageBytes);
		glCompressedTexSubImage2D(GL_TEXTURE_2D, mipLevel,
			0, 0, width, height, imageFormat, imageBytes, p.get());

		// 次のミップマップのために縦横を1/2サイズにする
		width = std::max(1u, width / 2);
		height = std::max(1u, height / 2);
	}
	glBindTexture(GL_TEXTURE_2D, 0); // バインド解除

	// 最大ミップマップレベルを設定する
	glTextureParameteri(tex, GL_TEXTURE_MAX_LEVEL, header.mipmapCount - 1);

	// 拡大縮小フィルタを設定する
	// 縮小フィルタはミップマップを考慮して選択
	if (header.mipmapCount > 1)
		glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	else
		glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 作成したテクスチャを返す
	return tex;
}
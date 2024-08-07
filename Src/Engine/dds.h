/**
* @file dds.h
*/
#ifndef DDS_H_INCLUDED
#define DDS_H_INCLUDED
#include "glad/glad.h"
#include <cstdint>

// DDSファイルからテクスチャを作成する
GLuint LoadDDS(const char* filename);

// 画像のピクセル形式
struct DDS_PIXEL_FORMAT
{
	uint32_t size;         // この構造体のバイト数(32)
	uint32_t flags;        // 画像に含まれるデータの種類を示すフラグ
	uint32_t fourCC;       // 画像フォーマットを示すFOURCC
	uint32_t rgbBitCount;  // 1ピクセルのビット数
	uint32_t redBitMask;   // 赤要素が使う部分を示すビット
	uint32_t greenBitMask; // 緑要素が使う部分を示すビット
	uint32_t blueBitMask;  // 青要素が使う部分を示すビット
	uint32_t alphaBitMask; // 透明要素が使う部分を示すビット
};

// ピクセル形式フラグ
constexpr uint32_t DDPF_ALPHAPIXELS = 0x01;   // alphaBitMaskが有効
constexpr uint32_t DDPF_ALPHA = 0x02;		  // alphaBitMaskが有効(RGBのBitMaskは無効)
constexpr uint32_t DDPF_FOURCC = 0x04;		  // fourCCが有効(BitMaskは無効)
constexpr uint32_t DDPF_RGB = 0x40;			  // RGBのBitMaskが有効
constexpr uint32_t DDPF_LUMINANCE = 0x2'0000; // redBitMaskが有効(GBのBitMaskは無効)

// DDSファイルヘッダ
struct DDS_HEADER
{
	uint32_t size;              // この構造体のバイト数(124)
	uint32_t flags;             // どのパラメータが有効かを示す基本フラグ
	uint32_t height;            // 画像の高さ(ピクセル数)
	uint32_t width;             // 画像の幅(ピクセル数)
	uint32_t pitchOrLinearSize; // 横のバイト数または画像1枚のバイト数
	uint32_t depth;             // 画像の奥行き(枚数)(3次元テクスチャ等で使用)
	uint32_t mipmapCount;       // 含まれているミップマップレベル数
	uint32_t reserved1[11];     // (将来のために予約されている)
	DDS_PIXEL_FORMAT ddspf;     // 画像のピクセル形式
	uint32_t caps[4];           // 画像の特性を示すフラグ
	uint32_t reserved2;         // (将来のために予約されている)
};

// 基本フラグ
constexpr uint32_t DDSD_CAPS = 0x00'0001;		 // capsが有効(必須)
constexpr uint32_t DDSD_HEIGHT = 0x00'0002;		 // widthが有効(必須)
constexpr uint32_t DDSD_WIDTH = 0x00'0004;		 // heightが有効(必須)
constexpr uint32_t DDSD_PITCH = 0x00'0008;		 // pitchOrLinearSizeのpitchが有効
constexpr uint32_t DDSD_PIXELFORMAT = 0x00'1000; // ddspfが有効(必須)
constexpr uint32_t DDSD_MIPMAPCOUNT = 0x02'0000; // mipmapCountが有効
constexpr uint32_t DDSD_LINEARSIZE = 0x08'0000;  // pitchOrLinearSizeのlinearSizeが有効
constexpr uint32_t DDSD_DEPTH = 0x80'0000;		 // depthが有効

// 画像特性フラグ1
constexpr uint32_t DDSCAPS_TEXTURE = 0x00'1000;  // テクスチャである(必須)
constexpr uint32_t DDSCAPS_MIPMAP = 0x40'0000;   // ミップマップが格納されている
constexpr uint32_t DDSCAPS_COMPLEX = 0x00'0008;  // 二枚以上のサーフェスが格納されている

// 画像特性フラグ2
constexpr uint32_t DDSCAPS2_CUBEMAP = 0x00'0200; // キューブマップテクスチャが格納されている
constexpr uint32_t DDSCAPS2_VOLUME = 0x20'0000;  // ボリュームテクスチャが格納されている

// DirectX10で追加された拡張DDSファイルヘッダ
// fourCCが"DX10"の場合、DDS_HEADERの直後に置かれる
struct DDS_HEADER_DXT10
{
	uint32_t dxgiFormat; // ピクセル形式
	uint32_t resourceDimension; // テクスチャの次元数
	uint32_t miscFlag;   // その他のフラグ(その1)
	uint32_t arraySize;  // 配列テクスチャの要素数
	uint32_t miscFlags2; // その他のフラグ(その2)
};

// ピクセル形式
constexpr uint32_t DXGI_FORMAT_BC1_UNORM = 71; // DXT1
constexpr uint32_t DXGI_FORMAT_BC2_UNORM = 74; // DXT3
constexpr uint32_t DXGI_FORMAT_BC3_UNORM = 77; // DXT5
constexpr uint32_t DXGI_FORMAT_BC4_UNORM = 80;
constexpr uint32_t DXGI_FORMAT_BC4_SNORM = 81;
constexpr uint32_t DXGI_FORMAT_BC5_UNORM = 83;
constexpr uint32_t DXGI_FORMAT_BC5_SNORM = 84;
constexpr uint32_t DXGI_FORMAT_BC6H_UF16 = 95;
constexpr uint32_t DXGI_FORMAT_BC6H_SF16 = 96;
constexpr uint32_t DXGI_FORMAT_BC7_UNORM = 98;

#endif // DDS_H_INCLUDED
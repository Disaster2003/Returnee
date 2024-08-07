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
	/// �o�C�g�񂩂琔�l�𕜌�����
	/// </summary>
	/// <param name="buf">�o�C�g��ւ̃|�C���^</param>
	/// <returns>�����������l</returns>
	uint32_t Get(const char* buf)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
		return p[0] + (p[1] << 8UL) + (p[2] << 16UL) + (p[3] << 24UL);
	}
	
	/// <summary>
	/// FOURCC���쐬����
	/// </summary>
	/// <param name="a">FOURCC��1������</param>
	/// <param name="b">FOURCC��2������</param>
	/// <param name="c">FOURCC��3������</param>
	/// <param name="d">FOURCC��4������</param>
	/// <returns>a, b, c, d����v�Z����FOURCC</returns>
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
	/// �o�b�t�@����s�N�Z���`����ǂݏo��
	/// </summary>
	/// <param name="buf">�ǂݏo�����o�b�t�@</param>
	/// <returns>�s�N�Z���`����\��DDS_PIXEL_FORMAT�\����</returns>
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
	/// �o�b�t�@����DDS�w�b�_��ǂݏo��
	/// </summary>
	/// <param name="buf">�ǂݏo�����o�b�t�@</param>
	/// <returns>DDS�w�b�_��\��DDS_HEADER�\����</returns>
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
	/// �o�b�t�@����DX10�g���w�b�_�摜����ǂݏo��
	/// </summary>
	/// <param name="buf">�ǂݏo�����o�b�t�@</param>
	/// <returns>DX10�g���w�b�_��\��DDSHeaderDXT10�\����</returns>
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
/// DDS�t�@�C������e�N�X�`�����쐬����
/// </summary>
/// <param name="filename">DDS�t�@�C����</param>
/// <returns>0 = �쐬���s�A0�ȊO = �쐬�����e�N�X�`���̊Ǘ��ԍ�</returns>
GLuint LoadDDS(const char* filename)
{
	// �t�@�C�����J��
	std::ifstream ifs(filename, std::ios_base::binary);
	if (!ifs)
	{
		LOG_ERROR("%s���J���܂���", filename);
		return 0; // �쐬���s
	}

	// �t�@�C���̐擪������ǂݍ���
	char buf[128];
	ifs.read(buf, 128);
	if (buf[0] != 'D' || buf[1] != 'D' || buf[2] != 'S' || buf[3] != ' ')
	{
		LOG_ERROR("%s��DDS�t�@�C���ł͂���܂���", filename);
		return 0; // �쐬���s
	}

	// DDS�w�b�_�����o��
	const DDS_HEADER header = ReadDDSHeader(buf + 4);
	if (header.size != 124)
	{
		LOG_ERROR("%s�͖��Ή���DDS�t�@�C���ł�", filename);
		return 0; // �쐬���s
	}

	// DDS�s�N�Z���`���ɑΉ�����OpenGL�s�N�Z���`����I������
	GLenum imageFormat;      // �摜�f�[�^�̃s�N�Z���`��
	GLenum gpuFormat;        // GPU���̃s�N�Z���`��
	uint32_t blockSize = 16; // 1�u���b�N�̃o�C�g��
	if (header.ddspf.flags & DDPF_FOURCC)
	{
		// �s�N�Z���`����FourCC�Œ�`����Ă���ꍇ
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

		// �s�N�Z���`����DX10�ǉ��f�[�^�Œ�`����Ă���ꍇ
		if (!supported && header.ddspf.fourCC == MakeFourCC('D', 'X', '1', '0'))
		{
			// DX10�̒ǉ��f�[�^��ǂݎ��
			char buf[sizeof(DDS_HEADER_DXT10)];
			ifs.read(buf, sizeof(DDS_HEADER_DXT10));
			const DDS_HEADER_DXT10 dx10header = ReadDX10Hader(buf);

			// DX10�ǉ��f�[�^�̃s�N�Z���`����OpenGL�̃s�N�Z���`���̑Ή��\
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

			// �Ή��\�����v����f�[�^���擾����
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
			LOG_ERROR("%s�͖��Ή���DDS�t�@�C���ł�", filename);
			return 0; // �쐬���s
		}

		// ���k�e�N�X�`���ł͉摜��GPU�̃s�N�Z���`�������킹��K�v������
		gpuFormat = imageFormat;
	}
	else
	{
		LOG_ERROR("%s�͈��kDDS�ł͂���܂���", filename);
		return 0; // �쐬���s
	}

	// �e�N�X�`�����쐬����GPU���������m�ۂ���
	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, header.mipmapCount, gpuFormat, header.width, header.height);

	// CPU���̃��������m�ۂ���
	// DXT1=0.5�o�C�g/�s�N�Z��, DXT1�ȊO=1�o�C�g/�s�N�Z��
	auto p = std::make_unique<char[]>(header.width * header.height);

	// �摜��ǂݍ���
	glBindTexture(GL_TEXTURE_2D, tex); // �e�N�X�`����OpenGL�R���e�L�X�g�Ƀo�C���h
	uint32_t width = header.width;
	uint32_t height = header.height;
	for (uint32_t mipLevel = 0; mipLevel < header.mipmapCount; ++mipLevel)
	{
		// ���k�e�N�X�`���̃f�[�^�̓u���b�N�P�ʂŋL�^�����̂ŁA
		// �c���̃u���b�N������o�C�g�����v�Z����
		const uint32_t w = (width + 3) / 4;  // ���̃u���b�N��
		const uint32_t h = (height + 3) / 4; // �c�̃u���b�N��
		const uint32_t imageBytes = w * h * blockSize; // �摜�̃o�C�g��

		// �f�[�^��ǂݍ���Ńe�N�X�`�����쐬����
		ifs.read(p.get(), imageBytes);
		glCompressedTexSubImage2D(GL_TEXTURE_2D, mipLevel,
			0, 0, width, height, imageFormat, imageBytes, p.get());

		// ���̃~�b�v�}�b�v�̂��߂ɏc����1/2�T�C�Y�ɂ���
		width = std::max(1u, width / 2);
		height = std::max(1u, height / 2);
	}
	glBindTexture(GL_TEXTURE_2D, 0); // �o�C���h����

	// �ő�~�b�v�}�b�v���x����ݒ肷��
	glTextureParameteri(tex, GL_TEXTURE_MAX_LEVEL, header.mipmapCount - 1);

	// �g��k���t�B���^��ݒ肷��
	// �k���t�B���^�̓~�b�v�}�b�v���l�����đI��
	if (header.mipmapCount > 1)
		glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	else
		glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// �쐬�����e�N�X�`����Ԃ�
	return tex;
}
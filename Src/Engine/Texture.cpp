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
/// �R���X�g���N�^
/// </summary>
/// <param name="filename">�e�N�X�`���t�@�C����</param>
Texture::Texture(const char* filename)
{
	// �g���q��dds�̏ꍇ��DDS�t�@�C���Ƃ݂Ȃ�
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
			LOG("%s��ǂݍ��݂܂���", filename);
		}
		return;
	}

	std::ifstream file(filename, std::ios::binary);
	if (!file)
	{
		LOG_ERROR("%s���J���܂���", filename);
		return;
	}

	// �t�@�C����ǂݍ���
	const size_t filesize = std::filesystem::file_size(filename);
	std::vector<uint8_t> buffer(filesize);
	file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
	file.close();

	// �w�b�_��������擾
	const size_t tgaHeaderSize = 18;				   // �w�b�_���̃o�C�g��
	const int imageOffset = tgaHeaderSize + buffer[0]; // �摜�f�[�^�̈ʒu
	const uint8_t imageType = buffer[2];               // �摜�`��
	const int pixelBytes = (buffer[16] + 7) / 8;       // 1�s�N�Z���̃o�C�g��
	width = buffer[12] + buffer[13] * 256;
	height = buffer[14] + buffer[15] * 256;

	// ���k�`���̏ꍇ�͓W�J����
	//  0(0b0000) : �摜�Ȃ�
	//  1(0b0001) : �C���f�b�N�X(�����k)
	//  2(0b0010) : �g�D���[�J���[(�����k)
	//  3(0b0011) : ����(�����k)
	//  9(0b1001) : �C���f�b�N�X(RLE)
	// 10(0b1010) : �g�D���[�J���[(RLE)
	// 11(0b1011) : ����(RLE)
	if (imageType & 0b1000)
	{
		// �W�J�p�̃o�b�t�@��p��
		const int imageBytes = width * height * pixelBytes; // �W�J��̃o�C�g��
		std::vector<uint8_t> tmp(imageOffset + imageBytes);

		// swap�ɔ����āA�摜�f�[�^���O�̕������R�s�[
		std::copy_n(buffer.begin(), imageOffset, tmp.begin());

		// �摜�f�[�^��W�J
		const uint8_t* src = buffer.data() + imageOffset; // ���k�f�[�^�̃A�h���X
		uint8_t* dest = tmp.data() + imageOffset;         // �f�[�^�W�J��A�h���X
		const uint8_t* const destEnd = dest + imageBytes; // �W�J�I���A�h���X

		while (dest != destEnd)
		{
			// �p�P�b�g�w�b�_����ID�ƃf�[�^�����擾
			const int isRLE = *src & 0x80;
			const int count = (*src & 0x7f) + 1;
			++src; // �A�h���X���p�P�b�g�f�[�^�̈ʒu�ɐi�߂�

			if (isRLE)
			{
				// ���k�f�[�^�̏ꍇ�A�p�P�b�g�f�[�^���w��񐔃R�s�[
				for (int a = 0; a < count; ++a)
				{
					memcpy(dest, src, pixelBytes);
					dest += pixelBytes;
				}
				src += pixelBytes;
			}
			else
			{
				// �����k�f�[�^�̏ꍇ�A�p�P�b�g�f�[�^�S�̂��R�s�[
				const int dataBytes = pixelBytes * count;
				memcpy(dest, src, dataBytes);
				dest += dataBytes;
				src += dataBytes;
			}
		} // while
		// buffer��tmp�̓��e������
		buffer.swap(tmp);
	}

	// �i�[�������u�ォ�牺�v�̏ꍇ�A�f�[�^���㉺���]
	const bool topToBottom = buffer[17] & 0b0010'0000;
	if (topToBottom)
	{
		const int pixelDepth = buffer[16];					 // 1�s�N�Z���̃r�b�g��
		const int lineByteSize = width * pixelDepth / 8;	 // 1�s�̃o�C�g��
		uint8_t* top = buffer.data() + tgaHeaderSize;		 // ��̍s�̈ʒu
		uint8_t* bottom = top + lineByteSize * (height - 1); // ���̍s�̈ʒu
		std::vector<uint8_t>tmp(lineByteSize);				 // �㉺����ւ��p�̃o�b�t�@

		// �s�P�ʂŏ㉺���]
		// �㉺�̍s�̈ʒu���t�]����܂ŌJ��Ԃ�
		while (top < bottom)
		{
			std::copy_n(top, lineByteSize, tmp.data());		// �u��̍s�̃R�s�[�v�����
			std::copy_n(bottom, lineByteSize, top);			// ���̍s���A��̍s�ɏ㏑��
			std::copy_n(tmp.data(), lineByteSize, bottom);	// �u��̍s�̃R�s�[�v�����̍s�ɏ㏑��
			top += lineByteSize;							// ��̍s�̈ʒu��1�s���Ɉړ�
			bottom -= lineByteSize;							// ���̍s�̈ʒu��1�s��Ɉړ�
		}
	} // if topToBottom

	  // 1�s�N�Z���̃r�b�g���ɑΉ�����`���̃Z�b�g������
	struct FORMAT
	{
		int pixelDepth;     // TGA��1�s�N�Z���̃r�b�g��
		GLenum imageFormat; // TGA�̃f�[�^�`��
		GLenum imageType;   // TGA�̃f�[�^�^
		GLenum gpuFormat;   // GPU���̃f�[�^�`��
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

	// ���݂̃A���C�������g���L�^
	GLint alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

	// �摜�̃A���C�������g������
	// 1�s�̃o�C�g����4�Ŋ���؂��Ƃ���4�A2�Ŋ���؂��Ƃ���2�A����ȊO��1�ɐݒ�
	constexpr int alignmentList[] = { 4, 1, 2, 1 };
	const int lineByteSize = width * pixelBytes; // 1�s�̃o�C�g��
	const int imageAlignment = alignmentList[lineByteSize % 4];

	// �A���C�������g��ύX
	if (alignment != imageAlignment)
		glPixelStorei(GL_UNPACK_ALIGNMENT, imageAlignment);

	// �e�N�X�`�����쐬
	GLuint object = 0; // �e�N�X�`���̊Ǘ��ԍ�
	glCreateTextures(GL_TEXTURE_2D, 1, &object);
	glTextureStorage2D(object, 1, format->gpuFormat, width, height);
	glTextureSubImage2D(object, 0, 0, 0, width, height,
		format->imageFormat, format->imageType, buffer.data() + tgaHeaderSize);

	id = object;
	name = filename;

	// �A���C�������g�����ɖ߂�
	if (alignment != imageAlignment)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	}

	// �O���[�X�P�[���e�N�X�`���̏ꍇ�A�Ԑ�����΂ƐɃR�s�[����悤�ɐݒ肷��
	if (format->imageFormat == GL_RED)
	{
		glTextureParameteri(object, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTextureParameteri(object, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTextureParameteri(object, GL_TEXTURE_SWIZZLE_B, GL_RED);
	}
}

/// <summary>
/// �L���[�u�}�b�v���쐬����R���X�g���N�^
/// </summary>
/// <param name="name">�e�N�X�`�����ʗp�̖��O</param>
/// <param name="cubemap_filenames">�L���[�u�}�b�v���\������6���̉摜�t�@�C����</param>
Texture::Texture
(
	const char* name,
	const char* const cubemap_filenames[6]
)
{
	// �摜�t�@�C����ǂݍ���
	Texture t[6] =
	{
	  Texture(cubemap_filenames[0]),
	  Texture(cubemap_filenames[1]),
	  Texture(cubemap_filenames[2]),
	  Texture(cubemap_filenames[3]),
	  Texture(cubemap_filenames[4]),
	  Texture(cubemap_filenames[5]),
	};

	// 1���ł��e�N�X�`���̓ǂݍ��݂Ɏ��s���Ă�����쐬���Ȃ�
	for (const auto& e : t)
	{
		if (!e)
		{
			LOG_ERROR("�L���[�u�}�b�v%s�̉摜�̓ǂݍ��݂Ɏ��s", name);
			return; // 6��������Ă��Ȃ��ƍ쐬�ł��Ȃ�
		}
	}

	// �摜�T�C�Y���擾����
	const int w = t[0].GetWidth();
	const int h = t[0].GetHeight();
	if (w != h)
	{
		LOG_ERROR("�L���[�u�}�b�v%s�������`�ł͂���܂���(%dx%d)", name, w, h);
		return; // �c�Ɖ��������T�C�Y�łȂ��ƍ쐬�ł��Ȃ�
	}
	for (const auto& e : t)
	{
		if (e.GetWidth() != w || e.GetHeight() != h)
		{
			LOG_ERROR("�L���[�u�}�b�v%s�̉摜�T�C�Y����v���܂���", name);
			return; // ���ׂẴT�C�Y���������Ȃ��ƍ쐬�ł��Ȃ�
		}
	}

	// �摜�`�����擾����
	GLint gpuFormat;
	glGetTextureLevelParameteriv(t[0], 0, GL_TEXTURE_INTERNAL_FORMAT, &gpuFormat);
	for (const auto& e : t)
	{
		GLint tmp;
		glGetTextureLevelParameteriv(e, 0, GL_TEXTURE_INTERNAL_FORMAT, &tmp);
		if (tmp != gpuFormat)
		{
			LOG_ERROR("�L���[�u�}�b�v%s�̉摜�`������v���܂���", name);
			return; // ���ׂẲ摜�`������v���Ȃ��ƍ쐬�ł��Ȃ�
		}
	} // for t

	// �摜�T�C�Y����~�b�v�}�b�v�����v�Z����
	const int levels = static_cast<int>(log2(w)) + 1;

	// �L���[�u�}�b�v���쐬����
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);
	glTextureStorage2D(id, levels, gpuFormat, w, h);
	for (int i = 0; i < 6; ++i)
	{
		glCopyImageSubData(t[i], GL_TEXTURE_2D, 0, 0, 0, 0,
			id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, i, w, h, 1);
	}

	// �~�b�v�}�b�v�𐶐�����
	glGenerateTextureMipmap(id);

	// �e�N�X�`���p�����[�^��ݒ肷��
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

/// <summary>
/// ��̃e�N�X�`�����쐬����R���X�g���N�^
/// </summary>
/// <param name="name">�e�N�X�`�����ʗp�̖��O</param>
/// <param name="width">�e�N�X�`���̕�(�s�N�Z����)</param>
/// <param name="height">�e�N�X�`���̍���(�s�N�Z����)</param>
/// <param name="format_gpu">�f�[�^�`��</param>
/// <param name="mode_wrap">���b�v���[�h</param>
/// <param name="levels">�~�b�v�}�b�v�e�N�X�`���̃��x����</param>
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

	// �[�x�e�N�X�`���̏ꍇ�A�V���h�E�}�b�s���O�̂��߂ɔ�r���[�h��ݒ肷��
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
/// �f�X�g���N�^
/// </summary>
Texture::~Texture()
{
	glDeleteTextures(1,&id);
}
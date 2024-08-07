/**
* @file dds.h
*/
#ifndef DDS_H_INCLUDED
#define DDS_H_INCLUDED
#include "glad/glad.h"
#include <cstdint>

// DDS�t�@�C������e�N�X�`�����쐬����
GLuint LoadDDS(const char* filename);

// �摜�̃s�N�Z���`��
struct DDS_PIXEL_FORMAT
{
	uint32_t size;         // ���̍\���̂̃o�C�g��(32)
	uint32_t flags;        // �摜�Ɋ܂܂��f�[�^�̎�ނ������t���O
	uint32_t fourCC;       // �摜�t�H�[�}�b�g������FOURCC
	uint32_t rgbBitCount;  // 1�s�N�Z���̃r�b�g��
	uint32_t redBitMask;   // �ԗv�f���g�������������r�b�g
	uint32_t greenBitMask; // �Ηv�f���g�������������r�b�g
	uint32_t blueBitMask;  // �v�f���g�������������r�b�g
	uint32_t alphaBitMask; // �����v�f���g�������������r�b�g
};

// �s�N�Z���`���t���O
constexpr uint32_t DDPF_ALPHAPIXELS = 0x01;   // alphaBitMask���L��
constexpr uint32_t DDPF_ALPHA = 0x02;		  // alphaBitMask���L��(RGB��BitMask�͖���)
constexpr uint32_t DDPF_FOURCC = 0x04;		  // fourCC���L��(BitMask�͖���)
constexpr uint32_t DDPF_RGB = 0x40;			  // RGB��BitMask���L��
constexpr uint32_t DDPF_LUMINANCE = 0x2'0000; // redBitMask���L��(GB��BitMask�͖���)

// DDS�t�@�C���w�b�_
struct DDS_HEADER
{
	uint32_t size;              // ���̍\���̂̃o�C�g��(124)
	uint32_t flags;             // �ǂ̃p�����[�^���L������������{�t���O
	uint32_t height;            // �摜�̍���(�s�N�Z����)
	uint32_t width;             // �摜�̕�(�s�N�Z����)
	uint32_t pitchOrLinearSize; // ���̃o�C�g���܂��͉摜1���̃o�C�g��
	uint32_t depth;             // �摜�̉��s��(����)(3�����e�N�X�`�����Ŏg�p)
	uint32_t mipmapCount;       // �܂܂�Ă���~�b�v�}�b�v���x����
	uint32_t reserved1[11];     // (�����̂��߂ɗ\�񂳂�Ă���)
	DDS_PIXEL_FORMAT ddspf;     // �摜�̃s�N�Z���`��
	uint32_t caps[4];           // �摜�̓����������t���O
	uint32_t reserved2;         // (�����̂��߂ɗ\�񂳂�Ă���)
};

// ��{�t���O
constexpr uint32_t DDSD_CAPS = 0x00'0001;		 // caps���L��(�K�{)
constexpr uint32_t DDSD_HEIGHT = 0x00'0002;		 // width���L��(�K�{)
constexpr uint32_t DDSD_WIDTH = 0x00'0004;		 // height���L��(�K�{)
constexpr uint32_t DDSD_PITCH = 0x00'0008;		 // pitchOrLinearSize��pitch���L��
constexpr uint32_t DDSD_PIXELFORMAT = 0x00'1000; // ddspf���L��(�K�{)
constexpr uint32_t DDSD_MIPMAPCOUNT = 0x02'0000; // mipmapCount���L��
constexpr uint32_t DDSD_LINEARSIZE = 0x08'0000;  // pitchOrLinearSize��linearSize���L��
constexpr uint32_t DDSD_DEPTH = 0x80'0000;		 // depth���L��

// �摜�����t���O1
constexpr uint32_t DDSCAPS_TEXTURE = 0x00'1000;  // �e�N�X�`���ł���(�K�{)
constexpr uint32_t DDSCAPS_MIPMAP = 0x40'0000;   // �~�b�v�}�b�v���i�[����Ă���
constexpr uint32_t DDSCAPS_COMPLEX = 0x00'0008;  // �񖇈ȏ�̃T�[�t�F�X���i�[����Ă���

// �摜�����t���O2
constexpr uint32_t DDSCAPS2_CUBEMAP = 0x00'0200; // �L���[�u�}�b�v�e�N�X�`�����i�[����Ă���
constexpr uint32_t DDSCAPS2_VOLUME = 0x20'0000;  // �{�����[���e�N�X�`�����i�[����Ă���

// DirectX10�Œǉ����ꂽ�g��DDS�t�@�C���w�b�_
// fourCC��"DX10"�̏ꍇ�ADDS_HEADER�̒���ɒu�����
struct DDS_HEADER_DXT10
{
	uint32_t dxgiFormat; // �s�N�Z���`��
	uint32_t resourceDimension; // �e�N�X�`���̎�����
	uint32_t miscFlag;   // ���̑��̃t���O(����1)
	uint32_t arraySize;  // �z��e�N�X�`���̗v�f��
	uint32_t miscFlags2; // ���̑��̃t���O(����2)
};

// �s�N�Z���`��
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
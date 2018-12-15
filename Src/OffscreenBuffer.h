/**
* @file OffscreenBuffer.h
*/
#ifndef OFFSCREENBUFFER_H_INCLUDED
#define OFFSCREENBUFFER_H_INCLUDED
#include "Texture.h"
#include <GL/glew.h>
#include <memory>

class OffscreenBuffer;
using OffscreenBufferPtr = std::shared_ptr<OffscreenBuffer>; ///< �I�t�X�N���[���o�b�t�@�|�C���^.

/**
* �I�t�X�N���[���o�b�t�@.
*/
class OffscreenBuffer
{
public:
	//static OffscreenBufferPtr Create(int w, int h);
	static OffscreenBufferPtr Create(int w, int h, GLenum f = GL_RGBA8);
	GLuint GetFramebuffer() const { return framebuffer; } ///< �t���[���o�b�t�@���擾����.
	GLuint GetTexutre() const { return tex->Id(); } ///< �t���[���o�b�t�@�p�e�N�X�`�����擾����.
	GLsizei Width() const { return tex->Width(); } ///< �t���[���o�b�t�@�p�e�N�X�`���̉������擾����.
	GLsizei Height() const { return tex->Height(); } ///< �t���[���o�b�t�@�p�e�N�X�`���̏c�����擾����.

private:
	OffscreenBuffer() = default;
	OffscreenBuffer(const OffscreenBuffer&) = delete;
	OffscreenBuffer& operator=(const OffscreenBuffer&) = delete;
	~OffscreenBuffer();

	TexturePtr tex; ///< �t���[���o�b�t�@�p�e�N�X�`��.
	GLuint depthbuffer = 0; ///< �[�x�o�b�t�@�I�u�W�F�N�g.
	GLuint framebuffer = 0; ///< �t���[���o�b�t�@�I�u�W�F�N�g.
};

#endif
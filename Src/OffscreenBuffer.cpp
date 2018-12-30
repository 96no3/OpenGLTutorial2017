/**
* @file OffscreenBuffer.cpp
*/
#include "OffscreenBuffer.h"
#include <iostream>

/**
* �I�t�X�N���[���o�b�t�@���쐬����.
*
* @param w �I�t�X�N���[���o�b�t�@�̕�(�s�N�Z���P��).
* @param h �I�t�X�N���[���o�b�t�@�̍���(�s�N�Z���P��).
* @param f �e�N�X�`���`��.
*
* @return �쐬�����I�t�X�N���[���o�b�t�@�ւ̃|�C���^.
*/
//OffscreenBufferPtr OffscreenBuffer::Create(int w, int h)
OffscreenBufferPtr OffscreenBuffer::Create(int w, int h, GLenum iformat)
{
	struct Impl : OffscreenBuffer {};
	OffscreenBufferPtr offscreen = std::make_shared<Impl>();
	if (!offscreen) {
		return offscreen;
	}

	GLenum format = GL_RGBA;
	switch (iformat) {
	default:
		format = GL_RGBA;
		break;
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH_COMPONENT32:
	case GL_DEPTH_COMPONENT32F:
		format = GL_DEPTH_COMPONENT;
		break;
	}

	// �e�N�X�`�����쐬����.
	//offscreen->tex = Texture::Create(w, h, iformat, GL_RGBA, nullptr);
	offscreen->tex = Texture::Create(w, h, iformat, format, nullptr);
	if (!offscreen->tex) {
		return {};
	}

	//// �[�x�o�b�t�@���쐬����.
	//glGenRenderbuffers(1, &offscreen->depthbuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthbuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//// �t���[���o�b�t�@���쐬����.
	//glGenFramebuffers(1, &offscreen->framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthbuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// �f�v�X�V���h�E�}�b�v���쐬����.
	if (format == GL_DEPTH_COMPONENT) {
		glGenFramebuffers(1, &offscreen->framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
		glDrawBuffer(GL_NONE);
	}
	else {// �[�x�o�b�t�@���쐬����.
		glGenRenderbuffers(1, &offscreen->depthbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// �t���[���o�b�t�@���쐬����.
		glGenFramebuffers(1, &offscreen->framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthbuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR: OffscreenBuffer�̍쐬�Ɏ��s." << std::endl;
		offscreen.reset();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return offscreen;
}

/**
* �f�X�g���N�^.
*/
OffscreenBuffer::~OffscreenBuffer()
{
	if (framebuffer) {
		glDeleteFramebuffers(1, &framebuffer);
	}
	if (depthbuffer) {
		glDeleteRenderbuffers(1, &depthbuffer);
	}
}


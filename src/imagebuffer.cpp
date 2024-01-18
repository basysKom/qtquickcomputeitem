// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "imagebuffer.h"

#include <QDebug>
#include <QImage>

ImageBuffer::ImageBuffer(QObject *parent)
    : ComputeShaderBuffer(parent)
{

}

ImageBuffer::~ImageBuffer()
{

}

QByteArray ImageBuffer::buffer() const  
{
    return m_buffer;
}

void ImageBuffer::setBuffer(const QByteArray &byteArray)
{
    // QImage img(reinterpret_cast<const uchar *>(byteArray.constData()), m_imageSize.width(), m_imageSize.height(), QImage::Format_RGBA8888);
    // img.save("test.png");
    m_buffer = byteArray;
    emit bufferChanged();
}

QString ImageBuffer::imageSource() const
{
    return m_imageSource;
}

void ImageBuffer::setImageSource(const QString &source)
{
    if (m_imageSource != source) {
        m_imageSource = source;
        emit imageSourceChanged();
    }
}

QSize ImageBuffer::imageSize() const
{
    return m_imageSize;
}

void ImageBuffer::setImageSize(const QSize &size)
{
    if (size != m_imageSize) {
        m_imageSize = size;
        emit imageSizeChanged();
    }
}

ImageBuffer::TextureFormat ImageBuffer::textureFormat() const
{
    return m_textureFormat;
}

void ImageBuffer::setTextureFormat(TextureFormat format)
{
    if (format != m_textureFormat) {
        m_textureFormat = format;
        emit textureFormatChanged();
    }
}


void ImageBuffer::setQSGTexture(QSGTexture *qsgTexture)
{
    m_qsgTexture = qsgTexture;
}

QSGTexture* ImageBuffer::qsgTexture() const
{
    if (m_qsgTexture.isNull()) {
        return nullptr;
    }
    return m_qsgTexture.data();
}

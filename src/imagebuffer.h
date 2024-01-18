// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QByteArray>
#include <QQuickItem>
#include <QSize>
#include <QPointer>

#include <QSGTexture>

#include "computeshaderbuffer.h"

class ImageBuffer : public ComputeShaderBuffer
{
    Q_OBJECT

    // buffer and bufferSize are only needed if no image source is given
    Q_PROPERTY(QByteArray buffer READ buffer WRITE setBuffer NOTIFY bufferChanged)
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)
    Q_PROPERTY(TextureFormat textureFormat READ textureFormat WRITE setTextureFormat NOTIFY textureFormatChanged)

    Q_PROPERTY(QString imageSource READ imageSource WRITE setImageSource NOTIFY imageSourceChanged)
    QML_ELEMENT

public:

    //! Enum representing the possible image formats, keep in sync with QRhiTexture::Format
    enum TextureFormat
    {
        RGBA8 = 1,
        RGBA16F = 8,
        RGBA32F = 9
    };
    Q_ENUM(TextureFormat)

    explicit ImageBuffer(QObject *parent = nullptr);
    ~ImageBuffer();

    virtual ComputeShaderBuffer::BufferType type() override { return ComputeShaderBuffer::Image; };

    QByteArray buffer() const override;
    void setBuffer(const QByteArray &byteArray) override;

    QString imageSource() const;
    void setImageSource(const QString &source);

    QSize imageSize() const;
    void setImageSize(const QSize &size);

    TextureFormat textureFormat() const;
    void setTextureFormat(TextureFormat format);

    void setQSGTexture(QSGTexture *qsgTexture);
    QSGTexture* qsgTexture() const;

signals:
    void imageSourceChanged();
    void imageSizeChanged();
    void textureFormatChanged();

private:
    QByteArray m_buffer;
    QString m_imageSource;
    QSize m_imageSize;
    TextureFormat m_textureFormat { TextureFormat::RGBA8 };

    QPointer<QSGTexture> m_qsgTexture;
};
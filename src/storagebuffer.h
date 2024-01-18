// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QByteArray>
#include <QQuickItem>

#include "computeshaderbuffer.h"

class StorageBuffer : public ComputeShaderBuffer
{
    Q_OBJECT
    Q_PROPERTY(QByteArray buffer READ buffer WRITE setBuffer NOTIFY bufferChanged)
    QML_ELEMENT

public:
    explicit StorageBuffer(QObject *parent = nullptr);
    ~StorageBuffer();

    virtual ComputeShaderBuffer::BufferType type() override { return ComputeShaderBuffer::StorageBuffer; };

    QByteArray buffer() const override;
    void setBuffer(const QByteArray &byteArray) override;

private:
    QByteArray m_buffer;
};
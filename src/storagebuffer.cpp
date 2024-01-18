// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "storagebuffer.h"

#include <QDebug>

StorageBuffer::StorageBuffer(QObject *parent)
    : ComputeShaderBuffer(parent)
{

}

StorageBuffer::~StorageBuffer()
{

}

QByteArray StorageBuffer::buffer() const
{
    return m_buffer;
}

void StorageBuffer::setBuffer(const QByteArray &byteArray)
{
    if (m_buffer != byteArray) {
        m_buffer = byteArray;
#if 0
        qDebug() << m_buffer.length();
        if (m_buffer.length() > 0) {
            const float *asF = reinterpret_cast<const float *>(m_buffer.constData());
            for (int i = 0; i < m_buffer.length() / 4; i++) {
                qDebug() << *asF;
                asF++;
            }

        }
#endif
        emit bufferChanged();
    }
}


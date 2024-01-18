// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QPointer>

class ComputeItem;

class ComputeShaderBuffer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE(QLatin1String(
        "Cannot create a ComputeShaderBuffer directly: Create a StorageBuffer or an ImageBuffer instead"
    ))

public:

    enum BufferType {
        StorageBuffer,
        Image
    };

    explicit ComputeShaderBuffer(QObject *parent = nullptr);
    ~ComputeShaderBuffer();

    virtual BufferType type() = 0;

    void setComputeItem(ComputeItem *computeItem);
    bool hasComputeItem() { return !m_computeItem.isNull(); }

    virtual QByteArray buffer() const = 0;
    virtual void setBuffer(const QByteArray &byteArray) = 0;

signals:
    void bufferChanged();

private:
    QPointer<ComputeItem> m_computeItem;

};
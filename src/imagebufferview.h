// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QString>
#include <QVector>
#include <QQuickItem>
#include <QSGNode>
#include <QPointer>

#include <qqml.h>

#include "computeitem.h"
#include "imagebuffer.h"

class ImageBufferView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(ComputeItem *computeItem READ computeItem WRITE setComputeItem NOTIFY computeItemChanged)
    Q_PROPERTY(ImageBuffer* resultBuffer READ resultBuffer WRITE setResultBuffer NOTIFY resultBufferChanged)
    QML_ELEMENT

public:
    explicit ImageBufferView(QQuickItem *parent = nullptr);
    ~ImageBufferView();

    ComputeItem* computeItem() const;
    void setComputeItem(ComputeItem *item);

    ImageBuffer* resultBuffer() const;
    void setResultBuffer(ImageBuffer* buffer);

signals:
    void computeItemChanged();
    void resultBufferChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    ComputeItem* m_computeItem { nullptr };
    QPointer<ImageBuffer> m_imageBuffer;


};
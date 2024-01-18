// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "imagebufferview.h"

#include <QDebug>
#include <QSGTexture>
#include <QSGSimpleTextureNode>

ImageBufferView::ImageBufferView(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
}

ImageBufferView::~ImageBufferView()
{

}

ComputeItem* ImageBufferView::computeItem() const
{
    return m_computeItem;
}

void ImageBufferView::setComputeItem(ComputeItem *item)
{
    if (item != m_computeItem) {
        if (m_computeItem) {
            m_computeItem->disconnect(this);
        }
        m_computeItem = item;
        if (m_computeItem) {
            connect(m_computeItem, &ComputeItem::notifyChange, this, &QQuickItem::update);
        }
        emit computeItemChanged();
    }
}

ImageBuffer* ImageBufferView::resultBuffer() const
{
    return m_imageBuffer.data();
}

void ImageBufferView::setResultBuffer(ImageBuffer* buffer)
{
    if (buffer != m_imageBuffer.data()) {
        m_imageBuffer = buffer;
        emit resultBufferChanged();
    }
}

QSGNode* ImageBufferView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{

    if (!m_computeItem) {
        return oldNode;
    }

    if (m_imageBuffer.isNull()) {
        qWarning() << "Cannot render without result buffer";
        return oldNode;
    }

    auto texture = m_imageBuffer->qsgTexture();
    if (!texture) {
        qWarning() << "Texture not active";
        return oldNode;
    }

    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!node) {
        node = new QSGSimpleTextureNode();
    }

   node->setTexture(texture);
   node->setFiltering(QSGTexture::Linear);

    node->setRect(boundingRect());
    return node;
}


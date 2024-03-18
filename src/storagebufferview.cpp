// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "storagebufferview.h"

#include <QDebug>
#include <QRectF>
#include <QFile>
#include <QQuickWindow>
#include <QRunnable>
#include <QSGSimpleTextureNode>
#include <QSGRenderNode>

#include "storagebuffer.h"

class PointCloudRenderNode : public QSGRenderNode
{
public:
    explicit PointCloudRenderNode();
    ~PointCloudRenderNode();

public:
    void setWindow(QQuickWindow *window) { m_window = window; }
    void setPointBuffer(QRhiBuffer* buffer) { m_buffer = buffer; }

    void setNumberOfPoints(int nop) { m_numberOfPoints = nop; }
    void setPointSize(float ps) { m_pointSize = ps; }
    void setStrideInByte(quint32 stride) { m_strideInByte = stride; }

    void setBoundingRect(const QRectF &rect) { m_boundingRect = rect; }

    void prepare() override;
    void render(const RenderState *state) override;
    void releaseResources() override;
    RenderingFlags flags() const override;

private:
    QShader loadShader(const QString &filename);
    QRhi* checkRhi() const;
    QRhiSwapChain* checkSwapChain() const;
    
    QRectF m_boundingRect;

    int m_numberOfPoints { 0 };
    float m_pointSize { 1.0 };
    quint32 m_strideInByte { 8 * sizeof(float) };

    QQuickWindow *m_window { nullptr };
    QRhiBuffer *m_buffer { nullptr }; 

    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;
    std::unique_ptr<QRhiShaderResourceBindings> m_resourceBindings;
    std::unique_ptr<QRhiBuffer> m_uniformBuffer;

};

PointCloudRenderNode::PointCloudRenderNode()
{

}

PointCloudRenderNode::~PointCloudRenderNode()
{

}

StorageBufferView::StorageBufferView(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

StorageBufferView::~StorageBufferView()
{

}

ComputeItem* StorageBufferView::computeItem() const
{
    return m_computeItem;
}

void StorageBufferView::setComputeItem(ComputeItem *item)
{
    if (item != m_computeItem) {

        if (m_computeItem) {
            disconnect(m_computeItem, &ComputeItem::notifyChange, this, &QQuickItem::update);
        }

        m_computeItem = item;
        connect(m_computeItem, &ComputeItem::notifyChange, this,  &QQuickItem::update);
        emit computeItemChanged();
    }
}

StorageBuffer* StorageBufferView::resultBuffer() const
{
    if (m_resultBufferIndex < 0) {
        return nullptr;
    }

    return qobject_cast<StorageBuffer*>(m_computeItem->bufferAt(m_resultBufferIndex));
}

void StorageBufferView::setResultBuffer(const StorageBuffer* buffer)
{
    if (!m_computeItem) {
        return;
    }
    int resultBufferIndex = m_computeItem->indexForBuffer<StorageBuffer>(buffer);
    if (resultBufferIndex != m_resultBufferIndex) {
        m_resultBufferIndex = resultBufferIndex;
        emit resultBufferChanged();
    }
}

void StorageBufferView::setNumberOfPoints(int nop)
{
    if (nop != m_numberOfPoints) {
        m_numberOfPoints = nop;
        emit numberOfPointsChanged();
        update();
    }
}

void StorageBufferView::setPointSize(float ps)
{
    if (ps != m_pointSize) {
        m_pointSize = ps;
        emit pointSizeChanged();
        update();
    }
}

void StorageBufferView::setStrideInByte(quint32 stride)
{
    if (stride < 8 * sizeof(float)) {
        qWarning() << "Cannot set stride below 8 * sizeof(float)";
        return;
    }
    if (stride != m_strideInByte) {
        m_strideInByte = stride;
        emit strideInByteChanged();
        update();
    }
}

QSGNode* StorageBufferView::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{

    if (!m_computeItem) {
        qWarning() << "Cannot render without ComputeItem";
        return old;
    }

    // get RHI resource; set to renderer
    QRhiBuffer *buffer = m_computeItem->rhiStorageBufferAt(m_resultBufferIndex);
    if (!buffer) {
        qWarning() << "Cannot render without StorageBuffer";
        return old;
    }

    PointCloudRenderNode *node = static_cast<PointCloudRenderNode *>(old);

    if (!node) {
        node = new PointCloudRenderNode();
        node->setWindow(window());
    }

    node->setNumberOfPoints(m_numberOfPoints);
    node->setPointSize(m_pointSize);
    node->setStrideInByte(m_strideInByte);
    node->setBoundingRect(boundingRect());
    node->setPointBuffer(buffer);

    return node;
}

QSGRenderNode::RenderingFlags PointCloudRenderNode::flags() const
{
    return QSGRenderNode::NoExternalRendering | QSGRenderNode::DepthAwareRendering;
}

QShader PointCloudRenderNode::loadShader(const QString &filename)
{
    QFile shaderFile(filename);
    bool success = shaderFile.open(QIODevice::ReadOnly);
    if (success) {
        return QShader::fromSerialized(shaderFile.readAll());
    }

    qWarning() << "Cannot open shader file:" << filename;
    return QShader();
}

QRhi* PointCloudRenderNode::checkRhi() const
{
    if (!m_window) {
        qWarning("No QQuickWindow present!");
        return nullptr; 
    }
    QSGRendererInterface *renderInterface = m_window->rendererInterface();
    QRhi *rhi = static_cast<QRhi *>(renderInterface->getResource(m_window, QSGRendererInterface::RhiResource));
    if (!rhi) {
        qWarning() << "No QRhi present!";
        return nullptr;
    }
    return rhi;
}


QRhiSwapChain* PointCloudRenderNode::checkSwapChain() const
{
    QSGRendererInterface *renderInterface = m_window->rendererInterface();
    QRhiSwapChain *swapChain = static_cast<QRhiSwapChain *>(renderInterface->getResource(m_window, QSGRendererInterface::RhiSwapchainResource));
    if (!swapChain) {
        qWarning() << "No QRhiSwapChain present!";
        return nullptr;
    }
    return swapChain;
}

void PointCloudRenderNode::prepare()
{
    QRhi *rhi = checkRhi();
    QRhiSwapChain *swapChain = checkSwapChain();
    if (!rhi || !swapChain) {
        return;
    }

    QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();

    if (!m_pipeline) {

        static quint32 ubufSize = 64 + 4 + 4 + 4 + 4; // QMatrix4x4 (mvpProjection) + float (width) + float (height) + float (pointSize) + int (flip);
        m_uniformBuffer.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize));
        m_uniformBuffer->create();

        m_resourceBindings.reset(rhi->newShaderResourceBindings());
        m_resourceBindings->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, m_uniformBuffer.get())
        });
        m_resourceBindings->create();

        m_pipeline.reset(rhi->newGraphicsPipeline());
        m_pipeline->setTopology(QRhiGraphicsPipeline::Points);
        m_pipeline->setShaderStages({
            { QRhiShaderStage::Vertex, loadShader(QLatin1String(":/shaders/pointcloud.vert.qsb")) },
            { QRhiShaderStage::Fragment, loadShader(QLatin1String(":/shaders/pointcloud.frag.qsb")) }
        });

        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({
            { m_strideInByte }
        });
        inputLayout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float4, 4 * sizeof(float) },
        });
        m_pipeline->setVertexInputLayout(inputLayout);
        m_pipeline->setShaderResourceBindings(m_resourceBindings.get());
        m_pipeline->setRenderPassDescriptor(swapChain->newCompatibleRenderPassDescriptor());
        m_pipeline->create();
    }

    const QMatrix4x4 *mvp = matrix();
    QMatrix4x4 mat(*projectionMatrix());

    // multiply to get mvpProjection matrix
    mat = mat * *mvp;

    float width = m_boundingRect.width();
    float height = m_boundingRect.height();
    qint32 flip = rhi->isYUpInFramebuffer() ? 1 : 0;

    resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 0,  64, mat.constData());
    resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 64, 4, &width);
    resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 68, 4, &height);
    resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 72, 4, &m_pointSize);
    resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 76, 4, &flip);
    swapChain->currentFrameCommandBuffer()->resourceUpdate(resourceUpdates);

}

void PointCloudRenderNode::render(const RenderState *state)
{

   if (!m_window) {
        qWarning("Cannot render without window");
        return; 
    }

    if (!m_buffer) {
        qWarning() << "Cannot render without buffer";
        return;
    }

    QRhi *rhi = checkRhi();
    QRhiSwapChain *swapChain = checkSwapChain();
    if (!rhi || !swapChain) {
        return;
    }

    QRhiCommandBuffer *commandBuffer = swapChain->currentFrameCommandBuffer();
    const auto outputPixelSize = swapChain->currentFrameRenderTarget()->pixelSize();
 
    qreal dpr = 1.0;
    const auto screen = m_window->screen();
    if (screen) {
      dpr = screen->devicePixelRatio();
    }
    commandBuffer->setViewport({ 0.0f, 0.0f,
                                 float(m_window->width() * dpr), float(m_window->height() * dpr) });
    commandBuffer->setGraphicsPipeline(m_pipeline.get());
    commandBuffer->setShaderResources();

    QRhiCommandBuffer::VertexInput vbufBinding(m_buffer, 0);
    commandBuffer->setVertexInput(0, 1, &vbufBinding);
    commandBuffer->draw(m_numberOfPoints);

}

void PointCloudRenderNode::releaseResources()
{
    m_pipeline.reset();
    m_resourceBindings.reset();
    m_uniformBuffer.reset();
    m_buffer = nullptr;
}







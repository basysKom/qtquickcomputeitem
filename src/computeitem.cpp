// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "computeitem.h"

#include <QDebug>
#include <QImage>
#include <QFile>
#include <QRunnable>
#include <QGuiApplication>

#include "imagebuffer.h"

class PlainComputeTexture : public QSGTexture
{
public:
    PlainComputeTexture(QRhiTexture *t, const QSize &s)
        : QSGTexture()
        , m_texture(t),
        m_size(s) {}

bool hasMipmaps() const override { return false; }
bool hasAlphaChannel() const override { return false; }
QSize textureSize() const override { return m_size; }
QRhiTexture* rhiTexture() const override { return m_texture; }

void setTexture(QRhiTexture *t, const QSize &s)
{
    m_texture = t;
    m_size = s;
}

qint64 comparisonKey() const override
{
    if (m_texture) {
        return qint64(m_texture);
    }
    return qint64(this);
}

private:
    QRhiTexture* m_texture;
    QSize m_size;
};

class CleanupPlainComputeTexture : public QRunnable
{
public:
    CleanupPlainComputeTexture(QSGTexture *qsgTexture) : m_qsgTexture(qsgTexture) { }
    void run() override { delete m_qsgTexture; }
private:
    QSGTexture *m_qsgTexture; 
};

ComputeItem::ComputeItem(QObject *parent)
    : QObject(parent)
{
    init();
}

ComputeItem::~ComputeItem()
{
    for (const auto sbuf : std::as_const(m_buffers)) {
        if (sbuf) {
            disconnect(sbuf, &ComputeShaderBuffer::bufferChanged, this, nullptr);
            sbuf->setComputeItem(nullptr);  
        } 
    }

    releaseResources();
}

QShader ComputeItem::loadShader(const QString &filename)
{
    QFile shaderFile(filename);
    bool success = shaderFile.open(QIODevice::ReadOnly);
    if (success) {
        return QShader::fromSerialized(shaderFile.readAll());
    }

    qWarning() << "Cannot open shader file:" << filename;
    return QShader();
}

QRhi* ComputeItem::rhiInterface() const
{
    if (!m_window) {
        return nullptr;
    }

    QSGRendererInterface *renderInterface = m_window->rendererInterface();
    QRhi *rhi = static_cast<QRhi *>(renderInterface->getResource(m_window, QSGRendererInterface::RhiResource));
    return rhi;
}


QString ComputeItem::computeShader() const
{
    return m_computeShaderFilename;
}

void ComputeItem::setComputeShader(const QString &filename)
{
    if (filename == m_computeShaderFilename) {
        return;
    }

    m_computeShaderFilename = filename;
    m_dirty = true;
    emit computeShaderChanged();
}

QQmlListProperty<ComputeShaderBuffer> ComputeItem::buffers()
{
    return QQmlListProperty<ComputeShaderBuffer>(this, nullptr, &ComputeItem::append_storageBuffer, nullptr,
        nullptr, nullptr, nullptr, nullptr);
}

void ComputeItem::append_storageBuffer(QQmlListProperty<ComputeShaderBuffer> *list, ComputeShaderBuffer *buffer)
{

    if (!buffer) {
        qWarning() << "Cannot add empty buffer";
        return;
    }

    ComputeItem *computeItem = qobject_cast<ComputeItem *>(list->object);
    if (computeItem) {
        if (buffer->hasComputeItem()) {
            qWarning() << "Cannot add a buffer object that is owned by another ComputeItem";
            return; 
        }

        computeItem->m_buffers.append(buffer);

        connect(buffer, &ComputeShaderBuffer::bufferChanged, computeItem, [computeItem]() {
            computeItem->m_dirty = true;
        }, Qt::DirectConnection );

        buffer->setComputeItem(computeItem);   
        computeItem->m_dirty = true;
    }

}

ComputeShaderBuffer* ComputeItem::bufferAt(int idx) const
{
    if ((idx < 0) || (idx >= m_buffers.size())) {
        qWarning() << "Cannot get buffer: Index out of bounds";
        return nullptr;
    }
    return m_buffers.at(idx);
}


QRhiBuffer* ComputeItem::rhiStorageBufferAt(int idx) const
{
    if ((idx < 0) || (idx >= m_rhiStorageBuffers.size())) {
        qWarning() << "Cannot get QRhiBuffer: Index out of bounds";
        return nullptr;
    }
    return m_rhiStorageBuffers.at(idx);
}

QSGTexture* ComputeItem::qsgTextureAt(int idx) const
{
    if ((idx < 0) || (idx >= m_qsgTextures.size())) {
        qWarning() << "Cannot get QSGTexture: Index out of bounds";
        return nullptr;
    }
    return m_qsgTextures.at(idx);
}


void ComputeItem::componentComplete()
{
    handleDynamicProperties();
}

void ComputeItem::handleDynamicProperties()
{

    const QMetaObject *metaObject = this->metaObject();

    QMetaMethod handlePropertyChangedMethod;
    int handlePropertyChangedMethodIndex = metaObject->indexOfSlot(QMetaObject::normalizedSignature("handlePropertyChanged()"));
    if (handlePropertyChangedMethodIndex != -1) { 
        handlePropertyChangedMethod = metaObject->method(handlePropertyChangedMethodIndex);
    }

    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        QMetaProperty property = metaObject->property(i);
        const QString propertyName = property.name();
        const QVariant propertyValue = this->property(propertyName.toUtf8());

        if (property.hasNotifySignal() && isValidUniformProperty(propertyName, propertyValue)) {
            const QMetaType::Type metaType = static_cast<const QMetaType::Type>(propertyValue.metaType().id());
            UniformProperty uniformProperty;
            uniformProperty.name = propertyName;
            uniformProperty.metaType = metaType;
            if (metaType == QMetaType::Int || metaType == QMetaType::Bool) {
                uniformProperty.i32value = propertyValue.toInt();
                uniformProperty.sizeInBytes = 4;
            } else if (metaType == QMetaType::Double) {
                //QML real and double properties are both stored as QMetaType::Double internally
                // we support only 4 byte float uniforms for now
                uniformProperty.fvalue = propertyValue.toFloat();
                uniformProperty.sizeInBytes = 4;
            }
            m_signalIndexMap.insert(property.notifySignalIndex(), m_uniformPropertyList.count());
            m_uniformPropertyList.append(uniformProperty);
            connect(this, property.notifySignal(), this, handlePropertyChangedMethod);
        }
    }
}

bool ComputeItem::isValidUniformProperty(const QString &name, const QVariant &value) const
{
    // check for ComputeItem's properties
    if (name == QLatin1String("dispatchX") || name == QLatin1String("dispatchY") || name == QLatin1String("dispatchZ")) {
        return false;
    }

    const QMetaType::Type metaType = static_cast<const QMetaType::Type>(value.metaType().id());
    return value.isValid() &&
        (metaType == QMetaType::Int || metaType == QMetaType::Bool || metaType == QMetaType::Double);
}

quint32 ComputeItem::uniformBufferSize() const
{
    quint32 size = 0;
    for (const auto &item : std::as_const(m_uniformPropertyList)) {
        size += item.sizeInBytes;
    }
    return size;
}

void ComputeItem::updateUniformBuffer(QRhiResourceUpdateBatch *updateBatch)
{
    quint32 offset = 0;
    for (const auto &uniformItem : std::as_const(m_uniformPropertyList)) {
        if (uniformItem.metaType == QMetaType::Double) {
            updateBatch->updateDynamicBuffer(m_computeUBuf, offset, uniformItem.sizeInBytes, &uniformItem.fvalue);
        } else {
            updateBatch->updateDynamicBuffer(m_computeUBuf, offset, uniformItem.sizeInBytes, &uniformItem.i32value);
        }
        offset += uniformItem.sizeInBytes;
    }
}


void ComputeItem::handlePropertyChanged()
{

    static const QString changeSignalPostfix = QLatin1String("Changed");
    QObject *senderObj = sender();
    if (!senderObj) {
        return;
    }

    const QMetaObject *senderMetaObj = senderObj->metaObject();
    int signalIndex = senderSignalIndex();

    if (senderMetaObj && m_signalIndexMap.contains(signalIndex)) {

        int uniformIndex = m_signalIndexMap[signalIndex];
        if (uniformIndex >= 0 && uniformIndex < m_uniformPropertyList.count()) {
            UniformProperty *toUpdate = &m_uniformPropertyList[uniformIndex];

            const QMetaMethod metaMethod = senderMetaObj->method(signalIndex);
            const QByteArray propName = metaMethod.name().chopped(changeSignalPostfix.length());
            const QVariant value = senderObj->property(propName);

            if (value.isValid()) {
                if (value.canConvert<int>() && (toUpdate->metaType == QMetaType::Int || toUpdate->metaType == QMetaType::Bool)) {
                    toUpdate->i32value = value.toInt();
                } else if (value.canConvert<float>() && toUpdate->metaType == QMetaType::Double) {
                    toUpdate->fvalue = value.toFloat();
                }
            }
        }

    }
}

void ComputeItem::computeStep()
{
    if (!m_isInitialized) {
        qWarning() << "ComputeItem is not initialized";
        return;
    }

    connect(m_window, &QQuickWindow::beforeRendering, this, [this]() {
        const auto rhi = rhiInterface();
        if (!m_pipelineIsInitialized || m_dirty) {
            initPipeline(rhi);
        }  
        doCompute(rhi);
     }, Qt::SingleShotConnection );

}

void ComputeItem::compute()
{
    if (!m_isInitialized) {
        qWarning() << "ComputeItem is not initialized";
        return;
    }

    if (m_isRunning) {
        return;
    }

    connect(m_window, &QQuickWindow::beforeRendering, this, [this]() {
        const auto rhi = rhiInterface();
        if (!m_pipelineIsInitialized || m_dirty) {
            initPipeline(rhi);
        }
        doCompute(rhi, /* continuously = */ true);
     }, Qt::DirectConnection );

}

void ComputeItem::stop()
{
    if (!m_isInitialized) {
        qWarning() << "ComputeItem is not initialized";
        return;
    }

    if (!m_isRunning) {
        return;
    }

    disconnect(m_window, &QQuickWindow::beforeRendering, this, nullptr);
    m_isRunning = false;
}


void ComputeItem::doCompute(QRhi *rhi, bool continuously)
{

    if (!m_isInitialized || !m_pipelineIsInitialized) {
        qWarning() << "ComputeItem is not initialized";
        return;
    }

    if (m_hasErrors) {
        qWarning() << "Error occurred";
        return;
    }

    QSGRendererInterface *renderInterface = m_window->rendererInterface();
    QRhiSwapChain *swapChain =
        static_cast<QRhiSwapChain *>(renderInterface->getResource(m_window, QSGRendererInterface::RhiSwapchainResource));

    QRhiCommandBuffer *cb = swapChain->currentFrameCommandBuffer();
    QRhiResourceUpdateBatch *updateBatch = rhi->nextResourceUpdateBatch();
    if (m_initialUpdates) {
        updateBatch->merge(m_initialUpdates);
        m_initialUpdates->release();
        m_initialUpdates = nullptr;
    }

    updateUniformBuffer(updateBatch);

    cb->beginComputePass(updateBatch);
    cb->setComputePipeline(m_computePipeline);
    cb->setShaderResources();
    cb->dispatch(m_dispatchX, m_dispatchY, m_dispatchZ);
    cb->endComputePass();

    if (continuously) {
        m_isRunning = true;
        m_window->update();
    }

    emit notifyChange();
}

void ComputeItem::releaseResources()
{

    qDeleteAll(m_releasePool);
    m_releasePool.clear();

    if (m_initialUpdates) {
        m_initialUpdates->release();
        m_initialUpdates = nullptr;
    }

    m_computeUBuf =  nullptr;
    m_computeBindings =  nullptr;
    m_computePipeline =  nullptr;

    m_rhiStorageBuffers.clear();

    if (m_window) {
        for (auto texture : m_qsgTextures) {
            m_window->scheduleRenderJob(new CleanupPlainComputeTexture(texture), QQuickWindow::BeforeSynchronizingStage);
        }
    } else {
        qDeleteAll(m_qsgTextures);
    }
    m_qsgTextures.clear();
}

void ComputeItem::init()
{
    const QWindowList windowList = QGuiApplication::allWindows();
    for (auto w : std::as_const(windowList)) {
        if (m_window = qobject_cast<QQuickWindow *>(w)) {
            break;
        }
    }

    if (!m_window) {
        qWarning() << "No QQuickWindow found; cannot init ComputeItem";
        return;
    }

    m_isInitialized = true;

    connect(m_window, &QQuickWindow::beforeSynchronizing, this, [this]() {
        const auto rhi = rhiInterface();
        initPipeline(rhi);
        doCompute(rhi);
    }, static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::SingleShotConnection) );

}

std::pair<QRhiTexture::Format, quint32> ComputeItem::toRhiTextureFormat(ImageBuffer::TextureFormat format) const
{
    switch (format) {
        case ImageBuffer::RGBA8: return std::pair<QRhiTexture::Format, quint32>(QRhiTexture::RGBA8, 4);
        case ImageBuffer::RGBA16F: return std::pair<QRhiTexture::Format, quint32>(QRhiTexture::RGBA16F, 8);
        case ImageBuffer::RGBA32F: return std::pair<QRhiTexture::Format, quint32>(QRhiTexture::RGBA32F, 16);     
        default: return std::pair<QRhiTexture::Format, quint32>(QRhiTexture::RGBA8, 4);
    }
}


void ComputeItem::initPipeline(QRhi *rhi)
{

    if (!m_isInitialized || !rhi) {
        return;
    }

    if (m_pipelineIsInitialized) {
        releaseResources();
        m_pipelineIsInitialized = false;
    }


    m_hasErrors = false;
    m_initialUpdates = rhi->nextResourceUpdateBatch();

    m_computeUBuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, uniformBufferSize());
    m_computeUBuf->create();
    m_releasePool << m_computeUBuf;

    updateUniformBuffer(m_initialUpdates);

    m_computeBindings = rhi->newShaderResourceBindings();

    std::vector<QRhiShaderResourceBinding> resourceBindingList;
    quint32 binding = 0;
    for (const auto buf : std::as_const(m_buffers)) {
        const auto byteBuffer = buf->buffer();
        qDebug() << "BYTE BUFFER HAS SIZE" << byteBuffer.size();
        if (buf->type() == ComputeShaderBuffer::StorageBuffer) {
            m_qsgTextures << nullptr;
            if (byteBuffer.size() > 0) {
                QRhiBuffer *rhiBuf = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::StorageBuffer | QRhiBuffer::VertexBuffer, byteBuffer.size());
                rhiBuf->create();

                m_releasePool << rhiBuf;
                m_rhiStorageBuffers << rhiBuf;

                m_initialUpdates->uploadStaticBuffer(rhiBuf, byteBuffer.constData());
                resourceBindingList.push_back(QRhiShaderResourceBinding::bufferLoadStore(binding, QRhiShaderResourceBinding::ComputeStage, rhiBuf));
                binding++;
            } else {
                qWarning() << "Cannot upload empty storage buffer";
            }
        } else {
            m_rhiStorageBuffers << nullptr;

            if (buf->type() == ComputeShaderBuffer::Image) {
                ImageBuffer *imageBuffer = qobject_cast<ImageBuffer*>(buf);
                Q_ASSERT(imageBuffer);
                if (byteBuffer.size() > 0) {

                    const auto formatData = toRhiTextureFormat(imageBuffer->textureFormat());
                    const auto textureFormat = formatData.first;
                    const auto bytesPerPixel = formatData.second;
                    const auto imageSize = imageBuffer->imageSize();
                    const auto dataSize = imageSize.width() * imageSize.height() * bytesPerPixel;
                    qDebug() << "Image size is" << dataSize;
                    if (dataSize == byteBuffer.size()) {

                        QRhiTexture *texture = rhi->newTexture(textureFormat, imageSize, 1, QRhiTexture::UsedWithLoadStore);
                        texture->create();
                        m_releasePool << texture;

                        QRhiTextureUploadDescription textureDesc({ 0, 0, { byteBuffer.constData(), quint32(byteBuffer.size()) } });
                        m_initialUpdates->uploadTexture(texture, textureDesc);
                        resourceBindingList.push_back(QRhiShaderResourceBinding::imageLoadStore(binding, QRhiShaderResourceBinding::ComputeStage, texture, 0));
                        binding++;

                        auto qsgTexture = new PlainComputeTexture(texture, imageSize);
                        imageBuffer->setQSGTexture(qsgTexture);
                        m_qsgTextures.push_back(qsgTexture);

                    } else {
                        qWarning() << "Size mismatch; cannot upload image buffer";
                        m_hasErrors = true;
                    }

                } else if (!imageBuffer->imageSource().isEmpty()) {
                    QImage image = QImage(imageBuffer->imageSource()).convertToFormat(QImage::Format_RGBA8888);

                    QRhiTexture *texture = rhi->newTexture(QRhiTexture::RGBA8, image.size(), 1, QRhiTexture::UsedWithLoadStore);
                    texture->create();
                    m_releasePool << texture;

                    m_initialUpdates->uploadTexture(texture, image);
                    resourceBindingList.push_back(QRhiShaderResourceBinding::imageLoadStore(binding, QRhiShaderResourceBinding::ComputeStage, texture, 0));
                    binding++;

                    auto qsgTexture = new PlainComputeTexture(texture, image.size());
                    imageBuffer->setQSGTexture(qsgTexture);
                    m_qsgTextures.push_back(qsgTexture);

                } else {
                    qWarning() << "Cannot upload image data";
                    m_hasErrors = true;
                }

            }
        }
    }

    Q_ASSERT(m_rhiStorageBuffers.length() == m_buffers.length()); // list contains rhi buffers for storage buffers and nullptr for images
    Q_ASSERT(m_qsgTextures.length() == m_buffers.length()); // list contains nullptr for storage buffers and QSGTextures for images

    resourceBindingList.push_back(QRhiShaderResourceBinding::uniformBuffer(binding, QRhiShaderResourceBinding::ComputeStage, m_computeUBuf));

    m_computeBindings->setBindings(resourceBindingList.cbegin(), resourceBindingList.cend());
    m_computeBindings->create();
    m_releasePool << m_computeBindings;

    m_computePipeline = rhi->newComputePipeline();
    m_computePipeline->setShaderResourceBindings(m_computeBindings);
    m_computePipeline->setShaderStage({ QRhiShaderStage::Compute, loadShader(m_computeShaderFilename) });
    m_computePipeline->create();
    m_releasePool << m_computePipeline;

    m_pipelineIsInitialized = true;
    m_dirty = false;

}


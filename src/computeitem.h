// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMetaType>
#include <QQmlListProperty>
#include <QQuickWindow>
#include <QSGTexture>

#include <qqml.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  #include <QRhi>
#else
  #include <private/qrhi_p.h>
#endif

#include "computeshaderbuffer.h"
#include "storagebuffer.h"
#include "imagebuffer.h"

class ComputeItem : public QObject,  public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString computeShader READ computeShader WRITE setComputeShader NOTIFY computeShaderChanged)
    Q_PROPERTY(int dispatchX READ dispatchX WRITE setDispatchX NOTIFY dispatchXChanged)
    Q_PROPERTY(int dispatchY READ dispatchY WRITE setDispatchY NOTIFY dispatchYChanged)
    Q_PROPERTY(int dispatchZ READ dispatchZ WRITE setDispatchZ NOTIFY dispatchZChanged)

    Q_PROPERTY(QQmlListProperty<ComputeShaderBuffer> buffers READ buffers FINAL)
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit ComputeItem(QObject *parent = nullptr);
    ~ComputeItem();

    QString computeShader() const;
    void setComputeShader(const QString &fileName);

    int dispatchX() const { return m_dispatchX; }
    void setDispatchX(int x) { if (x != m_dispatchX) { m_dispatchX = x; emit dispatchXChanged(); } };

    int dispatchY() const { return m_dispatchY; }
    void setDispatchY(int y) { if (y != m_dispatchY) { m_dispatchY = y; emit dispatchYChanged(); } };

    int dispatchZ() const { return m_dispatchZ; }
    void setDispatchZ(int z) { if (z != m_dispatchZ) { m_dispatchY = z; emit dispatchZChanged(); } };;;

    QQmlListProperty<ComputeShaderBuffer> buffers();

    ComputeShaderBuffer* bufferAt(int idx) const;

    template<typename T>
    int indexForBuffer(const T* buffer)
    {
        static_assert(std::is_base_of<ComputeShaderBuffer, T>::value, "T must inherit ComputeShaderBuffer");
        if (buffer == nullptr) {
            return -1;
        }
        int retVal = -1;
        for (int i = 0; i < m_buffers.length(); i++) {
            if (m_buffers[i] == buffer) {
                retVal = i;
                break;
            }
        }
        return retVal;
    }

    QRhiBuffer* rhiStorageBufferAt(int idx) const;
    QSGTexture* qsgTextureAt(int idx) const;

signals:
    void computeShaderChanged();
    void dispatchXChanged();
    void dispatchYChanged();
    void dispatchZChanged();

    void notifyChange();

public slots:
    void computeStep();
    void compute();
    void stop();

private slots:
    void initPipeline(QRhi *rhi);

protected:
    void componentComplete() override;
    void classBegin() override {};

private slots:
    void handlePropertyChanged();

private:
    struct UniformProperty {
        QString name;
        QMetaType::Type metaType;
        union {
            qint32 i32value;
            float fvalue;
        };
        quint32 sizeInBytes;
    };

    static void append_storageBuffer(QQmlListProperty<ComputeShaderBuffer> *list, ComputeShaderBuffer *storageBuffer);

    QShader loadShader(const QString &filename);
    QRhi* rhiInterface() const;
    void releaseResources();
    void init();
    void doCompute(QRhi *rhi, bool continuously = false);

    void handleDynamicProperties();
    bool isValidUniformProperty(const QString &name, const QVariant &value) const;
    quint32 uniformBufferSize() const;
    void updateUniformBuffer(QRhiResourceUpdateBatch *updateBatch);

    // return a pair with the corresponding RhiTexture::Format format and size in bytes
    std::pair<QRhiTexture::Format, quint32> toRhiTextureFormat(ImageBuffer::TextureFormat format) const;

    QVector<ComputeShaderBuffer *> m_buffers;
    QVector<QRhiBuffer *> m_rhiStorageBuffers;
    QVector<QSGTexture *> m_qsgTextures;

    int m_dispatchX { 1 };
    int m_dispatchY { 1 };
    int m_dispatchZ { 1 };

    QQuickWindow *m_window { nullptr };
    QString m_computeShaderFilename;
    bool m_isInitialized { false };
    bool m_pipelineIsInitialized { false };
    bool m_dirty { false };
    bool m_isRunning { false };

    bool m_hasErrors { false };

    // RHI
    QRhiResourceUpdateBatch *m_initialUpdates { nullptr };

    QRhiBuffer *m_computeUBuf { nullptr };
    QRhiShaderResourceBindings *m_computeBindings { nullptr };
    QRhiComputePipeline *m_computePipeline { nullptr };

    QVector<QRhiResource *> m_releasePool;

    QVector<UniformProperty> m_uniformPropertyList;
    // map signalIndex to uniform property index
    QMap<int, int> m_signalIndexMap;

};
// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QString>
#include <QVector>
#include <QQuickItem>
#include <QSGNode>

#include <qqml.h>

#include "computeitem.h"
#include "storagebuffer.h"

class StorageBufferView : public QQuickItem
{
    Q_OBJECT

    /**
     * \property StorageBufferView::computeItem
     *
     * \brief Represents the corresponding ComputeItem
     *
     * This property holds the ComputeItem that operates on the
     * buffer that is visualized by this StorageBufferView
     *
     * \note Ensure that the provided compute item is valid
     */
    Q_PROPERTY(ComputeItem *computeItem READ computeItem WRITE setComputeItem NOTIFY computeItemChanged)

   /**
     * \property StorageBufferView::resultBuffer
     *
     * \brief The result buffer to visualize
     *
     * The StorageBuffer on which the computation is executed
     * The StorageBufferView visualizes the StorageBuffer as a simple point cloud
     *
     * \note Ensure that the provided compute item is valid
     */
    Q_PROPERTY(StorageBuffer* resultBuffer READ resultBuffer WRITE setResultBuffer NOTIFY resultBufferChanged)

    /**
     * \property StorageBufferView::numberOfPoints
     *
     * \brief Renders the given number of points
     */
    Q_PROPERTY(int numberOfPoints READ numberOfPoints WRITE setNumberOfPoints NOTIFY numberOfPointsChanged)

    /**
     * \property StorageBufferView::pointSize
     *
     * \brief The size of one point int the point cloud
     */
    Q_PROPERTY(float pointSize READ pointSize WRITE setPointSize NOTIFY pointSizeChanged)

    // low-level buffer layout

    /**
     * \property StorageBufferView::strideInByte
     *
     * \brief Defines of how many bytes one particle consists. Must at least be 8 * sizeof(float)
     * 
     * The renderer expects a layout in this form
     *     0: x_position of the particle
     *     4: y_position of the particle
     *     8: unused
     *    12: unused
     *    16: red
     *    20: green
     *    24: blue
     *    28: alpha
     * 
     * The unused position can be used as desired. If you have more particle data in your buffer, set the stride accordingly 
     *     
     */
    Q_PROPERTY(quint32 strideInByte READ strideInByte WRITE setStrideInByte NOTIFY strideInByteChanged)
    QML_ELEMENT

public:
    explicit StorageBufferView(QQuickItem *parent = nullptr);
    ~StorageBufferView();

    ComputeItem* computeItem() const;
    void setComputeItem(ComputeItem *item);

    StorageBuffer* resultBuffer() const;
    void setResultBuffer(const StorageBuffer* buffer);

    int numberOfPoints() const { return m_numberOfPoints; }
    void setNumberOfPoints(int nop);

    float pointSize() const { return m_pointSize; }
    void setPointSize(float ps);

    quint32 strideInByte() const { return m_strideInByte; }
    void setStrideInByte(quint32 stride);

protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;

signals:
    void computeItemChanged();
    void resultBufferChanged();
    void numberOfPointsChanged();
    void pointSizeChanged();
    void strideInByteChanged();

private:
    ComputeItem* m_computeItem { nullptr };
    
    int m_resultBufferIndex { -1 };

    int m_numberOfPoints { 0 };
    float m_pointSize { 1.0 };

    quint32 m_strideInByte { 8 * sizeof(float) };

};
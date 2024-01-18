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
    Q_PROPERTY(ComputeItem *computeItem READ computeItem WRITE setComputeItem NOTIFY computeItemChanged)
    Q_PROPERTY(StorageBuffer* resultBuffer READ resultBuffer WRITE setResultBuffer NOTIFY resultBufferChanged)
    Q_PROPERTY(int numberOfPoints READ numberOfPoints WRITE setNumberOfPoints NOTIFY numberOfPointsChanged)
    Q_PROPERTY(float pointSize READ pointSize WRITE setPointSize NOTIFY pointSizeChanged)
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

protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;

signals:
    void computeItemChanged();
    void resultBufferChanged();
    void numberOfPointsChanged();
    void pointSizeChanged();

private:
    ComputeItem* m_computeItem { nullptr };
    
    int m_resultBufferIndex { -1 };

    int m_numberOfPoints { 0 };
    float m_pointSize { 1.0 };

};
// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "computeshaderbuffer.h"
#include "computeitem.h"

ComputeShaderBuffer::ComputeShaderBuffer(QObject *parent)
    : QObject(parent)
{

}

ComputeShaderBuffer::~ComputeShaderBuffer()
{

}

void ComputeShaderBuffer::setComputeItem(ComputeItem *computeItem)
{
    m_computeItem = computeItem;
}
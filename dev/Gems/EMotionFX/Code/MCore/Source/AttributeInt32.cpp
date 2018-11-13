/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "AttributeFloat.h"
#include "AttributeInt32.h"
#include "AttributeBool.h"
#include <MCore/Source/AttributeAllocator.h>


namespace MCore
{
    AZ_CLASS_ALLOCATOR_IMPL(AttributeInt32, AttributeAllocator, 0)

    bool AttributeInt32::InitFrom(const Attribute* other)
    {
        switch (other->GetType())
        {
        case TYPE_ID:
            mValue = static_cast<const AttributeInt32*>(other)->GetValue();
            return true;
        case MCore::AttributeBool::TYPE_ID:
            mValue = static_cast<const AttributeBool*>(other)->GetValue();
            return true;
        case MCore::AttributeFloat::TYPE_ID:
            mValue = static_cast<int32>(static_cast<const AttributeFloat*>(other)->GetValue());
            return true;
        default:
            return false;
        }
        return false;
    }
}   // namespace MCore

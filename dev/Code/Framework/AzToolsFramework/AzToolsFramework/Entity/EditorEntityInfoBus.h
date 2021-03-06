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
#pragma once

#include "EditorEntitySortBus.h"
#include <AzCore/Component/Entity.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/std/containers/unordered_set.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

namespace AzToolsFramework
{
    /**
     * This bus can be used to query any Entity in the EditorEntityContext
     * It contains a hierarchy derived from the transform hierarchy but also provides entity sort information at each level
     * The results returned here are cached and very efficient compared to listening and deriving the data from existing events
     */
    class EditorEntityInfoRequests : public AZ::EBusTraits
    {
    public:
        virtual ~EditorEntityInfoRequests() = default;

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;
        //////////////////////////////////////////////////////////////////////////

        // EntityId -> Parent Info
        virtual AZ::EntityId GetParent() const = 0;

        // EntityId -> Child Info
        virtual EntityIdList GetChildren() const = 0;
        virtual AZ::EntityId GetChild(AZStd::size_t index) const = 0;
        virtual AZStd::size_t GetChildCount() const = 0;
        virtual AZ::u64 GetChildIndex(AZ::EntityId childId) const = 0;

        virtual AZStd::string GetName() const = 0;

        // EntityId -> Slice Info
        virtual AZStd::string GetSliceAssetName() const = 0;
        virtual bool IsSliceEntity() const = 0;
        virtual bool IsSubsliceEntity() const = 0;
        virtual bool IsSliceRoot() const = 0;
        virtual bool IsSubsliceRoot() const = 0;
        virtual bool HasSliceEntityOverrides() const = 0;
        virtual bool HasSliceChildrenOverrides() const = 0;
        virtual bool HasSliceAnyOverrides() const = 0;

        // EntityId -> Order Info
        virtual AZ::u64 GetIndexForSorting() const = 0;

        virtual bool IsSelected() const = 0;
        virtual bool IsVisible() const = 0;
        virtual bool IsHidden() const = 0;
        virtual bool IsLocked() const = 0;
    };
    using EditorEntityInfoRequestBus = AZ::EBus<EditorEntityInfoRequests>;

    /**
     * This bus will be used to notify handlers about changes that occur that will change the result of InfoRequest calls.
     * These will generally line up one to one with the above InfoRequest calls.
     * Calling the appropriate info bus method during or after the notification event will return the new result.
     */
    class EditorEntityInfoNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~EditorEntityInfoNotifications() = default;
        virtual void OnEntityInfoResetBegin() {};
        virtual void OnEntityInfoResetEnd() {};
        virtual void OnEntityInfoUpdatedAddChildBegin(AZ::EntityId /*parentId*/, AZ::EntityId /*childId*/) {};
        virtual void OnEntityInfoUpdatedAddChildEnd(AZ::EntityId /*parentId*/, AZ::EntityId /*childId*/) {};
        virtual void OnEntityInfoUpdatedRemoveChildBegin(AZ::EntityId /*parentId*/, AZ::EntityId /*childId*/) {};
        virtual void OnEntityInfoUpdatedRemoveChildEnd(AZ::EntityId /*parentId*/, AZ::EntityId /*childId*/) {};
        virtual void OnEntityInfoUpdatedOrderBegin(AZ::EntityId /*parentId*/, AZ::EntityId /*childId*/, AZ::u64 /*index*/) {};
        virtual void OnEntityInfoUpdatedOrderEnd(AZ::EntityId /*parentId*/, AZ::EntityId /*childId*/, AZ::u64 /*index*/) {};
        virtual void OnEntityInfoUpdatedSelection(AZ::EntityId /*entityId*/, bool /*selected*/){};
        virtual void OnEntityInfoUpdatedLocked(AZ::EntityId /*entityId*/, bool /*locked*/){};
        virtual void OnEntityInfoUpdatedVisibility(AZ::EntityId /*entityId*/, bool /*visible*/){};
        virtual void OnEntityInfoUpdatedName(AZ::EntityId /*entityId*/, const AZStd::string& /*name*/){};
    };
    using EditorEntityInfoNotificationBus = AZ::EBus<EditorEntityInfoNotifications>;
}

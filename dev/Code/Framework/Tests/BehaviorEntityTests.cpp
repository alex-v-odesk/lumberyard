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

#include "FrameworkApplicationFixture.h"
#include <AzCore/Component/Component.h>
#include <AzFramework/Entity/BehaviorEntity.h>
#include <AzFramework/Entity/GameEntityContextBus.h>

// some fake components to test with
static const AZ::TypeId HatComponentTypeId = "{EADEF936-E987-4BF3-9651-A42251827628}";

class HatConfig : public AZ::ComponentConfig
{
public:
    AZ_RTTI(HatConfig, "{A3129800-43DF-48CA-9BC3-77632241B8ED}", ComponentConfig);
    float m_brimWidth = 1.f;
};

class HatComponent : public AZ::Component
{
public:
    AZ_COMPONENT(HatComponent, HatComponentTypeId);
    static void Reflect(AZ::ReflectContext*) {}
    void Activate() override {}
    void Deactivate() override {}

    bool ReadInConfig(const AZ::ComponentConfig* baseConfig)
    {
        if (auto config = azrtti_cast<const HatConfig*>(baseConfig))
        {
            m_config = *config;
            return true;
        }
        return false;
    }

    bool WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const
    {
        if (auto outConfig = azrtti_cast<HatConfig*>(outBaseConfig))
        {
            *outConfig = m_config;
            return true;
        }
        return false;
    }

    HatConfig m_config;
};

static const AZ::TypeId EarComponentTypeId = "{1F741BC1-451F-445F-891B-1204D6A434D0}";
class EarComponent : public AZ::Component
{
public:
    AZ_COMPONENT(EarComponent, EarComponentTypeId);
    static void Reflect(AZ::ReflectContext*) {}
    static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services) { services.push_back(AZ::Crc32("EarService")); }
    static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services) { services.push_back(AZ::Crc32("EarService")); }
    void Activate() override {}
    void Deactivate() override {}
};

class BehaviorEntityTest
    : public UnitTest::FrameworkApplicationFixture
{
protected:
    void SetUp() override
    {
        m_appDescriptor.m_enableScriptReflection = true;
        FrameworkApplicationFixture::SetUp();

        m_application->RegisterComponentDescriptor(HatComponent::CreateDescriptor());
        m_application->RegisterComponentDescriptor(EarComponent::CreateDescriptor());

        AzFramework::GameEntityContextRequestBus::BroadcastResult(m_rawEntity, &AzFramework::GameEntityContextRequestBus::Events::CreateGameEntity, "Hat");
        m_behaviorEntity = AzFramework::BehaviorEntity(m_rawEntity->GetId());
    }

    void TearDown() override
    {
        FrameworkApplicationFixture::TearDown();
    }

    AZ::Entity* m_rawEntity = nullptr;
    AzFramework::BehaviorEntity m_behaviorEntity;
};

TEST_F(BehaviorEntityTest, FixtureSanityCheck_Succeeds)
{
    EXPECT_NE(nullptr, m_rawEntity);
}

TEST_F(BehaviorEntityTest, GetName_Succeeds)
{
    EXPECT_EQ(m_rawEntity->GetName(), m_behaviorEntity.GetName());
}

TEST_F(BehaviorEntityTest, SetName_Succeeds)
{
    AZStd::string targetName = "Colden";
    m_behaviorEntity.SetName(targetName.c_str());
    EXPECT_EQ(targetName, m_rawEntity->GetName());
}

TEST_F(BehaviorEntityTest, GetOwningContextId_MatchesGameEntityContextId)
{
    AzFramework::EntityContextId gameEntityContextId = AzFramework::EntityContextId::CreateNull();
    AzFramework::GameEntityContextRequestBus::BroadcastResult(gameEntityContextId, &AzFramework::GameEntityContextRequestBus::Events::GetGameEntityContextId);
    EXPECT_EQ(m_behaviorEntity.GetOwningContextId(), gameEntityContextId);
    EXPECT_FALSE(m_behaviorEntity.GetOwningContextId().IsNull());
}

TEST_F(BehaviorEntityTest, Exists_ForActualEntity_True)
{
    EXPECT_TRUE(m_behaviorEntity.Exists());
}

TEST_F(BehaviorEntityTest, Exists_ForDeletedEntity_False)
{
    delete m_rawEntity;
    EXPECT_FALSE(m_behaviorEntity.Exists());
}

TEST_F(BehaviorEntityTest, IsActivated_ForNewEntity_False)
{
    EXPECT_EQ(AZ::Entity::ES_INIT, m_rawEntity->GetState()); // sanity check
    EXPECT_FALSE(m_behaviorEntity.IsActivated());
}

TEST_F(BehaviorEntityTest, IsActivated_ForActivatedEntity_True)
{
    m_rawEntity->Activate();
    EXPECT_EQ(AZ::Entity::ES_ACTIVE, m_rawEntity->GetState()); // sanity check
    EXPECT_TRUE(m_behaviorEntity.IsActivated());
}

TEST_F(BehaviorEntityTest, Activate_Succeeds)
{
    m_behaviorEntity.Activate();
    EXPECT_EQ(AZ::Entity::ES_ACTIVE, m_rawEntity->GetState());
}

TEST_F(BehaviorEntityTest, Deactivate_Succeeds)
{
    m_rawEntity->Activate();
    EXPECT_EQ(AZ::Entity::ES_ACTIVE, m_rawEntity->GetState()); // sanity check

    m_behaviorEntity.Deactivate();
    m_application->Tick(); // some contexts queue entity deactivation on TickBus
    EXPECT_EQ(AZ::Entity::ES_INIT, m_rawEntity->GetState());
}

TEST_F(BehaviorEntityTest, CreateComponent_Succeeds)
{
    AzFramework::BehaviorComponentId componentId = m_behaviorEntity.CreateComponent(HatComponentTypeId);
    EXPECT_TRUE(componentId.IsValid());

    AZ::Component* rawComponent = m_rawEntity->FindComponent(componentId);
    EXPECT_NE(nullptr, rawComponent);
    EXPECT_EQ(HatComponentTypeId, azrtti_typeid(rawComponent));
}

TEST_F(BehaviorEntityTest, CreateComponent_WithNonexistentType_Fails)
{
    AzFramework::BehaviorComponentId componentId = m_behaviorEntity.CreateComponent(AZ::TypeId::CreateNull());
    EXPECT_FALSE(componentId.IsValid());
    EXPECT_EQ(0, m_rawEntity->GetComponents().size());
}

TEST_F(BehaviorEntityTest, CreateComponent_WithIncompatibleType_Fails)
{
    AzFramework::BehaviorComponentId componentId1 = m_behaviorEntity.CreateComponent(EarComponentTypeId);
    AzFramework::BehaviorComponentId componentId2 = m_behaviorEntity.CreateComponent(EarComponentTypeId);
    EXPECT_TRUE(componentId1.IsValid());
    EXPECT_FALSE(componentId2.IsValid());
    EXPECT_EQ(1, m_rawEntity->GetComponents().size());
    EXPECT_NE(nullptr, m_rawEntity->FindComponent(componentId1));
}

TEST_F(BehaviorEntityTest, DestroyComponent_Succeeds)
{
    AZ::Component* rawComponent = m_rawEntity->CreateComponent(HatComponentTypeId);
    EXPECT_NE(nullptr, rawComponent); // sanity check
    
    bool destroyed = m_behaviorEntity.DestroyComponent(rawComponent->GetId());
    EXPECT_TRUE(destroyed);

    EXPECT_TRUE(m_rawEntity->GetComponents().empty());
}

TEST_F(BehaviorEntityTest, GetComponents_ReturnsAllComponents)
{
    AZ::Component* rawComponent1 = m_rawEntity->CreateComponent(HatComponentTypeId);
    AZ::Component* rawComponent2 = m_rawEntity->CreateComponent(EarComponentTypeId);

    AZStd::vector<AzFramework::BehaviorComponentId> componentIds = m_behaviorEntity.GetComponents();
    EXPECT_EQ(2, componentIds.size());
    EXPECT_NE(componentIds.end(), AZStd::find(componentIds.begin(), componentIds.end(), AzFramework::BehaviorComponentId(rawComponent1->GetId())));
    EXPECT_NE(componentIds.end(), AZStd::find(componentIds.begin(), componentIds.end(), AzFramework::BehaviorComponentId(rawComponent2->GetId())));
}

TEST_F(BehaviorEntityTest, FindComponentOfType_Succeeds)
{
    m_rawEntity->CreateComponent(HatComponentTypeId);
    AZ::Component* rawComponent2 = m_rawEntity->CreateComponent(EarComponentTypeId);

    AzFramework::BehaviorComponentId foundComponentId = m_behaviorEntity.FindComponentOfType(azrtti_typeid(rawComponent2));
    EXPECT_EQ(rawComponent2->GetId(), foundComponentId);
}

TEST_F(BehaviorEntityTest, FindComponentOfType_ForNonexistentComponent_ReturnsInvalidComponentId)
{
    AzFramework::BehaviorComponentId foundComponentId = m_behaviorEntity.FindComponentOfType(HatComponentTypeId);
    EXPECT_FALSE(foundComponentId.IsValid());
}

TEST_F(BehaviorEntityTest, FindAllComponentsOfType_Succeeds)
{
    m_rawEntity->CreateComponent(HatComponentTypeId);
    AZ::Component* rawComponent2 = m_rawEntity->CreateComponent(EarComponentTypeId);
    m_rawEntity->CreateComponent(HatComponentTypeId);
    AZ::Component* rawComponent4 = m_rawEntity->CreateComponent(EarComponentTypeId);

    AZStd::vector<AzFramework::BehaviorComponentId> componentIds = m_behaviorEntity.FindAllComponentsOfType(EarComponentTypeId);
    EXPECT_EQ(2, componentIds.size());
    EXPECT_NE(componentIds.end(), AZStd::find(componentIds.begin(), componentIds.end(), AzFramework::BehaviorComponentId(rawComponent2->GetId())));
    EXPECT_NE(componentIds.end(), AZStd::find(componentIds.begin(), componentIds.end(), AzFramework::BehaviorComponentId(rawComponent4->GetId())));
}

TEST_F(BehaviorEntityTest, GetComponentType_Succeeds)
{
    AZ::Component* rawComponent = m_rawEntity->CreateComponent(HatComponentTypeId);
    EXPECT_EQ(azrtti_typeid(rawComponent), m_behaviorEntity.GetComponentType(rawComponent->GetId()));
}

TEST_F(BehaviorEntityTest, SetComponentConfiguration_Succeeds)
{
    HatComponent* rawComponent = m_rawEntity->CreateComponent<HatComponent>();
    HatConfig customConfig;
    customConfig.m_brimWidth = 5.f;

    bool configSuccess = m_behaviorEntity.SetComponentConfiguration(rawComponent->GetId(), customConfig);
    EXPECT_TRUE(configSuccess);
    EXPECT_EQ(customConfig.m_brimWidth, rawComponent->m_config.m_brimWidth);
}

TEST_F(BehaviorEntityTest, GetComponentConfiguration_Succeeds)
{
    HatComponent* rawComponent = m_rawEntity->CreateComponent<HatComponent>();
    rawComponent->m_config.m_brimWidth = 12.f;

    HatConfig retrievedConfig;
    bool configSuccess = m_behaviorEntity.GetComponentConfiguration(rawComponent->GetId(), retrievedConfig);
    EXPECT_TRUE(configSuccess);
    EXPECT_EQ(rawComponent->m_config.m_brimWidth, retrievedConfig.m_brimWidth);
}
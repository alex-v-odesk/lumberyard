/*
* All or portions of this file Copyright(c) Amazon.com, Inc.or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution(the "License").All use of this software is governed by the License,
*or, if provided, by the license below or the license accompanying this file.Do not
* remove or modify any license notices.This file is distributed on an "AS IS" BASIS,
*WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#pragma once

#include "../utilities/ApplicationManagerAPI.h"
#include "native/AssetManager/assetProcessorManager.h"
#include "native/utilities/PlatformConfiguration.h"
#include "native/utilities/assetUtils.h"
#include "native/resourcecompiler/RCBuilder.h"
namespace AssetProcessor
{
    class InternalMockBuilder
        : public InternalRecognizerBasedBuilder
    {
        friend class MockApplicationManager;
    public:
        //////////////////////////////////////////////////////////////////////////
        InternalMockBuilder(const QHash<QString, BuilderIdAndName>& inputBuilderNameByIdMap);

        virtual ~InternalMockBuilder();


        bool InitializeMockBuilder(const AssetRecognizer& assetRecognizer);

        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response);
        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response);

        AssetBuilderSDK::AssetBuilderDesc CreateBuilderDesc(const QString& builderName, const QString& builderId, const AZStd::vector<AssetBuilderSDK::AssetBuilderPattern>& builderPatterns);


        void ResetCounters();
        int GetCreateJobCalls();
        const AssetBuilderSDK::CreateJobsResponse& GetLastCreateJobResponse();
        const AssetBuilderSDK::CreateJobsRequest& GetLastCreateJobRequest();

    protected:
        mutable int m_createJobCallsCount = 0;
        mutable int m_processJobCallsCount = 0;
        mutable AssetBuilderSDK::CreateJobsRequest  m_lastCreateJobRequest;
        mutable AssetBuilderSDK::CreateJobsResponse m_lastCreateJobResponse;
    };

    class MockApplicationManager
        : public AssetProcessor::AssetBuilderInfoBus::Handler
    {
    public:
        MockApplicationManager()
            : m_getMatchingBuildersInfoFunctionCalls(0), m_internalBuilderRegistrationCount(0) {}

        bool RegisterAssetRecognizerAsBuilder(const AssetRecognizer& rec);
        bool UnRegisterAssetRecognizerAsBuilder(const char* name);
        void UnRegisterAllBuilders();

        void GetMatchingBuildersInfo(const AZStd::string& assetPath, AssetProcessor::BuilderInfoList& builderInfoList) override;

        void ResetMatchingBuildersInfoFunctionCalls();
        int GetMatchingBuildersInfoFunctionCalls();
        void ResetMockBuilderCreateJobCalls();
        int GetMockBuilderCreateJobCalls();

        bool GetBuilderByID(const AZStd::string& builderName, AZStd::shared_ptr<InternalMockBuilder>& builder);
        bool GetBuildUUIDFromName(const AZStd::string& builderName, AZ::Uuid& buildUUID);
    protected:
        AZStd::unordered_map<AZStd::string, AZStd::shared_ptr<InternalMockBuilder> > m_internalBuilders;

        struct BuilderFilePatternMatcherAndBuilderDesc
        {
            AssetUtilities::BuilderFilePatternMatcher   m_matcherBuilderPattern;
            AssetBuilderSDK::AssetBuilderDesc           m_builderDesc;
            AZ::Uuid                                    m_internalUuid;
            AZStd::string                               m_internalBuilderName;
        };
        
        AZStd::list<BuilderFilePatternMatcherAndBuilderDesc> m_matcherBuilderPatterns;
        AZStd::unordered_map<AZStd::string, AZ::Uuid > m_internalBuilderUUIDByName;
        int m_getMatchingBuildersInfoFunctionCalls;
        int m_internalBuilderRegistrationCount;
    };

    class MockAssetBuilderInfoHandler
        : public AssetProcessor::AssetBuilderInfoBus::Handler
    {
    public:
        MockAssetBuilderInfoHandler();
        virtual ~MockAssetBuilderInfoHandler();

        //! AssetProcessor::AssetBuilderInfoBus Interface
        void GetMatchingBuildersInfo(const AZStd::string& assetPath, AssetProcessor::BuilderInfoList& builderInfoList) override;
        ////////////////////////////////////////////////

        AssetBuilderSDK::AssetBuilderDesc m_assetBuilderDesc;
        int m_numberOfJobsToCreate = 0;
    };
}


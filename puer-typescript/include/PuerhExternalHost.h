/*
 * Tencent is pleased to support the open source community by making puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#pragma once
#include <string>
#include <functional>
#include <v8.h>
#include "PuerhNamespaceDefine.h"
#include "interoperator/ScriptBackend.hpp"

namespace PUERTS_NAMESPACE
{
	class InspectorHandle;
    class PuerhExternalHost
    {
    public: 
        virtual void Load(const char* path, std::function<void(std::string)> callback) const = 0;
        virtual std::string Resolve(const char* path, const char* parentURL) const = 0;

        virtual void LoadFromJS(const char* path, Function callback) const
        {
            Load(path, [&callback](std::string content) {
                callback.Action<std::string>(content);
            });
        }
        std::string LoadBlockSync(const char* path)
        {
	        std::string content_std = "";
            bool callbacked = false;
            Load(path, [&content_std, &callbacked](std::string content) {
                callbacked = true;
                content_std = content;
            });
            while (!callbacked);

            return content_std;
        }
        
        int32_t InspectorPort = 0;
        bool WaitInspector = false;

        virtual bool OnInspectorMessage(PUERTS_NAMESPACE::InspectorHandle* handle, const std::string& payload) { return false; }
        virtual void OnInspectorOpen(PUERTS_NAMESPACE::InspectorHandle* handle) { }
        virtual void OnInspectorClose(PUERTS_NAMESPACE::InspectorHandle* handle) { }
        virtual ~PuerhExternalHost() {}
    };
}
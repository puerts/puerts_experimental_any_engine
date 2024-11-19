/*
 * Tencent is pleased to support the open source community by making puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#pragma once

#include <stdint.h>
#include <functional>
#include <string>
#include "PuerhNamespaceDefine.h"

#pragma warning(push)
#pragma warning(disable : 4251)
#include "v8.h"
#pragma warning(pop)

namespace PUERTS_NAMESPACE
{
// not for external
class V8InspectorChannel
{
public:
    virtual void DispatchProtocolMessage(const std::string& Message) = 0;

    virtual void OnMessage(std::function<void(const std::string&)> Handler) = 0;

    virtual ~V8InspectorChannel()
    {
    }
};


// class V8InspectorPlugin
// {
// public:
// 	virtual void OnOpen(
// 		void* handle,
// 		v8::Isolate* Isolate,
// 		v8::Local<v8::Context> Context,
// 		std::function<void(const std::string&)> sendMessage
// 	)
// 	{
// 	}
// 	virtual std::string OnHTTPGet(void* handle, std::string path) { return ""; }
// 	virtual bool OnWSMessage(void* handle, std::string payload) { return false; }
// };

class V8Inspector
{
public:
    virtual void Close() = 0;

    virtual void Tick() = 0;

    virtual bool IsAnyInspectorConnected() = 0;

    virtual V8InspectorChannel* CreateV8InspectorChannel() = 0;

    virtual ~V8Inspector()
    {
    }
};


// accept port and v8::Local<v8::Context> pointer, return a new V8Inspector pointer
V8Inspector* CreateV8Inspector(int32_t Port, void* InContextPtr);
};    // namespace PUERTS_NAMESPACE
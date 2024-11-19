/*
 * Tencent is pleased to support the open source community by making puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#pragma once
#include "env/BackendEnvQuickJS.h"
#include "env/BackendEnvV8.h"
#include "env/Util.h"
#include "PuerhExternalHost.h"
#include "interoperator/Binding.hpp"
#include "interoperator/CppObjectMapper.h"
#include "PuerhNamespaceDefine.h"

#include <functional>
#include <unordered_map>

#define DEFINE_JS_HOOK(NAME)

#define RUN_JS_HOOK(NAME, ...)                                                                                                             \
	if (PUERTS_NAMESPACE::PuerhHotfixer::hasJSHook(#NAME))                                                                                           \
	{                                                                                                                                      \
		PUERTS_NAMESPACE::PuerhHotfixer::runJSHook(#NAME, ##__VA_ARGS__);                                                                            \
	}

namespace PUERTS_NAMESPACE
{
	class PuerhPromiseWrap;
	class Puerh
	{
	protected:
		v8::Isolate* MainIsolate;
		v8::UniquePersistent<v8::Context> MainContext;

		BackendEnv* BackendEnv;
		FCppObjectMapper* CppObjectMapper;

		PuerhExternalHost* ExternalHost;
		bool firstTick = true;
	public:
		Puerh(PuerhExternalHost* externalHost);
		~Puerh();
		v8::Isolate* GetIsolate() const { return MainIsolate; }
		v8::Local<v8::Context> GetContext() const { return MainContext.Get(MainIsolate); }

		void Tick();

		bool isLastExecuteSuccess;
		PUERTS_NAMESPACE::PuerhPromiseWrap ExecuteModule(const char* moduleName);
	};
} // namespace PUERTS_NAMESPACE

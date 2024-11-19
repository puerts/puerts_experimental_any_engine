/*
 * Tencent is pleased to support the open source community by making puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#pragma once
#ifdef WITH_QUICKJS

#include "env/BackendEnv.h"
#pragma warning(push, 0)
#include "v8.h"
#pragma warning(pop)

#include "quickjs-msvc.h"
#include "PuerhNamespaceDefine.h"

namespace PUERTS_NAMESPACE
{
	class BackendEnvQuickJS: public BackendEnv
	{
	public:
		// overrides start
		v8::Isolate* CreateIsolate(void* external_quickjs_runtime) override;
		void FreeIsolate() override;
		void OnContextCreated(v8::Isolate* Isolate, v8::Local<v8::Context> Context) override;

		bool ClearModuleCache(v8::Isolate* Isolate, v8::Local<v8::Context> Context, const char* Path) override;
		v8::MaybeLocal<v8::Promise> ImportModule(v8::Local<v8::Context> Context, v8::Local<v8::Value> ReferrerName, v8::Local<v8::String> Specifier) override;
		// overrides end

		virtual ~BackendEnvQuickJS() {}
		BackendEnvQuickJS(PuerhExternalHost* ExternalHost): BackendEnv(ExternalHost) {}
		
		V8_INLINE static BackendEnvQuickJS* Get(v8::Isolate* Isolate)
		{
			return (BackendEnvQuickJS*)Isolate->GetData(1);
		}
	private:

		std::map<std::string, JSModuleDef*> PathToModuleMap;
        std::map<int, std::string> ScriptHashToPathMap;

		static JSModuleDef* QJSHook_LoadModule(JSContext* ctx, const char *name, void *opaque);
		
		static char* QJSHook_ResolveModule(JSContext *ctx, const char *base_name, const char *name, void* opaque);
	};

}
#endif
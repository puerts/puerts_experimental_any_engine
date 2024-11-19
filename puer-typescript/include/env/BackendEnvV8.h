/*
 * Tencent is pleased to support the open source community by making puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#pragma once
#ifndef WITH_QUICKJS
#pragma warning(push, 0)
#include "v8.h"
#pragma warning(pop)

#include <map>
#include <algorithm>
//#include "Log.h"
#include "env/BackendEnv.h"
#include "env/V8InspectorImplEx.h"
#include "env/Util.h"
// #include "interoperator/V8Object.hpp"
#include "PuerhNamespaceDefine.h"

namespace PUERTS_NAMESPACE
{	
	class BackendEnvV8;

	class V8ModuleWrap
	{
	private:
		v8::UniquePersistent<v8::Module> v8Module;
		std::string moduleName;
		v8::Isolate* Isolate;

	public:
		v8::Local<v8::Module> getModule() { return v8Module.Get(Isolate); }
		int getModuleHash() { return v8Module.Get(Isolate)->GetIdentityHash(); }
		const std::string& getModuleName() { return moduleName; }
		int getModuleRequestsLength() { return getModule()->GetModuleRequestsLength(); }
		std::string getModuleRequest(int i) { 
			auto Str = getModule()->GetModuleRequest(i);
			return *v8::String::Utf8Value(Isolate, Str);
		}
		V8ModuleWrap() {}
		virtual ~V8ModuleWrap()
		{
			v8Module.Reset();
		}

		void init(v8::Isolate* isolate, const std::string& _moduleName, const std::string& content)
		{
			this->Isolate = isolate;
			v8::Local<v8::String> Code = v8::String::NewFromUtf8(Isolate, content.c_str()).ToLocalChecked();

			v8::ScriptOrigin Origin(
			    v8::String::NewFromUtf8(Isolate, _moduleName.c_str()).ToLocalChecked(),
			    v8::Integer::New(Isolate, 0), // line offset
			    v8::Integer::New(Isolate, 0), // column offset
			    v8::True(Isolate),            // is crossOrigin
			    v8::Local<v8::Integer>(),     // script id
			    v8::Local<v8::Value>(),       // source map URL
			    v8::False(Isolate),           // is opaque (?)
			    v8::False(Isolate),           // is WASM
			    v8::True(Isolate),            // is ES Module
			    v8::PrimitiveArray::New(Isolate, 10)
			);

			v8::ScriptCompiler::Source Source(Code, Origin);
			v8::Local<v8::Module> Module;

			if (v8::ScriptCompiler::CompileModule(Isolate, &Source).ToLocal(&Module))
			{
				v8Module.Reset(Isolate, Module);
				this->moduleName = _moduleName;
			}
		}
	};


	class BackendEnvV8: public BackendEnv
	{
	public:
		// overrides start
		v8::Isolate* CreateIsolate(void* external_quickjs_runtime) override;
		void FreeIsolate() override;
		void OnContextCreated(v8::Isolate* Isolate, v8::Local<v8::Context> Context) override;

		void LogicTick() override;

		bool ClearModuleCache(v8::Isolate* Isolate, v8::Local<v8::Context> Context, const char* Path) override;
		v8::MaybeLocal<v8::Promise> ImportModule(v8::Local<v8::Context> Context, v8::Local<v8::Value> ReferrerName, v8::Local<v8::String> Specifier) override;
		// overrides end

		virtual ~BackendEnvV8() {
			// all the work should be done in FreeIsolate
		}
		BackendEnvV8(PuerhExternalHost* ExternalHost): BackendEnv(ExternalHost)
		{
			Inspector = nullptr;
		}

		V8_INLINE static BackendEnvV8* Get(v8::Isolate* Isolate)
		{
			return (BackendEnvV8*)Isolate->GetData(1);
		}
	private: 
		V8InspectorEx* Inspector;
		bool InspectorTick() const;
		std::map<void*, InspectorHandle*> InspectorHandles;
		bool firstImport = true;

	public: 
		// Inspector
		void CreateInspector(v8::Isolate* Isolate, const v8::Global<v8::Context>* ContextGlobal, int32_t Port) override;
		void DestroyInspector(v8::Isolate* Isolate, const v8::Global<v8::Context>* ContextGlobal) override;
		int32_t InspectorCount() override 
		{
			return Inspector->IsAnyInspectorConnected() ? 1 : 0;
		}

	private:
		v8::UniquePersistent<v8::Function> v8ImportModuleByJS;
		std::function<V8ModuleWrap*(std::string specifier, std::string referrer)> jsV8GetModuleByNameAndRef;
		std::function<V8ModuleWrap*(int)> jsV8GetModuleByHash;

		void HostInitializeImportMetaObject(v8::Local<v8::Context> Context, v8::Local<v8::Module> Module, v8::Local<v8::Object> meta);
		
	public:
		static void V8Hook_HostInitializeImportMetaObject(v8::Local<v8::Context> Context, v8::Local<v8::Module> Module, v8::Local<v8::Object> meta)
		{
			v8::Isolate* Isolate = Context->GetIsolate();
			BackendEnvV8* mm = BackendEnvV8::Get(Isolate);

			mm->HostInitializeImportMetaObject(Context, Module, meta);
		}

		static v8::MaybeLocal<v8::Promise> V8Hook_DynamicImport(
				v8::Local<v8::Context> Context,
				v8::Local<v8::ScriptOrModule> Referrer,
				v8::Local<v8::String> Specifier
		)
		{
			v8::Isolate* Isolate = Context->GetIsolate();
			BackendEnvV8* mm = BackendEnvV8::Get(Isolate);

			return mm->ImportModule(Context, Referrer->GetResourceName(), Specifier);
		}
        static v8::MaybeLocal<v8::Module> V8Hook_ResolveModule(v8::Local<v8::Context> Context, v8::Local<v8::String> Specifier, v8::Local<v8::Module> Referrer)
		{
			auto Isolate = Context->GetIsolate();
			BackendEnvV8* mm = BackendEnvV8::Get(Isolate);
			v8::String::Utf8Value Specifier_utf8(Isolate, Specifier);

			V8ModuleWrap* refModuleWrap = mm->jsV8GetModuleByHash(Referrer->GetIdentityHash());
			if (refModuleWrap == nullptr) return v8::MaybeLocal<v8::Module>();

			V8ModuleWrap* moduleWrap = mm->jsV8GetModuleByNameAndRef(*Specifier_utf8, refModuleWrap->getModuleName()); 
			if (moduleWrap == nullptr) return v8::MaybeLocal<v8::Module>();
			return moduleWrap->getModule();
		}
	};

}
#endif
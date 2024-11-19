/*
 * Tencent is pleased to support the open source community by making puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#pragma once
#pragma warning(push, 0)
#include "v8.h"
#pragma warning(pop)

#include <map>
#include <algorithm>
#include "PuerhExternalHost.h"
#include "PuerhNamespaceDefine.h"
#include "V8InspectorImplEx.h"
#include "Util.h"

namespace PUERTS_NAMESPACE
{
	class Puerh;

	class InspectorHandle
	{
	protected:
		V8InspectorEx* Inspector;
	public: 
		std::weak_ptr<void> Handle;
		InspectorHandle(std::weak_ptr<void> Handle, V8InspectorEx* Inspector) {
			this->Inspector = Inspector;
			this->Handle = Handle;
		}

		void SendPayload(const std::string& payload) 
		{
#if !defined(WITH_QUICKJS) 
			Inspector->SendMessagePayload(Handle, payload);
#endif
		}

		bool operator=(std::weak_ptr<void> handle2)
		{
			return Handle.lock().get() == handle2.lock().get();
		}
	};

	class BackendEnv
	{
	friend class Puerh;
	protected:
		v8::Isolate* MainIsolate;
		v8::Global<v8::Context> MainContext;
		PuerhExternalHost* ExternalHost;

		v8::Isolate::CreateParams* CreateParams;
	public:
		virtual ~BackendEnv() {
		}
		BackendEnv(PuerhExternalHost* ExternalHost)
		{
			this->ExternalHost = ExternalHost;
		}

		// Inspector
		V8_INLINE static BackendEnv* Get(v8::Isolate* Isolate)
		{
			return (BackendEnv*)Isolate->GetData(1);
		}

		static void GlobalPrepare();

		virtual v8::Isolate* CreateIsolate(void* external_quickjs_runtime) = 0;
		virtual void FreeIsolate() 
		{
			JsPromiseRejectCallback.Reset();
			TimerUpdate.Reset();
			LastEvalResult.Reset();
		}
		virtual void OnContextCreated(v8::Isolate* Isolate, v8::Local<v8::Context> Context) = 0;

		virtual bool ClearModuleCache(v8::Isolate* Isolate, v8::Local<v8::Context> Context, const char* Path) = 0;
		virtual v8::MaybeLocal<v8::Promise> ImportModule(v8::Local<v8::Context> Context, v8::Local<v8::Value> ReferrerName, v8::Local<v8::String> Specifier) = 0;
		
		v8::UniquePersistent<v8::Function> JsPromiseRejectCallback;

		virtual void CreateInspector(v8::Isolate* Isolate, const v8::Global<v8::Context>* ContextGlobal, int32_t Port) { };
		virtual void DestroyInspector(v8::Isolate* Isolate, const v8::Global<v8::Context>* ContextGlobal) {};
		virtual int32_t InspectorCount() { return 1; };
	private:
		v8::UniquePersistent<v8::Function> TimerUpdate;
		int8_t TimerUpdateTryGetCount = 0;
		
	public: 
		virtual void LogicTick() {
			v8::Isolate::Scope isolatescope(MainIsolate);
			v8::HandleScope handleScope(MainIsolate);
			auto Context = MainContext.Get(MainIsolate);
			v8::Context::Scope contextScope(Context);
			
			v8::Local<v8::Function> update = TimerUpdate.Get(MainIsolate);
			if ((update.IsEmpty() || update->IsUndefined()) && TimerUpdateTryGetCount < 100) 
			{
				update = Context->Global()
					->Get(Context, v8::String::NewFromUtf8(MainIsolate, "timerUpdate").ToLocalChecked())
					.ToLocalChecked().As<v8::Function>();
				TimerUpdateTryGetCount++;
			}
			if (!update.IsEmpty() && update->IsFunction())
			{
				if (TimerUpdate.IsEmpty())
					TimerUpdate.Reset(MainIsolate, update);

				v8::TryCatch tryCatch(MainIsolate);

				auto res = update->Call(Context, Context->Global(), 0, nullptr);
				if (res.IsEmpty())
					printf("%s\n", PUERTS_NAMESPACE::Util::ExceptionToString(MainIsolate, tryCatch.Exception()).c_str());
			}
		};

		v8::UniquePersistent<v8::Value> LastEvalResult;
		bool Eval(const char* scriptContent, const char* path)
		{
			v8::Isolate::Scope isolatescope(MainIsolate);
			v8::HandleScope handleScope(MainIsolate);
			auto context = MainContext.Get(MainIsolate);
			v8::Context::Scope contextScope(context);

			v8::TryCatch tryCatch(MainIsolate);
			v8::ScriptOrigin origin(v8::String::NewFromUtf8(MainIsolate, path).ToLocalChecked());

			// Create a string containing the JavaScript source code.
			v8::Local<v8::String> source = v8::String::NewFromUtf8(MainIsolate, scriptContent)
					.ToLocalChecked();

			// Compile the source code.
			v8::MaybeLocal<v8::Script> maybeScript = v8::Script::Compile(context, source, &origin);
			if (maybeScript.IsEmpty())
			{
				LastEvalResult.Reset(MainIsolate, tryCatch.Exception());
				return false;
			}

			// Run the script
			auto maybeResult = maybeScript.ToLocalChecked()->Run(context);
			if (tryCatch.HasCaught())
			{

				LastEvalResult.Reset(MainIsolate, tryCatch.Exception());
				return false;
			}

			if (!maybeResult.IsEmpty())
			{
				LastEvalResult.Reset(MainIsolate, maybeResult.ToLocalChecked());
				return true;

			}
			return false;
		}
	};
}

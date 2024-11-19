/*
* Tencent is pleased to support the open source community by making Puerts available.
* Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
* Puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may be subject to their corresponding license terms.
* This file is subject to the terms and conditions defined in file 'LICENSE', which is part of this source code package.
 */
#ifndef WITH_QUICKJS
#include "env/BackendEnvV8.h"
#include "env/PromiseRejectCallback.hpp"
#include "interoperator/Binding.hpp"
#include "builtin-generated/v8module.h"
#include "builtin-generated/inspector.h"

#pragma warning(push, 0)
#include "libplatform/libplatform.h"
#include "v8.h"
#pragma warning(pop)
using namespace PUERTS_NAMESPACE;
UsingCppType(PUERTS_NAMESPACE::V8ModuleWrap);
UsingCppType(PUERTS_NAMESPACE::PuerhExternalHost);

void EvaluateModule(const v8::FunctionCallbackInfo<v8::Value>& info) //V8ModuleWrap* moduleWrap)
{

	auto Isolate = info.GetIsolate();
	auto Context = Isolate->GetCurrentContext();

	V8ModuleWrap* moduleWrap = v8_impl::Converter<V8ModuleWrap*>::toCpp(Context, info[0]);
	auto moduleChecked = moduleWrap->getModule();

	// v8::TryCatch trycatch(Isolate);
	v8::Maybe<bool> ret = moduleChecked->InstantiateModule(Context, BackendEnvV8::V8Hook_ResolveModule);
	if (/*trycatch.HasCaught() || */ret.IsNothing() || !ret.ToChecked())
	{
		// Isolate->ThrowException(trycatch.Exception());
		return;
	}
	v8::MaybeLocal<v8::Value> evalRetMaybe = moduleChecked->Evaluate(Context);
	if (/*trycatch.HasCaught() || */evalRetMaybe.IsEmpty())
	{
		// Isolate->ThrowException(trycatch.Exception());
		return;
	}
	v8::Local<v8::Value> evalRet = evalRetMaybe.ToLocalChecked();
	// if (trycatch.HasCaught())
	// {
	// 	Isolate->ThrowException(trycatch.Exception());
	// 	return;
	// }
	if (evalRet->IsPromise()) 
	{
		auto promise = evalRet.As<v8::Promise>();
		// mark: seems like it will never be in status kPending.
		// or it will be kPending when top level await is on
		if (promise->State() == v8::Promise::PromiseState::kFulfilled)
		{
			info.GetReturnValue().Set(moduleChecked->GetModuleNamespace());
		} 
	}
	else 
	{
		info.GetReturnValue().Set(moduleChecked->GetModuleNamespace());
	}
}


static std::unique_ptr<v8::Platform> GPlatform;

void PUERTS_NAMESPACE::BackendEnv::GlobalPrepare()
{
	if (!GPlatform)
	{
		GPlatform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(GPlatform.get());
		v8::V8::Initialize();
		
        std::string Flags = "--no-harmony-top-level-await --stack_size=856";
#if PUERTS_DEBUG
        Flags += " --expose-gc";
#endif
#if defined (__APPLE__)
        Flags += " --jitless --no-expose-wasm";
#endif
        v8::V8::SetFlagsFromString(Flags.c_str(), static_cast<int>(Flags.size()));
	}
	PUERTS_NAMESPACE::DefineClass<PUERTS_NAMESPACE::V8ModuleWrap>()
		.Constructor<>()
		.Method("getModuleHash", MakeFunction(&PUERTS_NAMESPACE::V8ModuleWrap::getModuleHash))
		.Method("getModuleName", MakeFunction(&PUERTS_NAMESPACE::V8ModuleWrap::getModuleName))
		.Method("getModuleRequestsLength", MakeFunction(&PUERTS_NAMESPACE::V8ModuleWrap::getModuleRequestsLength))
		.Method("getModuleRequest", MakeFunction(&PUERTS_NAMESPACE::V8ModuleWrap::getModuleRequest))
		.Register();
	PUERTS_NAMESPACE::DefineClass<PUERTS_NAMESPACE::PuerhExternalHost>()
		.Method("Resolve", MakeFunction(&PUERTS_NAMESPACE::PuerhExternalHost::Resolve))
		.Method("LoadFromJS", MakeFunction(&PUERTS_NAMESPACE::PuerhExternalHost::LoadFromJS))
		.Register();
}

v8::Isolate* PUERTS_NAMESPACE::BackendEnvV8::CreateIsolate(void* external_quickjs_runtime)
{
	// initialize isolate and default context
	CreateParams = new v8::Isolate::CreateParams();
	CreateParams->array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	MainIsolate = v8::Isolate::New(*CreateParams);

	return MainIsolate;
}

void PUERTS_NAMESPACE::BackendEnvV8::FreeIsolate()
{
	BackendEnv::FreeIsolate();

	jsV8GetModuleByNameAndRef = nullptr;
	jsV8GetModuleByHash = nullptr;
	v8ImportModuleByJS.Reset();
	MainContext.Reset();
	MainIsolate->Dispose();
	MainIsolate = nullptr;
	
	delete CreateParams->array_buffer_allocator;
	delete CreateParams;
}


void PUERTS_NAMESPACE::BackendEnvV8::LogicTick()
{
	PUERTS_NAMESPACE::BackendEnv::LogicTick();
	InspectorTick();
}

void PUERTS_NAMESPACE::BackendEnvV8::OnContextCreated(v8::Isolate* Isolate, v8::Local<v8::Context> Context)
{
	MainContext.Reset(Isolate, Context);

    Eval(PUER_BUILTIN_INSPECTOR, "puer://builtin/inspector.js");
    Eval(PUER_BUILTIN_MODULE, "puer://builtin/v8module.js");
	auto moduleEvalResult = v8::Local<v8::Object>::Cast(LastEvalResult.Get(Isolate));
	v8ImportModuleByJS.Reset(Isolate, v8::Local<v8::Function>::Cast(moduleEvalResult->Get(
		Context, 
		v8::String::NewFromUtf8(Isolate, "v8ImportModule").ToLocalChecked()
	).ToLocalChecked()));
	jsV8GetModuleByNameAndRef = v8_impl::Converter<std::function<V8ModuleWrap*(std::string specifier, std::string referrer)>>::toCpp(
		Context,
		moduleEvalResult->Get(Context, v8::String::NewFromUtf8(Isolate, "v8GetModuleByNameAndRef").ToLocalChecked()).ToLocalChecked()
	);
	jsV8GetModuleByHash = v8_impl::Converter<std::function<V8ModuleWrap*(int)>>::toCpp(
		Context,
		moduleEvalResult->Get(Context, v8::String::NewFromUtf8(Isolate, "v8GetModuleByIdentityHash").ToLocalChecked()).ToLocalChecked()
	);

	Isolate->SetPromiseRejectCallback(&PromiseRejectCallback<PUERTS_NAMESPACE::BackendEnvV8>);
	Isolate->SetHostInitializeImportMetaObjectCallback(&V8Hook_HostInitializeImportMetaObject);
	Isolate->SetHostImportModuleDynamicallyCallback(&V8Hook_DynamicImport);

	Context->Global()->Set(
		Context, 
		v8::String::NewFromUtf8(Isolate, "__tgjsSetPromiseRejectCallback").ToLocalChecked(), 
		v8::FunctionTemplate::New(Isolate, &SetPromiseRejectCallback<PUERTS_NAMESPACE::BackendEnvV8>)->GetFunction(Context).ToLocalChecked()
	).Check();
}

void PUERTS_NAMESPACE::BackendEnvV8::CreateInspector(
    v8::Isolate* Isolate,
    const v8::Global<v8::Context>* ContextGlobal,
    int32_t Port
)
{
#ifdef THREAD_SAFE
	v8::Locker Locker(Isolate);
#endif
	v8::Isolate::Scope IsolateScope(Isolate);
	v8::HandleScope HandleScope(Isolate);
	v8::Local<v8::Context> Context = ContextGlobal->Get(Isolate);
	v8::Context::Scope ContextScope(Context);

	if (Inspector == nullptr)
	{
		Inspector = (V8InspectorEx*)CreateV8Inspector(Port, &Context);
	}
	Inspector->AddMessageListener([this](std::weak_ptr<void> Handle, const std::string& payload) {
		auto key = Handle.lock().get();
		return this->ExternalHost->OnInspectorMessage(this->InspectorHandles[key], payload);
	});
	Inspector->AddOnOpenListener([this](std::weak_ptr<void> Handle) {
		auto key = Handle.lock().get();
		this->InspectorHandles[key] = new InspectorHandle(Handle, Inspector);
		this->ExternalHost->OnInspectorOpen(this->InspectorHandles[key]);
	});
	Inspector->AddOnCloseListener([this](std::weak_ptr<void> Handle) {
		auto handle = Handle.lock().get();
		auto ptr = this->InspectorHandles[handle];
		this->ExternalHost->OnInspectorClose(ptr);
		this->InspectorHandles.erase(handle);
		delete ptr;
	});
}

void PUERTS_NAMESPACE::BackendEnvV8::DestroyInspector(v8::Isolate* Isolate, const v8::Global<v8::Context>* ContextGlobal)
{
	if (Inspector != nullptr)
	{
#ifdef THREAD_SAFE
		v8::Locker Locker(Isolate);
#endif
		v8::Isolate::Scope IsolateScope(Isolate);
		v8::HandleScope HandleScope(Isolate);
		v8::Local<v8::Context> Context = ContextGlobal->Get(Isolate);
		v8::Context::Scope ContextScope(Context);
		for (auto i = InspectorHandles.begin(); i != InspectorHandles.end(); i++) {
			delete i->second;
		}
		delete Inspector;
		Inspector = nullptr;
	}
}

bool PUERTS_NAMESPACE::BackendEnvV8::InspectorTick() const
{
	if (Inspector != nullptr)
	{
		Inspector->Tick();
		return Inspector->IsAnyInspectorConnected();
	}
	return true;
}

bool PUERTS_NAMESPACE::BackendEnvV8::ClearModuleCache(v8::Isolate* Isolate, v8::Local<v8::Context> Context, const char* Path)
{
	// TODO
	return false;
}

void PUERTS_NAMESPACE::BackendEnvV8::HostInitializeImportMetaObject(v8::Local<v8::Context> Context, v8::Local<v8::Module> Module, v8::Local<v8::Object> meta)
{
	V8ModuleWrap* wrap = jsV8GetModuleByHash(Module->GetIdentityHash());
	if (wrap != nullptr)
	{
		meta->CreateDataProperty(
			Context,
			v8::String::NewFromUtf8(MainIsolate, "url").ToLocalChecked(),
			v8::String::NewFromUtf8(MainIsolate, wrap->getModuleName().c_str()).ToLocalChecked()
		).ToChecked();
	}
}

v8::MaybeLocal<v8::Promise> PUERTS_NAMESPACE::BackendEnvV8::ImportModule(
	v8::Local<v8::Context> Context,
	v8::Local<v8::Value> ReferrerName,
	v8::Local<v8::String> Specifier)
{
	if (firstImport)
	{
		firstImport = false;
		auto Isolate = Context->GetIsolate();
		Context->Global()->Set(
			Context,
			v8::String::NewFromUtf8(Isolate, "__puerEvaluateModuleByModuleWrap").ToLocalChecked(), 
			v8::FunctionTemplate::New(Isolate, &EvaluateModule)->GetFunction(Context).ToLocalChecked()
		).Check();
		Context->Global()->Set(
			Context,
			v8::String::NewFromUtf8(Isolate, "__puerInitModuleWrap").ToLocalChecked(), 
			v8::FunctionTemplate::New(Isolate, [](const v8::FunctionCallbackInfo<v8::Value>& info) {
			    auto Context = info.GetIsolate()->GetCurrentContext();
			        auto moduleWrap = v8_impl::Converter<V8ModuleWrap*>::toCpp(Context, info[0]);
				auto moduleName = v8_impl::Converter<std::string>::toCpp(Context, info[1]);
				auto content = v8_impl::Converter<std::string>::toCpp(Context, info[2]);
				moduleWrap->init(info.GetIsolate(), moduleName, content);
				info.GetReturnValue().Set(v8_impl::Converter<V8ModuleWrap*>::toScript(Context, moduleWrap));
			}, v8::External::New(Isolate, this->ExternalHost))->GetFunction(Context).ToLocalChecked()
		).Check();
		Context->Global()->Set(
			Context,
			v8::String::NewFromUtf8(Isolate, "__puerExternalHost").ToLocalChecked(), 
			v8_impl::Converter<PuerhExternalHost*>::toScript(Context, ExternalHost)
		).Check();
	}
	auto jsImportModule = v8ImportModuleByJS.Get(MainIsolate);
	v8::Local<v8::Value> args[2] = { Specifier, ReferrerName };

	// v8::TryCatch trycatch(MainIsolate);
	auto ret = jsImportModule->Call(Context, Context->Global(), 2, args);
	// if (trycatch.HasCaught())
	// {
	// 	printf("%s %d\n", PUERTS_NAMESPACE::Util::ExceptionToString(MainIsolate, trycatch.Exception()).c_str(), jsImportModule->IsUndefined());
	// 	return v8::MaybeLocal<v8::Promise>{};
	// }

	if (ret.IsEmpty())
	{
		return v8::MaybeLocal<v8::Promise>{};
	}
	else
		return v8::MaybeLocal<v8::Promise>(v8::Local<v8::Promise>::Cast(ret.ToLocalChecked()));
}
#endif
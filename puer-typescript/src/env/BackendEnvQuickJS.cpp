/*
* Tencent is pleased to support the open source community by making Puerts available.
* Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
* Puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may be subject to their corresponding license terms.
* This file is subject to the terms and conditions defined in file 'LICENSE', which is part of this source code package.
 */
#ifdef WITH_QUICKJS
#include "env/BackendEnvQuickJS.h"
//#include "Log.h"
#include "env/PromiseRejectCallback.hpp"

#pragma warning(push, 0)
#include "libplatform/libplatform.h"
#include "v8.h"
#pragma warning(pop)

static std::unique_ptr<v8::Platform> GPlatform;

void PUERTS_NAMESPACE::BackendEnv::GlobalPrepare()
{
	if (!GPlatform)
	{
		GPlatform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(GPlatform.get());
		v8::V8::Initialize();
	}
}

v8::Isolate* PUERTS_NAMESPACE::BackendEnvQuickJS::CreateIsolate(void* external_quickjs_runtime)
{
	// initialize isolate and default context
	CreateParams = new v8::Isolate::CreateParams();
	CreateParams->array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

	MainIsolate = (external_quickjs_runtime == nullptr) ? v8::Isolate::New(*CreateParams) : v8::Isolate::New(external_quickjs_runtime);

	return MainIsolate;
}

void PUERTS_NAMESPACE::BackendEnvQuickJS::FreeIsolate()
{
	BackendEnv::FreeIsolate();
	MainContext.Reset();
	MainIsolate->Dispose();
	MainIsolate = nullptr;

	delete CreateParams->array_buffer_allocator;
	delete CreateParams;
}

void PUERTS_NAMESPACE::BackendEnvQuickJS::OnContextCreated(v8::Isolate* Isolate, v8::Local<v8::Context> Context)
{
	MainContext.Reset(Isolate, Context);

	Isolate->SetPromiseRejectCallback(&PromiseRejectCallback<PUERTS_NAMESPACE::BackendEnv>);

	Context->Global()->Set(Context, v8::String::NewFromUtf8(Isolate, "__tgjsSetPromiseRejectCallback").ToLocalChecked(), v8::FunctionTemplate::New(Isolate, &SetPromiseRejectCallback<PUERTS_NAMESPACE::BackendEnv>)->GetFunction(Context).ToLocalChecked()).Check();
}

bool PUERTS_NAMESPACE::BackendEnvQuickJS::ClearModuleCache(v8::Isolate* Isolate, v8::Local<v8::Context> Context, const char* Path)
{
	std::string key(Path);
	if (key.empty())
	{
		PathToModuleMap.clear();
		return true;
	}
	else
	{
		auto finder = PathToModuleMap.find(key);
		if (finder != PathToModuleMap.end())
		{
			PathToModuleMap.erase(key);
			v8::Isolate::Scope IsolateScope(Isolate);
			v8::HandleScope HandleScope(Isolate);
			JSContext* ctx = Context->context_;
			return (bool)JS_ReleaseLoadedModule(ctx, Path);
		}
	}
	return false;
}

v8::MaybeLocal<v8::Promise> PUERTS_NAMESPACE::BackendEnvQuickJS::ImportModule(v8::Local<v8::Context> Context, v8::Local<v8::Value> ReferrerName, v8::Local<v8::String> Specifier)
{
	auto Isolate = Context->GetIsolate();
	
    JSValue resolving_funcs[2];
    auto ctx = Context->context_;
    auto qjsPromise = JS_NewPromiseCapability(ctx, resolving_funcs);
    auto v8Promise = Isolate->Alloc<v8::Promise>();
    v8Promise->value_ = qjsPromise;
    auto v8MaybeLocalPromise = v8::MaybeLocal<v8::Promise>(v8::Local<v8::Promise>(v8Promise));

    JS_FreeValue(ctx, resolving_funcs[0]);
    JS_FreeValue(ctx, resolving_funcs[1]);

    JS_SetModuleLoaderFunc(Isolate->runtime_, BackendEnvQuickJS::QJSHook_ResolveModule, BackendEnvQuickJS::QJSHook_LoadModule, NULL);

    v8::String::Utf8Value Specifier_utf8(Isolate, Specifier);
    std::string Specifier_std(*Specifier_utf8, Specifier_utf8.length());

    char* resolved_name = BackendEnvQuickJS::QJSHook_ResolveModule(ctx, "", Specifier_std.c_str(), nullptr);
    if (resolved_name == nullptr)
    {
        JS_FullfillOrRejectPromise(ctx, qjsPromise, JS_GetException(Isolate->current_context_->context_), 1);
		return v8MaybeLocalPromise;
    }

    JSModuleDef* EntryModule = BackendEnvQuickJS::QJSHook_LoadModule(ctx, resolved_name, nullptr);
    if (EntryModule == nullptr) 
    {
        JS_FullfillOrRejectPromise(ctx, qjsPromise, JS_GetException(Isolate->current_context_->context_), 1);
		return v8MaybeLocalPromise;
    }

    auto func_obj = JS_DupModule(ctx, EntryModule);
    auto evalRet = JS_EvalFunction(ctx, func_obj);

    v8::Value* val = nullptr;
    if (JS_IsException(evalRet)) {
        
		JS_FullfillOrRejectPromise(ctx, qjsPromise, JS_GetException(Isolate->current_context_->context_), 1);
        
		JS_FreeValue(ctx, evalRet);
		return v8MaybeLocalPromise;

    } else {
        val = Isolate->Alloc<v8::Value>();
        val->value_ = JS_GET_MODULE_NS(ctx, EntryModule);
        JS_FreeValue(ctx, evalRet);
        v8::Local<v8::Value> ns = v8::Local<v8::Value>(val);

        if (ns->IsNullOrUndefined())
        {
            ns = v8::Object::New(Isolate);
        }

        JS_FullfillOrRejectPromise(ctx, qjsPromise, ns->value_, 0);
		return v8MaybeLocalPromise;
    }
}

JSModuleDef* PUERTS_NAMESPACE::BackendEnvQuickJS::QJSHook_LoadModule(JSContext* ctx, const char *name, void *opaque)
{
    JSRuntime *rt = JS_GetRuntime(ctx);
    v8::Isolate* Isolate = (v8::Isolate*)JS_GetRuntimeOpaque(rt);
    BackendEnvQuickJS* mm = BackendEnvQuickJS::Get(Isolate);
    v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
    
    std::string name_std(name, strlen(name));

    auto Iter = mm->PathToModuleMap.find(name_std);
    if (Iter != mm->PathToModuleMap.end())//create and link
    {
        return Iter->second;
    }

    std::string pathForDebug;
    v8::TryCatch TryCatch(Isolate);
	std::string content_std = mm->ExternalHost->LoadBlockSync(name);
    
    if (content_std.empty())
    {
        // should be a exception on mockV8's VM

        // JSValue ex = TryCatch.catched_;
        // TODO rethrow this error will crash, why?
        std::string ErrorMessage = std::string("[Puer003]module not found ") + name;
        JSValue ex = JS_NewStringLen(ctx, ErrorMessage.c_str(), ErrorMessage.length());
        JS_Throw(ctx, ex);
        // there should be a exception in quickjs VM now
        return nullptr;
    }

    const char* Code = content_std.c_str();
    if (Code == nullptr) 
    {
        return nullptr;
    }
    JSValue func_val = JS_Eval(ctx, Code, strlen(Code), name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(func_val)) {
        
        // there should be a exception in quickjs VM now
        return nullptr;
    }

    auto module_ = (JSModuleDef *) JS_VALUE_GET_PTR(func_val);

    auto obj = JS_GetImportMeta(ctx, module_);
    auto atom = JS_NewAtom(ctx, "url");
    JS_SetProperty(ctx, obj, atom, JS_NewString(ctx, ("puer:" + name_std).c_str()));
    JS_FreeValue(ctx, obj);
    JS_FreeAtom(ctx, atom);

    mm->PathToModuleMap[name_std] = module_;

    return module_;
}

char* PUERTS_NAMESPACE::BackendEnvQuickJS::QJSHook_ResolveModule(JSContext *ctx, const char *base_name, const char *name, void* opaque)
{
    JSRuntime *rt = JS_GetRuntime(ctx);
    v8::Isolate* Isolate = (v8::Isolate*)JS_GetRuntimeOpaque(rt);
    BackendEnvQuickJS* mm = BackendEnvQuickJS::Get(Isolate);
    v8::Local<v8::Context> Context = Isolate->GetCurrentContext();

    v8::TryCatch TryCatch(Isolate);
	std::string resolvedSpecifier_std = mm->ExternalHost->Resolve(name, base_name).c_str();
    if (resolvedSpecifier_std.empty()) 
    {
        // should be a exception on mockV8's VM

        // TODO rethrow this error will crash, why?
        // JSValue ex = TryCatch.catched_;
        std::string ErrorMessage = std::string("[Puer002]module not found ") + name;
        JSValue ex = JS_NewStringLen(ctx, ErrorMessage.c_str(), ErrorMessage.length());
        JS_Throw(ctx, ex);
        // there should be a exception in quickjs VM now
        return nullptr;
    }

    size_t size = resolvedSpecifier_std.length();
    char* rname = (char*)js_malloc(ctx, size + 1);
    memcpy(rname, resolvedSpecifier_std.c_str(), size);
    rname[size] = '\0';
    return rname;
}
#endif

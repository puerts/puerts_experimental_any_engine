#include "Puerh.h"
#include "PuerhPromiseWrap.h"
#include "builtin-generated/timer.h"


PUERTS_NAMESPACE::Puerh::Puerh(PuerhExternalHost* externalHost) {
    // env
    PUERTS_NAMESPACE::BackendEnv::GlobalPrepare();
#ifdef WITH_QUICKJS
	BackendEnv = new PUERTS_NAMESPACE::BackendEnvQuickJS(externalHost);
#else
	BackendEnv = new PUERTS_NAMESPACE::BackendEnvV8(externalHost);
#endif
	CppObjectMapper = new PUERTS_NAMESPACE::FCppObjectMapper();
    MainIsolate = BackendEnv->CreateIsolate(nullptr);
    MainIsolate->SetData(1, BackendEnv);
    ExternalHost = externalHost;

#ifdef THREAD_SAFE
    v8::Locker Locker(Isolate);
#endif
    v8::Isolate::Scope isolatescope(MainIsolate);
    v8::HandleScope handleScope(MainIsolate);
    v8::Local<v8::Context> context = v8::Context::New(MainIsolate);
    v8::Context::Scope contextScope(context);
    MainContext.Reset(MainIsolate, context);

    // interop
    CppObjectMapper->Initialize(MainIsolate, context);
    MainIsolate->SetData(0, static_cast<PUERTS_NAMESPACE::ICppObjectMapper*>(CppObjectMapper));
    context->Global()->Set(context, v8::String::NewFromUtf8(MainIsolate, "loadCppType").ToLocalChecked(), v8::FunctionTemplate::New(MainIsolate, [](const v8::FunctionCallbackInfo<v8::Value>& info)
    {
        auto pom = static_cast<PUERTS_NAMESPACE::FCppObjectMapper*>((v8::Local<v8::External>::Cast(info.Data()))->Value());
        pom->LoadCppType(info);
    }, v8::External::New(MainIsolate, CppObjectMapper))->GetFunction(context).ToLocalChecked()).Check();

    BackendEnv->OnContextCreated(MainIsolate, context);
    BackendEnv->Eval(PUER_BUILTIN_TIMER, "puer://builtin/timer.js");
    
    context->Global()
        ->Get(context, v8::String::NewFromUtf8(MainIsolate, "Symbol").ToLocalChecked())
        .ToLocalChecked().As<v8::Object>()
        ->Set(context, v8::String::NewFromUtf8(MainIsolate, "dispose").ToLocalChecked(), CppObjectMapper->SymbolDispose.Get(MainIsolate)).Check();

    context->Global()
        ->Get(context, v8::String::NewFromUtf8(MainIsolate, "Symbol").ToLocalChecked())
        .ToLocalChecked().As<v8::Object>()
        ->Set(context, v8::String::NewFromUtf8(MainIsolate, "alloc").ToLocalChecked(), CppObjectMapper->SymbolAlloc.Get(MainIsolate)).Check();

}

PUERTS_NAMESPACE::Puerh::~Puerh() {
    if (ExternalHost->InspectorPort > 0)
        BackendEnv->DestroyInspector(MainIsolate, &MainContext);

    MainContext.Reset();
    CppObjectMapper->UnInitialize(MainIsolate);
    BackendEnv->FreeIsolate();
    delete BackendEnv;
    delete CppObjectMapper;
}

PUERTS_NAMESPACE::PuerhPromiseWrap PUERTS_NAMESPACE::Puerh::ExecuteModule(const char* moduleName) {
    v8::Isolate::Scope IsolateScope(MainIsolate);
    v8::HandleScope HandleScope(MainIsolate);
    v8::Local<v8::Context> Context = MainContext.Get(MainIsolate);
    v8::Context::Scope ContextScope(Context);

	// inspector
    if (firstTick)
    {
        firstTick = false;
        if (ExternalHost->InspectorPort > 0)
        {
			BackendEnv->CreateInspector(MainIsolate, &MainContext, ExternalHost->InspectorPort);

            if (ExternalHost->WaitInspector)
                while (BackendEnv->InspectorCount() == 0) Tick();
            Tick();
            BackendEnv->TimerUpdateTryGetCount = 0;
        }
    }

    
    return PUERTS_NAMESPACE::PuerhPromiseWrap(
        this, 
        BackendEnv->ImportModule(
            Context, 
            v8::Undefined(MainIsolate), 
            v8::String::NewFromUtf8(MainIsolate, moduleName).ToLocalChecked()
        ).ToLocalChecked()
    );
}


void PUERTS_NAMESPACE::Puerh::Tick() 
{   
    BackendEnv->LogicTick();
}
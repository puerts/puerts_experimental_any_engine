#pragma once
#include "PuerhNamespaceDefine.h"

#pragma warning(push, 0)
#include "v8.h"
#pragma warning(pop)

namespace PUERTS_NAMESPACE
{
    class PuerhPromiseWrap
    {
    private:
        v8::UniquePersistent<v8::Promise::Resolver> _resolver;
        v8::UniquePersistent<v8::Promise> _promise;
        const PUERTS_NAMESPACE::Puerh* puerh;

        friend class PUERTS_NAMESPACE::Puerh;
		PuerhPromiseWrap(Puerh* puerh, v8::Local<v8::Promise> promise)
		{
			this->puerh = puerh;
			_promise.Reset(puerh->GetIsolate(), promise);
		}
    public:
		PuerhPromiseWrap(Puerh* puerh)
		{
			this->puerh = puerh;
			auto MainIsolate = puerh->GetIsolate();
			v8::Isolate::Scope isolatescope(MainIsolate);
			v8::HandleScope handleScope(MainIsolate);
			auto context = puerh->GetContext();
			v8::Context::Scope contextScope(context);
			v8::Local<v8::Promise::Resolver> LocalResolver = v8::Promise::Resolver::New(context).ToLocalChecked();
			v8::Local<v8::Promise> LocalPromise = LocalResolver->GetPromise();
			_resolver.Reset(puerh->GetIsolate(), LocalResolver);
			_promise.Reset(puerh->GetIsolate(), LocalPromise);
		}

        v8::Local<v8::Promise> getPromise() 
        { 
            auto MainIsolate = puerh->GetIsolate();
			auto promise = _resolver.IsEmpty() ? _promise.Get(MainIsolate) : _resolver.Get(MainIsolate)->GetPromise();
            return promise;
        }

        v8::Promise::PromiseState State()
		{
			auto MainIsolate = puerh->GetIsolate();
			v8::Isolate::Scope isolatescope(MainIsolate);
			v8::HandleScope handleScope(MainIsolate);
			auto context = puerh->GetContext();
			v8::Context::Scope contextScope(context);
			auto promise = _resolver.IsEmpty() ? _promise.Get(MainIsolate) : _resolver.Get(MainIsolate)->GetPromise();
            return promise->State();
        }

        bool Resolve(v8::Local<v8::Value> value) 
        {
			auto context = puerh->GetContext();
			if (_resolver.IsEmpty())
				return false;
			auto resolver = _resolver.Get(puerh->GetIsolate());
			return !resolver->Resolve(context, value).IsNothing();
        }
		bool Reject(v8::Local<v8::Value> error) 
        {
			auto context = puerh->GetContext();
			if (_resolver.IsEmpty())
				return false;
			auto resolver = _resolver.Get(puerh->GetIsolate());
			return !resolver->Reject(context, error).IsNothing();
        }

        std::string GetRejectedPromiseStack()
		{
			if (State() != v8::Promise::PromiseState::kRejected)	
			{
				return "";
			}
			auto MainIsolate = puerh->GetIsolate();
			v8::Isolate::Scope isolatescope(MainIsolate);
			v8::HandleScope handleScope(MainIsolate);
			auto context = puerh->GetContext();
			v8::Context::Scope contextScope(context);
			auto promise = _resolver.IsEmpty() ? _promise.Get(MainIsolate) : _resolver.Get(MainIsolate)->GetPromise();
			return PUERTS_NAMESPACE::Util::ExceptionToString(
                puerh->GetIsolate(), 
                promise->Result()
            );
		}
        
		PUERTS_NAMESPACE::v8_impl::Object GetResolvedPromiseObject()
		{
			auto MainIsolate = puerh->GetIsolate();
			v8::Isolate::Scope isolatescope(MainIsolate);
			v8::HandleScope handleScope(MainIsolate);
			auto context = puerh->GetContext();
			v8::Context::Scope contextScope(context);
			if (State() != v8::Promise::PromiseState::kFulfilled)	
			{
				return PUERTS_NAMESPACE::v8_impl::Object(
					context, 
					context->Global()
				);
			}
			auto promise = _resolver.IsEmpty() ? _promise.Get(MainIsolate) : _resolver.Get(MainIsolate)->GetPromise();

			return PUERTS_NAMESPACE::v8_impl::Object(
                context, 
                promise->Result()
            );

		}
    };
}
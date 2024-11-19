/*
 * Tencent is pleased to support the open source community by making puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */
#pragma once
#include "v8.h"
#include <algorithm>
#include <sstream>
#include <map>
#include "PuerhNamespaceDefine.h"

namespace PUERTS_NAMESPACE {
	class Util
	{
	public:
		V8_INLINE static std::string ExceptionToString(v8::Isolate* Isolate, v8::Local<v8::Value> ExceptionValue)
		{
#ifdef THREAD_SAFE
			v8::Locker Locker(Isolate);
#endif
			v8::Isolate::Scope IsolateScope(Isolate);
			v8::HandleScope HandleScope(Isolate);
			v8::String::Utf8Value Exception(Isolate, ExceptionValue);
			const char* StrException = *Exception;
			std::string ExceptionStr(StrException == nullptr ? "" : StrException);
			v8::Local<v8::Message> Message = v8::Exception::CreateMessage(Isolate, ExceptionValue);
			if (Message.IsEmpty())
			{
				// if there is no more detail message, just output Exception
				return ExceptionStr;
			}
			else
			{
				v8::Local<v8::Context> Context(Isolate->GetCurrentContext());

				// output (filename):(line number): (message).
				std::ostringstream stm;
				v8::String::Utf8Value FileName(Isolate, Message->GetScriptResourceName());
				int LineNum = Message->GetLineNumber(Context).FromJust();
				const char* StrFileName = *FileName;
				stm << (StrFileName == nullptr ? "unknow file" : StrFileName) << ":" << LineNum << ": " << ExceptionStr;

				stm << std::endl;

				// output stack trace
				v8::MaybeLocal<v8::Value> MaybeStackTrace = v8::TryCatch::StackTrace(Context, ExceptionValue);
				if (!MaybeStackTrace.IsEmpty())
				{
					v8::String::Utf8Value StackTraceVal(Isolate, MaybeStackTrace.ToLocalChecked());
					stm << std::endl << *StackTraceVal;
				}
				return stm.str();
			}
		}
	};
}
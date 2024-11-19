#pragma once
#if !defined(WITH_QUICKJS)
namespace PUERTS_NAMESPACE
{
    static const char* PUER_BUILTIN_MODULE = R"(
        @PUER_BUILTIN_MODULE_CONTENT@
    )";
}
#endif
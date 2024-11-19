set(puer_dir ${CMAKE_CURRENT_LIST_DIR})

if(NOT DEFINED puer_third_party_websocketpp)
    set(puer_third_party_websocketpp ${puer_dir}/thirdparty/websocketpp)
endif()

if(NOT DEFINED puer_third_party_asio)
    set(puer_third_party_asio ${puer_dir}/thirdparty/asio/include)
endif()

file(READ ${puer_dir}/builtin-src/timer.js PUER_BUILTIN_TIMER_CONTENT)
configure_file(${puer_dir}/builtin-src/timer.in.h ${puer_dir}/include/builtin-generated/timer.h @ONLY)
file(READ ${puer_dir}/builtin-src/v8module.js PUER_BUILTIN_MODULE_CONTENT)
configure_file(${puer_dir}/builtin-src/v8module.in.h ${puer_dir}/include/builtin-generated/v8module.h @ONLY)
file(READ ${puer_dir}/builtin-src/inspector.js PUER_BUILTIN_INSPECTOR_CONTENT)
configure_file(${puer_dir}/builtin-src/inspector.in.h ${puer_dir}/include/builtin-generated/inspector.h @ONLY)

set(puer_sources
	${puer_dir}/src/Puerh.cpp
	${puer_dir}/src/env/BackendEnvV8.cpp
	${puer_dir}/src/env/BackendEnvQuickJS.cpp
	${puer_dir}/src/interoperator/CppObjectMapper.cpp
	${puer_dir}/src/interoperator/DataTransfer.cpp
	${puer_dir}/src/interoperator/JSClassRegister.cpp
)
set(puer_headers
    ${puer_dir}/include
)

set(USE_V8 ON)
if (USE_V8)
    set(puer_definitions
        _WEBSOCKETPP_NO_EXCEPTIONS_
        CUSTOMV8NAMESPACE=v8
    )
    set(puer_sources
        ${puer_sources}
        ${puer_dir}/src/env/V8InspectorImpl.cpp
    )
    set(puer_headers
        ${puer_headers}
        ${puer_dir}/thirdparty/v8_9.4/Inc
        ${puer_third_party_websocketpp}
        ${puer_third_party_asio}
    )
    if (MSVC)
        set(puer_links
            ${puer_dir}/thirdparty/v8_9.4/Lib/Win64DLL/v8.dll.lib
            ${puer_dir}/thirdparty/v8_9.4/Lib/Win64DLL/v8_libplatform.dll.lib
        )
    elseif (ANDROID)
        set(puer_links
            ${puer_dir}/thirdparty/v8_9.4/Lib/Android/${ANDROID_ABI}/libwee8.a
        )
    else()
        set(puer_links
            ${puer_dir}/thirdparty/v8_9.4/Lib/iOS/arm64/libwee8.a
        )
    endif()
else()
    set(puer_definitions
        WITH_QUICKJS
    )
    set(puer_headers
        ${puer_headers}
        ${puer_dir}/thirdparty/quickjs/Inc
    )
    if (MSVC)
        set(puer_links
            ${puer_dir}/thirdparty/quickjs/Lib/Win64DLL/quickjs.dll.lib
        )
    elseif (ANDROID)
        set(puer_links
            ${puer_dir}/thirdparty/quickjs/Lib/Android/${ANDROID_ABI}/libquickjs.a
        )
    else()
        set(puer_links
            ${puer_dir}/thirdparty/quickjs/Lib/iOS/arm64/libquickjs.a
        )
    endif()
endif()

add_library(puer STATIC ${puer_sources})
if(MSVC)
    target_compile_definitions(puer PRIVATE 
        $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>
        $<$<CONFIG:DebugOptimized>:_ITERATOR_DEBUG_LEVEL=0>
    )
    set_property(
        TARGET puer
        PROPERTY
            MSVC_RUNTIME_LIBRARY
            "$<$<CONFIG:Debug>:MultiThreadedDebugDLL>$<$<CONFIG:DebugOptimized>:MultiThreadedDebugDLL>$<$<CONFIG:Release>:MultiThreadedDLL>"
    )
endif()

target_include_directories(puer PRIVATE ${puer_headers})
target_compile_definitions(puer PRIVATE ${puer_definitions})
target_link_libraries(puer PRIVATE ${puer_links})
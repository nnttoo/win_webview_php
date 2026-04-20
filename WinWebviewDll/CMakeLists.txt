cmake_minimum_required(VERSION 3.14)
project(MyProject CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Download WebView2
FetchContent_Declare(
  webview2
  URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.3856.49
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(webview2)

# 2. Path Setup (Setelah MakeAvailable)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WV2_ARCH "x64")
else()
    set(WV2_ARCH "x86")
endif()

set(WV2_BASE "${webview2_SOURCE_DIR}/build/native")
set(WV2_INCLUDE "${WV2_BASE}/include")
set(WV2_LIB_PATH "${WV2_BASE}/${WV2_ARCH}/WebView2Loader.dll.lib") 
set(WV2_DLL_PATH "${WV2_BASE}/${WV2_ARCH}/WebView2Loader.dll")

# 3. Build Target
add_library(${PROJECT_NAME} SHARED src/main.cpp)

# 4. Flags & Definitions (Sesuai project lama kamu)
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

# 5. Include & Link
target_include_directories(${PROJECT_NAME} PRIVATE "${WV2_INCLUDE}")
target_link_libraries(${PROJECT_NAME} PRIVATE 
    "${WV2_LIB_PATH}"
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    version.lib
)

# Copy DLL WebView2 ke folder output agar bisa di-require oleh Node.js
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${WV2_DLL_PATH}"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
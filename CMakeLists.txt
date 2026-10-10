# ============================================================================
#  DSH Cube — 3x3x3 Rubik's Cube / C++17 + OpenGL
#
#  本机工具链（已探测）：C:\mingw64 下的 g++ / cmake / ninja，无 MSVC、无 vcpkg。
#  依赖全部由 FetchContent 拉取，首次 configure 需要联网；之后走本地缓存。
#
#  快速上手：
#     cmake --preset debug
#     cmake --build --preset debug
#     .\build\debug\cube_app.exe
# ============================================================================
cmake_minimum_required(VERSION 3.20)

project(cube
    VERSION 0.1.0
    DESCRIPTION "3x3x3 Rubik's Cube - learning project"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)                       # 严格 -std=c++17

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# 生成 compile_commands.json，给 clangd / VS Code 用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Windows 上可执行文件默认进 build/ 根目录，方便直接双击运行
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------------
#  依赖
# ---------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# GLFW —— 窗口 + 输入 + OpenGL 上下文
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE)

# GLM —— 纯头文件的数学库
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE)

# GLAD —— OpenGL 函数加载器，由它自己的 CMake 脚本在线生成
set(GLAD_PROFILE  "core"     CACHE STRING "" FORCE)
set(GLAD_API      "gl=3.3"   CACHE STRING "" FORCE)
set(GLAD_GENERATOR "cmake"   CACHE STRING "" FORCE)
FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(glfw glm glad)

# ---------------------------------------------------------------------------
#  源文件清单（显式列出，不用 GLOB —— 新增文件要记得加到这里）
#
#  你是一个阶段一个阶段写的，不是一次写完 11 个 cpp。所以下面的"进度守卫"
#  会把还没写的文件从构建里剔除，让项目随时处于可编译状态，并在 configure
#  阶段打印清楚哪些模块已就位、哪些还没动。
#
#  注意：清单以这里为准。如果你把文件名写错了，进度表会打印 [skip] 提醒你 ——
#  别忽略它，那说明构建里少了东西。
# ---------------------------------------------------------------------------
set(CUBE_CORE_SOURCES
    src/math/math.cpp
    src/cube/state.cpp
    src/cube/moves.cpp)

# 注意：cubie.cpp 已从清单里去掉。
# Cubie 只有一个构造函数，它的实现直接写在 state.cpp 里了，
# 单独为它开一个 .cpp 不值当。

set(CUBE_GFX_SOURCES
    src/render/mesh.cpp
    src/render/shader.cpp
    src/render/camera.cpp
    src/input/picking.cpp
    src/input/drag.cpp
    src/anim/animator.cpp)

set(CUBE_APP_SOURCES
    src/app/main.cpp
    src/app/input_state.cpp)

set(CUBE_TEST_SOURCES
    tests/test_main.cpp
    tests/test_cube.cpp)

# 工具程序：有自己独立的 main()，单独编成 exe，不进 cube_core
set(CUBE_TOOL_SOURCES
    tools/math_check.cpp
    tools/cube_check.cpp)

# ---------------------------------------------------------------------------
#  进度守卫
# ---------------------------------------------------------------------------
set(CUBE_WRITTEN "")
set(CUBE_PENDING "")
foreach(relpath IN LISTS CUBE_CORE_SOURCES CUBE_GFX_SOURCES CUBE_APP_SOURCES
                          CUBE_TEST_SOURCES CUBE_TOOL_SOURCES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${relpath}")
        list(APPEND CUBE_WRITTEN "${relpath}")
    else()
        list(APPEND CUBE_PENDING "${relpath}")
    endif()
endforeach()

message(STATUS "")
message(STATUS "  Cube 工程进度")
message(STATUS "  --------------------------------")
foreach(relpath IN LISTS CUBE_WRITTEN)
    message(STATUS "    [ok]   ${relpath}")
endforeach()
foreach(relpath IN LISTS CUBE_PENDING)
    message(STATUS "    [skip] ${relpath}  <- 还没写")
endforeach()
message(STATUS "")

# ---------------------------------------------------------------------------
#  cube_core —— 纯逻辑，零 OpenGL。这一层必须能脱离窗口独立编译和测试。
# ---------------------------------------------------------------------------
set(_core_existing "")
foreach(src IN LISTS CUBE_CORE_SOURCES)
    # 这里是一个曾经踩过的坑，留着给你看：
    #
    #   math.cpp 里必须没有 main()。
    #   曾经在 math.cpp 里放过一个 main() 用来做自测，结果它一进这个库，
    #   就会和 tests/test_main.cpp 里的 main() 撞车 —— 一个可执行文件
    #   只能有一个 main()。
    #
    #   现在 math.cpp 里是纯函数，随便进库，所以下面这段跳过逻辑已经拆掉了。
    #   凡是往 src/ 里加 .cpp，先问自己一句：它有 main() 吗？
    #   有 → 它属于 tools/ 或 tests/，不属于 src/。
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        list(APPEND _core_existing "${src}")
    endif()
endforeach()

if(_core_existing)
    add_library(cube_core STATIC ${_core_existing}
       src/cube/state.cpp
       src/cube/moves.cpp)
    target_include_directories(cube_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

    # cube_core 只用 GLM 的「整数」类型（imat3 / ivec3）。
    # 这跟"零 OpenGL"不矛盾：GLM 的整数部分不碰 OpenGL，
    #   它就是个头文件数学库，用 int 当底层类型，不会有浮点误差。
    # 真正被禁止的是 GLFW 和 glad —— 那两个才会把窗口和显卡拖进来。
    target_link_libraries(cube_core PUBLIC glm::glm)
else()
    message(STATUS "  cube_core 还是空的 —— 先写 src/math/ 和 src/cube/ 吧。")
endif()

# ---------------------------------------------------------------------------
#  math_check —— 数学层验证台（阶段 1 的作业用这个跑）
#
#  两个要点：
#    1. 每新增一个工具程序，要在这里 add_executable 一次 ——
#       VS Code 的 CMake 扩展就是靠这个认出"哪个目标拥有这个 cpp"的。
#       没登记的文件，它会报"未找到目标"（不是生成失败）。
#    2. 逻辑写在 src/ 里（将来并入 cube_core），验证用的 main() 写在 tools/ 里。
#       一个可执行文件只能有一个 main()，物理分开就永不冲突。
#
#  注意：上面 cube_core 那里跳过了 math.cpp，正是为了让这个目标独占它。
#        文件只能被"排除出某个目标"，不能全局排除 ——
#        曾经误用 HEADER_FILE_ONLY，结果两边都不编译它，报 undefined reference。
# ---------------------------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/math_check.cpp")
    # 只编译 main()，实现从 cube_core 里链接过来。
    # 千万别把 src/math/math.cpp 也列在这里 —— 那会让 math.cpp 被编译两遍
    # （一遍进库、一遍进 exe），链接时报 multiple definition。
    add_executable(math_check tools/math_check.cpp)

    target_link_libraries(math_check PRIVATE cube_core glm::glm)
endif()

# cube_check —— 逻辑层验证台（阶段 1 第二块：src/cube/）
# 等到 state.cpp / moves.cpp 出现，这个目标才会被创建。
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/cube_check.cpp"
   AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/cube/state.cpp"
        OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/cube/moves.cpp"))
    add_executable(cube_check tools/cube_check.cpp)
    target_link_libraries(cube_check PRIVATE cube_core glm::glm)
endif()

# state_check —— 只验证 state.cpp，不依赖 moves.cpp（临时验证器，用完可删）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/state_check.cpp")
    add_executable(state_check tools/state_check.cpp)
    target_link_libraries(state_check PRIVATE cube_core glm::glm)
endif()

# ---------------------------------------------------------------------------
#  cube_gfx —— 渲染 / 拾取 / 交互 / 动画
# ---------------------------------------------------------------------------
set(_gfx_existing "")
foreach(src IN LISTS CUBE_GFX_SOURCES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        list(APPEND _gfx_existing "${src}")
    endif()
endforeach()

if(_gfx_existing AND TARGET cube_core)
    add_library(cube_gfx STATIC ${_gfx_existing})
    target_include_directories(cube_gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(cube_gfx PUBLIC cube_core glfw glad::glad glm::glm)
endif()

# ---------------------------------------------------------------------------
#  cube_app —— 可执行文件（main + 输入状态机；求解器按需接进来）
# ---------------------------------------------------------------------------
set(_app_existing "")
foreach(src IN LISTS CUBE_APP_SOURCES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        list(APPEND _app_existing "${src}")
    endif()
endforeach()

if(_app_existing AND TARGET cube_gfx)
    add_executable(cube_app ${_app_existing})
    target_link_libraries(cube_app PRIVATE cube_gfx)
    set(CUBE_HAVE_APP TRUE)
else()
    set(CUBE_HAVE_APP FALSE)
endif()

# ---------------------------------------------------------------------------
#  cube_tests —— 只依赖 cube_core，与图形无关（这是分层设计的意义所在）
# ---------------------------------------------------------------------------
enable_testing()

set(_test_existing "")
foreach(src IN LISTS CUBE_TEST_SOURCES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        list(APPEND _test_existing "${src}")
    endif()
endforeach()

if(_test_existing AND TARGET cube_core)
    add_executable(cube_tests ${_test_existing})
    target_include_directories(cube_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(cube_tests PRIVATE cube_core)
    add_test(NAME cube_tests COMMAND cube_tests)
endif()

# ---------------------------------------------------------------------------
#  编译选项（放在所有 target 建好之后，否则后面的 target 拿不到这些开关）
# ---------------------------------------------------------------------------
set(CUBE_TARGETS "")
foreach(tgt IN ITEMS cube_core cube_gfx cube_app cube_tests)
    if(TARGET ${tgt})
        list(APPEND CUBE_TARGETS ${tgt})
    endif()
endforeach()

if(MSVC)
    foreach(tgt IN LISTS CUBE_TARGETS)
        target_compile_options(${tgt} PRIVATE /W4 /permissive-)
    endforeach()
    if(TARGET cube_app)
        target_compile_definitions(cube_app PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
else()
    # -Wconversion / -Wsign-conversion 在图形代码里会很吵（float/int 转换满地都是），
    # 但它们能提前抓出"格坐标被浮点污染"这类致命问题，所以留着，别关。
    set(CUBE_WARNINGS -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
    foreach(tgt IN LISTS CUBE_TARGETS)
        target_compile_options(${tgt} PRIVATE ${CUBE_WARNINGS})
    endforeach()
endif()

# Windows GUI 子系统：Release 下不开控制台窗口。
# Debug 下保留控制台，方便看 printf 调试输出 —— 这个开关你会经常用到。
if(CUBE_HAVE_APP AND WIN32
   AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
   AND CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(cube_app PRIVATE -mwindows)
endif()

# ---------------------------------------------------------------------------
#  收尾
# ---------------------------------------------------------------------------
message(STATUS "  下一步：读 NOTES.md，然后从 src/math + src/cube 开始动手。")
message(STATUS "")

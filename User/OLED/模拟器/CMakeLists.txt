cmake_minimum_required(VERSION 3.10)
project(OLED_Enhanced_Simulator C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -O2")

# 查找SDL2
find_package(SDL2 REQUIRED)

# 添加可执行文件
add_executable(${PROJECT_NAME} oled_simulator_enhanced.c ../logo.c)

# 链接SDL2库
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

# 数学库（在Linux/macOS上需要）
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# 包含头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# 定义宏以区分模拟器和真实硬件
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    OLED_SIMULATOR=1
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 创建构建目录提示
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "编译器: ${CMAKE_C_COMPILER}")
message(STATUS "SDL2库路径: ${SDL2_LIBRARIES}")
message(STATUS "SDL2头文件路径: ${SDL2_INCLUDE_DIRS}")

# 添加自定义目标用于运行模拟器
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行OLED增强模拟器"
)

# 添加清理目标
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "完全清理构建目录"
)

# 输出有用的信息
message(STATUS "项目配置完成")
message(STATUS "要构建项目，请运行:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make")
message(STATUS "  或者运行 make run 来构建并运行模拟器")
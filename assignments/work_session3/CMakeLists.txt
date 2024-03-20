file(
 GLOB_RECURSE work_session3_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE work_session3_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this work_session3's asset folder to bin when it is built
add_custom_target(copyAssetsA7 COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${work_session3_INC} DESTINATION include/work_session3)
add_executable(work_session3 ${work_session3_SRC} ${work_session3_INC})
target_link_libraries(work_session3 PUBLIC core IMGUI assimp)
target_include_directories(work_session3 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when work_session3 is built
add_dependencies(work_session3 copyAssetsA7)
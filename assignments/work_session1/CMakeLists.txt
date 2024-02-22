file(
 GLOB_RECURSE work_session1_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE work_session1_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this work_session1's asset folder to bin when it is built
add_custom_target(copyAssetsA4 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${work_session1_INC} DESTINATION include/work_session1)
add_executable(work_session1 ${work_session1_SRC} ${work_session1_INC})
target_link_libraries(work_session1 PUBLIC core IMGUI assimp)
target_include_directories(work_session1 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when work_session1 is built
add_dependencies(work_session1 copyAssetsA4)
# Files the executable needs next to it at run time, and the macOS application bundle.
#   OpenAL Soft: loaded at run time (OpenAL32.dll / libopenal.dylib / libopenal.so.1 next to the executable first), else silent.
#   SDL3 (Windows, Linux): next to the executable.
#   gamecontrollerdb.txt (optional): the community gamepad mappings for SDL.
#   qol tier: Mod/Levels/ (the in-game options' strings and, with ZOOSTERS_MODSTREAM_TOOL, their art).
# macOS: Zoosters.app with SDL3 and OpenAL Soft in Contents/Frameworks (no Homebrew needed to run it), the mod files in
# Contents/Resources, ad-hoc signed. Started without arguments it asks for the game folder (src/port/loader/player_launch.cpp).
# The bundle holds no game data.

function(zoosters_runtime_files target)
  set(dir "$<TARGET_FILE_DIR:${target}>")
  if(ZOOSTERS_OPENAL_TARGET)
    if(WIN32)
      set(al_name OpenAL32.dll)
    elseif(APPLE)
      set(al_name libopenal.dylib)
    else()
      set(al_name libopenal.so.1)
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${ZOOSTERS_OPENAL_TARGET}>" "${dir}/${al_name}" VERBATIM)
    if(TARGET OpenAL)          # fetched: build it before the copy
      add_dependencies(${target} OpenAL)
    endif()
  endif()
  if(WIN32 AND ZOOSTERS_NEEDS_SDL3)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "${dir}/SDL3.dll" VERBATIM)
  elseif(NOT APPLE AND ZOOSTERS_NEEDS_SDL3)
    # Linux: SDL3 next to the executable, found through $ORIGIN, so the folder runs on systems without SDL3
    set_property(TARGET ${target} APPEND PROPERTY BUILD_RPATH "\$ORIGIN")
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "${dir}/$<TARGET_SONAME_FILE_NAME:SDL3::SDL3>" VERBATIM)
  endif()
  if(ZOOSTERS_GAMECONTROLLERDB_DIR AND ZOOSTERS_WINDOW STREQUAL "sdl")
    cmake_path(ABSOLUTE_PATH ZOOSTERS_GAMECONTROLLERDB_DIR BASE_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE gcdb)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${gcdb}/gamecontrollerdb.txt" "${dir}/gamecontrollerdb.txt"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${gcdb}/LICENSE" "${dir}/gamecontrollerdb_LICENSE.txt" VERBATIM)
  endif()
  if(ZOOSTERS_TIER_INDEX GREATER_EQUAL 2)
    set(opt "${CMAKE_CURRENT_SOURCE_DIR}/src/port/options")
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${dir}/Mod/Levels"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${opt}/strings/en.txt" "${dir}/Mod/Levels/options_en.txt" VERBATIM)
    if(ZOOSTERS_MODSTREAM_TOOL)
      find_package(Python3 REQUIRED COMPONENTS Interpreter)
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND Python3::Interpreter -I "${ZOOSTERS_MODSTREAM_TOOL}" build "${opt}/assets" "${dir}/Mod/Levels/options.stream" --no-mips --global
        VERBATIM)
    endif()
  endif()
  if(APPLE)
    find_package(Git QUIET)
    set(bundle_args)
    if(ZOOSTERS_NEEDS_SDL3)
      list(APPEND bundle_args "-DSDL3_LIB=$<TARGET_FILE:SDL3::SDL3>")
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${target}> -DAPP=${dir}/Zoosters.app ${bundle_args}
              -DPLIST_IN=${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
              -DGIT=${GIT_EXECUTABLE} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MakeBundle.cmake
      VERBATIM)
  endif()
endfunction()

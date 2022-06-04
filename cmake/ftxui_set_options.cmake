find_program( CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable" )
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
endif()


function(ftxui_set_options library)
  set_target_properties(${library} PROPERTIES OUTPUT_NAME "ftxui-${library}")

  if(CLANG_TIDY_EXE AND FTXUI_CLANG_TIDY)
    set_target_properties(${library}
      PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-warnings-as-errors=*"
    )
  endif()

  target_include_directories(${library}
    PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
      src
  )

  # C++20 is used. We require fold expression at least.
  target_compile_features(${library} PUBLIC cxx_std_20)

  # Force Microsoft Visual Studio to decode sources files in UTF-8. This applies
  # to the library and the library users.
  if (MSVC)
    target_compile_options(${library} PUBLIC "/utf-8")
  endif()

  # Add as many warning as possible:
  if (WIN32)
    if (MSVC)
      target_compile_options(${library} PRIVATE "/W3")
      target_compile_options(${library} PRIVATE "/WX")
      target_compile_options(${library} PRIVATE "/wd4244")
      target_compile_options(${library} PRIVATE "/wd4267")
      target_compile_options(${library} PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
    endif()
    # Force Win32 to UNICODE
    target_compile_definitions(${library} PRIVATE UNICODE _UNICODE)
  else()
    target_compile_options(${library} PRIVATE "-Wall")
    target_compile_options(${library} PRIVATE "-Wextra")
    target_compile_options(${library} PRIVATE "-pedantic")
    target_compile_options(${library} PRIVATE "-Werror")
    target_compile_options(${library} PRIVATE "-Wmissing-declarations")
    target_compile_options(${library} PRIVATE "-Wdeprecated")
    target_compile_options(${library} PRIVATE "-Wshadow")
  endif()

  if (FTXUI_MICROSOFT_TERMINAL_FALLBACK)
    target_compile_definitions(${library}
      PRIVATE "FTXUI_MICROSOFT_TERMINAL_FALLBACK")
  endif()
endfunction()

if (EMSCRIPTEN)
  #string(APPEND CMAKE_CXX_FLAGS " -s ASSERTIONS=1")
  string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s PROXY_TO_PTHREAD")
endif()


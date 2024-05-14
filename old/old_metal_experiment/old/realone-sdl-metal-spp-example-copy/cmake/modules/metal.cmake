find_program(XCRUN xcrun)

execute_process(
    COMMAND ${XCRUN} --sdk macosx --find metal
    OUTPUT_VARIABLE METAL
    OUTPUT_STRIP_TRAILING_WHITESPACE)

find_program(XXD xxd)

function(generate_header src working_directory result)
    get_filename_component(src_base "${src}" NAME_WE)
    get_filename_component(src_ext "${src}" EXT)
    string(SUBSTRING "${src_ext}" 1 -1 src_ext)
    set(dot_h "${src_base}_${src_ext}.h")

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${dot_h}
        COMMAND ${XXD} -i ${src} > ${CMAKE_CURRENT_BINARY_DIR}/${dot_h}
        MAIN_DEPENDENCY ${src}
        WORKING_DIRECTORY "${working_directory}")
    message("Generated ${CMAKE_CURRENT_BINARY_DIR}/${dot_h}")

    set("${result}" "${dot_h}" PARENT_SCOPE)
endfunction()

function(compile_metal_source src result)
    get_filename_component(src_base "${src}" NAME_WE)
    set(dot_metallib "${src_base}.metallib")

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${dot_metallib}
        COMMAND ${METAL} -std=macos-metal2.2 -o ${CMAKE_CURRENT_BINARY_DIR}/${dot_metallib} ${src}
        MAIN_DEPENDENCY ${src}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    message("Generated ${CMAKE_CURRENT_BINARY_DIR}/${dot_metallib}")

    set("${result}" "${dot_metallib}" PARENT_SCOPE)
endfunction()

macro(add_metal_source srclist src)
    set(dot_h)
    generate_header("${src}" "${CMAKE_CURRENT_SOURCE_DIR}" dot_h)

    set(${srclist} ${${srclist}} ${CMAKE_CURRENT_BINARY_DIR}/${dot_h})
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${dot_h} PROPERTIES GENERATED 1)
endmacro()

macro(add_metal_sources srclist)
    foreach(arg IN ITEMS ${ARGN})
        add_metal_source(${srclist} ${arg})
    endforeach()
endmacro()

#
macro(add_compiled_metal_source srclist src)
    set(dot_metallib)
    compile_metal_source("${src}" dot_metallib)

    set(dot_h)
    generate_header("${dot_metallib}" "${CMAKE_CURRENT_BINARY_DIR}" dot_h)

    set(${srclist} ${${srclist}} ${CMAKE_CURRENT_BINARY_DIR}/${dot_h})
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${dot_h} PROPERTIES GENERATED 1)
endmacro()

macro(add_compiled_metal_sources srclist)
    foreach(arg IN ITEMS ${ARGN})
        add_compiled_metal_source(${srclist} ${arg})
    endforeach()
endmacro()
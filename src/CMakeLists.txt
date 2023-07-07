FILE(GLOB SRC_FILES *.cpp)
add_library(utils SHARED ${SRC_FILES})

function(define_file_basename target)
    get_target_property(source_files ${target} SOURCES)
    foreach (sourcefile ${source_files})
        get_property(defs SOURCE ${sourcefile} PROPERTY COMPILE_DEFINITIONS)
        get_filename_component(basename ${sourcefile} NAME)
        list(APPEND defs "__FILENAME__=\"${basename}\"")
        set_property(SOURCE ${sourcefile} PROPERTY COMPILE_DEFINITIONS ${defs})
    endforeach ()
endfunction()
define_file_basename(utils)
#ast操纵
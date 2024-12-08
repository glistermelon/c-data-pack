function(pack_data target_name resource_dir)
    if (NOT TARGET ${target_name})
        message(FATAL_ERROR "Invalid target passed to packdata: '${target_name}'")
    endif()
    # set(files_joined)
    # math(EXPR argc_m1 "${ARGC} - 1")
    # foreach(i RANGE 1 ${argc_m1})
    #     list(APPEND files_joined "${ARGV${i}}")
    # endforeach()
    add_custom_target(
        docoolshit ALL
        DEPENDS target_name
        COMMAND "${CMAKE_SOURCE_DIR}/packdata.exe" "$<TARGET_FILE:${target_name}>" ${resource_dir}
        COMMENT "Packing data for target ${target_name}"
    )
endfunction()
############################################################################
# Various macros for 3DS homebrews tools
#
# Credit: 3ds-cmake
############################################################################

if(NOT 3DS)
    message(WARNING "Those tools can only be used if you are using the 3DS toolchain file. Please erase this build directory or create another one, and then use -DCMAKE_TOOLCHAIN_FILE=DevkitArm3DS.cmake when calling cmake for the 1st time. For more information, see the Readme.md for more information.")
endif()

get_filename_component(__tools3dsdir ${CMAKE_CURRENT_LIST_FILE} PATH) # Used to locate files to be used with configure_file

message(STATUS "Looking for 3ds tools...")

##############
## 3DSXTOOL ##
##############
if(NOT _3DSXTOOL)
    # message(STATUS "Looking for 3dsxtool...")
    find_program(_3DSXTOOL 3dsxtool ${DEVKITARM}/bin)
    if(_3DSXTOOL)
        message(STATUS "3dsxtool: ${_3DSXTOOL} - found")
    else()
        message(WARNING "3dsxtool - not found")
    endif()
endif()


##############
## SMDHTOOL ##
##############
if(NOT SMDHTOOL)
    # message(STATUS "Looking for smdhtool...")
    find_program(SMDHTOOL smdhtool ${DEVKITARM}/bin)
    if(SMDHTOOL)
        message(STATUS "smdhtool: ${SMDHTOOL} - found")
    else()
        message(WARNING "smdhtool - not found")
    endif()
endif()


###############
##  3DSLINK  ##
###############
if(NOT _3DSLINK)
    # message(STATUS "Looking for 3dslink...")
    find_program(_3DSLINK 3dslink ${DEVKITARM}/bin)
    if(_3DSLINK)
        message(STATUS "3dslink: ${_3DSLINK} - found")
    else()
        message(WARNING "3dslink - not found")
    endif()
endif()

###################
### EXECUTABLES ###
###################


function(__add_smdh target APP_TITLE APP_DESCRIPTION APP_AUTHOR APP_ICON)
    if(BANNERTOOL AND NOT FORCE_SMDHTOOL)
        set(__SMDH_COMMAND ${BANNERTOOL} makesmdh -s ${APP_TITLE} -l ${APP_DESCRIPTION}  -p ${APP_AUTHOR} -i ${APP_ICON} -o ${CMAKE_CURRENT_BINARY_DIR}/${target})
    else()
        set(__SMDH_COMMAND ${SMDHTOOL} --create ${APP_TITLE} ${APP_DESCRIPTION} ${APP_AUTHOR} ${APP_ICON} ${CMAKE_CURRENT_BINARY_DIR}/${target})
    endif()
    add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target}
                        COMMAND ${__SMDH_COMMAND}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                        DEPENDS ${APP_ICON}
                        VERBATIM
    )
endfunction()

function(add_3dsx_target target)
    get_filename_component(target_we ${target} NAME_WE)
    if((NOT (${ARGC} GREATER 1 AND "${ARGV1}" STREQUAL "NO_SMDH") ) OR (${ARGC} GREATER 3) )
        if(${ARGC} GREATER 3)
            set(APP_TITLE ${ARGV1})
            set(APP_DESCRIPTION ${ARGV2})
            set(APP_AUTHOR ${ARGV3})
        endif()
        if(${ARGC} EQUAL 5)
            set(APP_ICON ${ARGV4})
        endif()
        if(NOT APP_TITLE)
            set(APP_TITLE ${target})
        endif()
        if(NOT APP_DESCRIPTION)
            set(APP_DESCRIPTION "Built with devkitARM & libctru")
        endif()
        if(NOT APP_AUTHOR)
            set(APP_AUTHOR "Unspecified Author")
        endif()
        if(NOT APP_ICON)
            if(EXISTS ${target}.png)
                set(APP_ICON ${target}.png)
            elseif(EXISTS icon.png)
                set(APP_ICON icon.png)
            elseif(CTRULIB)
                set(APP_ICON ${CTRULIB}/default_icon.png)
            else()
                message(FATAL_ERROR "No icon found ! Please use NO_SMDH or provide some icon.")
            endif()
        endif()
        if( NOT ${target_we}.smdh)
            __add_smdh(${target_we}.smdh ${APP_TITLE} ${APP_DESCRIPTION} ${APP_AUTHOR} ${APP_ICON})
        endif()
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target_we}.3dsx
                            COMMAND ${_3DSXTOOL} $<TARGET_FILE:${target}> ${CMAKE_CURRENT_BINARY_DIR}/${target_we}.3dsx --smdh=${CMAKE_CURRENT_BINARY_DIR}/${target_we}.smdh --romfs=../${ARGV5}
                            DEPENDS ${target} ${CMAKE_CURRENT_BINARY_DIR}/${target_we}.smdh
                            VERBATIM
        )
    else()
        message(STATUS "No smdh file will be generated")
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target_we}.3dsx
                            COMMAND ${_3DSXTOOL} $<TARGET_FILE:${target}> ${CMAKE_CURRENT_BINARY_DIR}/${target_we}.3dsx --romfs=../${ARGV5}
                            DEPENDS ${target}
                            VERBATIM
        )
    endif()
    add_custom_target(${target_we}_3dsx ALL SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${target_we}.3dsx)
    set_target_properties(${target} PROPERTIES LINK_FLAGS "-specs=3dsx.specs")
endfunction()
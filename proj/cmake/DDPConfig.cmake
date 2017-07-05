if( NOT TARGET DDP )
    get_filename_component( DDP_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )

    FILE(GLOB DDP_SOURCES ${DDP_SOURCE_PATH}/*.cpp)

    add_library( DDP ${DDP_SOURCES} )
    target_include_directories( DDP PUBLIC "${DDP_SOURCE_PATH}" )
    target_include_directories( DDP PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../../../Asio/src" )
    message(STATUS ${DDP_SOURCE_PATH} )

    # If Cinder included from this block:

    target_include_directories( DDP SYSTEM BEFORE PUBLIC "${CINDER_PATH}/include" )

    if( NOT TARGET cinder )
        include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
        find_package( cinder REQUIRED PATHS
                "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
                "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
    endif()

    target_link_libraries( DDP PRIVATE cinder )
endif()

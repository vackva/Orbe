if(MSVC)
    set(OPEN_BLAS_VERSION 0.3.26)
    set(OPEN_BLAS_DIR_NAME "OpenBLAS-${OPEN_BLAS_VERSION}")
    set(OPEN_BLAS_ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/modules/${OPEN_BLAS_DIR_NAME})

    if(EXISTS ${OPEN_BLAS_ROOTDIR}/)
        message(STATUS "OpenBLAS found at ${OPEN_BLAS_ROOTDIR}")
    else()
        file(MAKE_DIRECTORY ${OPEN_BLAS_ROOTDIR}/)
        message(STATUS "OpenBLAS library not found - preparing to download.")

        set(OPEN_BLAS_LIB_NAME "OpenBLAS-${OPEN_BLAS_VERSION}-x64.zip")
        set(OPEN_BLAS_URL "https://github.com/OpenMathLib/OpenBLAS/releases/download/v${OPEN_BLAS_VERSION}/${OPEN_BLAS_LIB_NAME}")
        set(OPEN_BLAS_PATH ${CMAKE_BINARY_DIR}/import/${OPEN_BLAS_LIB_NAME})

        message(STATUS "Downloading: ${OPEN_BLAS_URL}")

        file(DOWNLOAD ${OPEN_BLAS_URL} ${OPEN_BLAS_PATH}
                STATUS OPEN_BLAS_DOWNLOAD_STATUS
                SHOW_PROGRESS)

        list(GET OPEN_BLAS_DOWNLOAD_STATUS 0 OPEN_BLAS_DOWNLOAD_STATUS_NO)

        if(OPEN_BLAS_DOWNLOAD_STATUS_NO EQUAL 0)
            message(STATUS "Successfully downloaded OpenBLAS library.")
            file(ARCHIVE_EXTRACT
                    INPUT ${OPEN_BLAS_PATH}
                    DESTINATION ${OPEN_BLAS_ROOTDIR})

            message(STATUS "Extracted OpenBLAS to ${OPEN_BLAS_ROOTDIR}")
        else()
            message(WARNING "Failed to download OpenBLAS. Check your internet connection and try again.")
            file(REMOVE_RECURSE ${OPEN_BLAS_ROOTDIR})
            file(REMOVE ${OPEN_BLAS_PATH})
        endif()
    endif()

    get_directory_property(hasParent PARENT_DIRECTORY)
    if(hasParent)
        set(OPEN_BLAS_VERSION_USED "${OPEN_BLAS_VERSION}" PARENT_SCOPE)
    else ()
        set(OPEN_BLAS_VERSION_USED "${OPEN_BLAS_VERSION}")
    endif()

endif ()

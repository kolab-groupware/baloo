add_executable(baloo_file_extractor_dummy extractor.cpp)
set_target_properties(baloo_file_extractor_dummy
    PROPERTIES OUTPUT_NAME baloo_file_extractor
)

target_link_libraries(baloo_file_extractor_dummy
  ${KDE4_KDECORE_LIBS}
)
if(WIN32)
    set_target_properties(baloo_file_extractor_dummy
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
add_definitions("-DBALOO_TEMP_PATH=\\\"${CMAKE_CURRENT_BINARY_DIR}\\\"")
kde4_add_unit_test(fileindexingjobtest NOGUI fileindexingjobtest.cpp
                                             ../../fileindexingjob.cpp)
set_target_properties(fileindexingjobtest PROPERTIES COMPILE_FLAGS -DBALOO_FILE_EXPORT=)
target_link_libraries(fileindexingjobtest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KFILEMETADATA_LIBRARY}
  ${XAPIAN_LIBRARIES}
)

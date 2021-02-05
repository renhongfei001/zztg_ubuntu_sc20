# Copyright (C) 2016 Verizon. All Rights Reserved.

cmake_minimum_required(VERSION 2.8)

find_library(DL_LIBS dl PATHS /usr/lib/)

## TODO: move it out of 'if(lcov etc.)' block. Add standalone check for 'doxygen'
function(DOXYGEN_TARGET)
  set(DOC_OUT_PATH doxygen_doc)

  ADD_CUSTOM_TARGET("doxygen_doc"
    COMMAND doxygen ${CMAKE_SOURCE_DIR}/doxyfile
    COMMENT "Generating API documentation with Doxygen"
  )
endfunction()


if (ENABLE_TEST)
  find_library(CUNIT_LIBS cunit PATHS /usr/lib/)

  set(CMAKE_C_FLAGS_COVERAGE
      "-g -O0 -W -fprofile-arcs -ftest-coverage")

  #TODO: need to add user-friendly error messages in case if something not found
  FIND_PROGRAM( GCOV_BIN gcov )
  FIND_PROGRAM( LCOV_BIN lcov )
  FIND_PROGRAM( GENHTML_BIN genhtml )

  if(GCOV_BIN AND LCOV_BIN AND GENHTML_BIN)
    ## function to simplify code coverage collection
    function(ADD_COVERAGE_TARGET _projectName _testName)
      message("CMAKE_CTEST_COMMAND === ${CMAKE_CTEST_COMMAND}")
      set(REPORT_PATH lcov_report)

      ADD_CUSTOM_TARGET("cover-${_projectName}"
        COMMENT "testName == ${_testName}"
        # Cleanup previous data
        ${LCOV_BIN} --directory . --zerocounters

        # Run tests
        COMMAND ${_testName}

        # Capturing lcov counters and generating report
        COMMAND ${LCOV_BIN} --directory . --capture --output-file ${REPORT_PATH}.info
        COMMAND ${LCOV_BIN} --remove ${REPORT_PATH}.info '*test/*' '/usr/*' '*_mock*' --output-file ${REPORT_PATH}.info.cleaned
        COMMAND ${GENHTML_BIN} -o ${REPORT_PATH} ${REPORT_PATH}.info.cleaned
        COMMAND ${CMAKE_COMMAND} -E remove ${REPORT_PATH}.info ${REPORT_PATH}.info.cleaned

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Resetting code coverage counters to zero.\nProcessing code coverage counters and generating report."
      )
    endfunction()

    ## function to simplify creation of new test targets
    function(ADD_TEST_TARGET _testId _testSrc _testLibs)
      set(test_tgt_name test-${_testId})
      add_executable(${test_tgt_name} ${${_testSrc}})
      target_link_libraries(${test_tgt_name} ${${_testLibs}})
      add_test(MY-${test_tgt_name} ${test_tgt_name})
      ADD_COVERAGE_TARGET(${_testId} ${test_tgt_name})
    endfunction()

  else()  # (GCOV_BIN AND LCOV_BIN AND GENHTML_BIN)
    message(FATAL_ERROR "Failed to find GCOV LCOV or GENHTML")
    return()
  endif()
endif() # ENABLE_TEST

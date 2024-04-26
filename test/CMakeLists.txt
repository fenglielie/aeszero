## test.txt

add_test(NAME Test1 COMMAND ${PYTHON_EXECUTABLE} python test1.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(Test1 PROPERTIES
        ENVIRONMENT "PATH=${PATH}:${PROJECT_SOURCE_DIR}/bin"
        )
## image.png

add_test(NAME Test2 COMMAND ${PYTHON_EXECUTABLE} python test2.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(Test2 PROPERTIES
         ENVIRONMENT "PATH=${PATH}:${PROJECT_SOURCE_DIR}/bin"
         )
## demo.mp4

add_test(NAME Test3 COMMAND ${PYTHON_EXECUTABLE} python test3.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(Test3 PROPERTIES
         ENVIRONMENT "PATH=${PATH}:${PROJECT_SOURCE_DIR}/bin"
         )

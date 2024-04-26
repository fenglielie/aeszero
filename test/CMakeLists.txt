## test.txt

add_test(NAME Test1 COMMAND ${PYTHON_EXECUTABLE} python test1.py ${PROJECT_SOURCE_DIR}/bin/aeszero
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

## image.png

add_test(NAME Test2 COMMAND ${PYTHON_EXECUTABLE} python test2.py ${PROJECT_SOURCE_DIR}/bin/aeszero
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

## demo.mp4

add_test(NAME Test3 COMMAND ${PYTHON_EXECUTABLE} python test3.py ${PROJECT_SOURCE_DIR}/bin/aeszero
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

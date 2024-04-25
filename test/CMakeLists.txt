## test.txt

add_test(NAME test11
        COMMAND aeszero -e ../test.txt -k abcdefghijklmnop -o test.aes
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp
        )

add_test(NAME test12
        COMMAND aeszero -d test.aes -k abcdefghijklmnop -o test_new.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp
        )

add_test(NAME test13
        COMMAND aeszero -d test.aes -f test.aes.key -o test_new2.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp
        )

add_test(NAME test14
        COMMAND aeszero -d test.aes -k abcdefghijklm -o test_fail.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp
        )

add_test(NAME check11
        COMMAND python check.py test.txt tmp/test_new.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_test(NAME check12
        COMMAND python check.py test.txt tmp/test_new2.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_test(NAME check13
        COMMAND python check.py test.txt tmp/test_fail.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
set_property(TEST check13 PROPERTY WILL_FAIL true)


## image.png

add_test(NAME test21
        COMMAND aeszero -e ../image.png -o image.aes
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp
        )

add_test(NAME test22
        COMMAND aeszero -d image.aes -o image_new.png -f image.aes.key
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp
        )

add_test(NAME test23
        COMMAND aeszero -d image.aes -o image_fail.png -k bsgbgvcxz
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp
        )


add_test(NAME check21
        COMMAND python check.py image.png tmp/image_new.png
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_test(NAME check22
        COMMAND python  check.py image.png tmp/image_fail.png
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
set_property(TEST check22 PROPERTY WILL_FAIL true)


## demo.mp4

# add_test(NAME test31
#         COMMAND aeszero demo.mp4
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         )

# add_test(NAME test32
#         COMMAND aeszero demo.mp4.aes0
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         )

# add_test(NAME check31
#         COMMAND python check.py demo.mp4 demo.mp4.aes0.dec
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         )

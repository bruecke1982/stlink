IF(DEFINED QT5_PATH)
    PROJECT(StLinkGui)
    SET(EXE_NAME stlink-qt)

    SET(INSTALL_CONFIGURATIONS Release Debug RelWithDebInfo)

    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
    IF(NOT DEFINED WIN32)
       SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
       SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Werror=return-type")
    ENDIF()

    INCLUDE_DIRECTORIES(
        )

    set ( SOURCES
       main.cpp
       mainwindow.cpp
       mainwindow.h
       )


    SET(CMAKE_AUTORCC ON)
    SET(CMAKE_AUTOMOC ON)
    SET(CMAKE_AUTOUIC ON)

    SET(QT_MODULES
             Core
             Gui
             Widgets
             DBus
             Concurrent )

    IF (DEFINED QT5_PATH)
    FIND_PACKAGE(Qt5 COMPONENTS ${QT_MODULES} PATHS ${QT5_PATH})
    ENDIF (DEFINED QT5_PATH)



    ADD_EXECUTABLE(${EXE_NAME} ${SOURCES})

    ADD_CUSTOM_COMMAND(
        TARGET ${EXE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt5::Widgets>
            $<TARGET_FILE:Qt5::Core>
            $<TARGET_FILE:Qt5::Gui>
            $<TARGET_FILE_DIR:${EXE_NAME}>
    )

    TARGET_LINK_LIBRARIES(${EXE_NAME}
                            Qt5::Widgets
                            Qt5::Gui
                            Qt5::Core
                          )
	if (WIN32 OR APPLE)
	#${LIBUSB_WIN_OUTPUT_FOLDER}/MS${ARCH}/dll
		ADD_CUSTOM_COMMAND(
			TARGET ${EXE_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${LIBUSB_WIN_OUTPUT_FOLDER}/MS${ARCH}/dll/libusb-1.0.dll"
				$<TARGET_FILE_DIR:${EXE_NAME}>
		)
		TARGET_LINK_LIBRARIES(${EXE_NAME} ${STLINK_LIB_STATIC} ${SSP_LIB})
	else ()
		TARGET_LINK_LIBRARIES(${EXE_NAME} ${STLINK_LIB_SHARED} ${SSP_LIB})
	endif ()
    INSTALL(TARGETS ${EXE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
ENDIF (DEFINED QT5_PATH)


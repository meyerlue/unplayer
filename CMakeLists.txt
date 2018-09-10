cmake_minimum_required(VERSION 3.1.0)

project(harbour-unplayer VERSION 1.3.0)

include(GNUInstallDirs)

option(HARBOUR "Build for Harbour" ON)
option(QTMPRIS_STATIC "Link with qtmpris statically" OFF)
option(TAGLIB_STATIC "Link with taglib statically" OFF)

find_package(Qt5Core CONFIG REQUIRED)
if (Qt5Core_VERSION VERSION_LESS 5.6)
    message(FATAL_ERROR "Qt >= 5.6 is required")
endif()

add_subdirectory("translations")
add_subdirectory("src")

install(DIRECTORY "icons/hicolor" DESTINATION "${CMAKE_INSTALL_DATADIR}/icons")

set(qml_dir "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/qml")
install(FILES "qml/main.qml" DESTINATION "${qml_dir}")
install(DIRECTORY "qml/components" DESTINATION "${qml_dir}")
if (HARBOUR)
    set(media_keys "qml/MediaKeysEmpty.qml")
else()
    set(media_keys "qml/MediaKeysPrivate.qml")
endif()
install(FILES "${media_keys}"
        DESTINATION "${qml_dir}/components"
        RENAME "MediaKeys.qml")

install(FILES "${PROJECT_NAME}.desktop" DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
cmake_minimum_required(VERSION 3.5)

project(MychatClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql)

set(TS_FILES MychatClient_yue_CN.ts)

# 设置qrc路径
qt6_add_resources(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib/myresource.qrc)

add_definitions(-DWIN32_LEAN_AND_MEAN)


# 设置boost库路径
set(BOOST_ROOT "D:/boost_1_83_0")
set(BOOST_LIBRARYDIR "D:/boost_1_83_0/stage/lib")
#  设置OPenSSL路径
set(OPENSSL_ROOT_DIR "D:/vcpkg/vcpkg/installed/x64-windows")

# 添加必要组件
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)




if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MychatClient
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        networkmanager.h networkmanager.cpp
        showtext.h showtext.cpp
        loginwidget.h loginwidget.cpp loginwidget.ui
        ${RESOURCES}
        businesswidget.h businesswidget.cpp businesswidget.ui
        chatwindow/chatwindow.h chatwindow/chatwindow.cpp chatwindow/chatwindow.ui
        chatwindow/bubbleinfo.h chatwindow/bubbleinfo.cpp
        chatwindow/bubbleitemwidget.h chatwindow/bubbleitemwidget.cpp
        chatwindow/qnchatmessage.h chatwindow/qnchatmessage.cpp
        clientwindow.h clientwindow.cpp
        chatsession/cell.h chatsession/cell.cpp
        chatsession/listwidget.h chatsession/listwidget.cpp
        chatsession/floatingscrollbar.h chatsession/floatingscrollbar.cpp
        chatsession/cellviewdad.h chatsession/cellviewdad.cpp
        chatsession/cellviewson.h chatsession/cellviewson.cpp
        base/roundlabel.h base/roundlabel.cpp
        clientapi/myapplication.h clientapi/myapplication.cpp
        clientapi/status.h clientapi/status.cpp
        sqlite/sqldatabase.h sqlite/sqldatabase.cpp
        myconfig.h myconfig.cpp
        base/mybutton.h base/mybutton.cpp
        base/searchbar.h base/searchbar.cpp
        leftmenu/leftbar.h leftmenu/leftbar.cpp
        base/emojipicker.h base/emojipicker.cpp
        leftmenu/contactslistwidget.h leftmenu/contactslistwidget.cpp
        backgroundwidget.h backgroundwidget.cpp
        leftmenu/addsubgroup.h leftmenu/addsubgroup.cpp
        leftmenu/findfriendwnd.h leftmenu/findfriendwnd.cpp
        leftmenu/addfriendwnd.h leftmenu/addfriendwnd.cpp
        base/clicklabel.h base/clicklabel.cpp
        registerwnd.h registerwnd.cpp
        base/mywindow.h base/mywindow.cpp
        globalstorage.h globalstorage.cpp
        settingswnd.h settingswnd.cpp
        leftmenu/creategroupwnd.h leftmenu/creategroupwnd.cpp
        clientapi/screenwidget.h clientapi/screenwidget.cpp


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MychatClient APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(MychatClient SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MychatClient
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(MychatClient PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)


target_link_libraries(MychatClient PRIVATE dwmapi)


# 将 Boost 库链接到目标应用程序
if(Boost_FOUND)
    target_include_directories(MychatClient PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(MychatClient
        PRIVATE
        ws2_32

        Threads::Threads
        OpenSSL::Crypto
        OpenSSL::SSL
    )
endif()





# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MychatClient)
endif()
set_target_properties(MychatClient PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MychatClient
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MychatClient)
endif()

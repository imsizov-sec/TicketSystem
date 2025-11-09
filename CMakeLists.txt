cmake_minimum_required(VERSION 3.16)
project(TicketSystem VERSION 0.1 LANGUAGES CXX)

# --- VMime include & lib ---
include_directories("C:/msys64/mingw64/include")
link_directories("C:/msys64/mingw64/lib")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Sql PrintSupport)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

set(PROJECT_SOURCES
    src/main.cpp

    src/header/mainwindow.h src/source/mainwindow.cpp
    src/header/loginwindow.h src/source/loginwindow.cpp
    src/header/myticketstab.h src/source/myticketstab.cpp
    src/header/myprojectstab.h src/source/myprojectstab.cpp
    src/header/profiletab.h src/source/profiletab.cpp
    src/header/createticketdialog.h src/source/createticketdialog.cpp
    src/header/createprojectdialog.h src/source/createprojectdialog.cpp
    src/header/addemployeedialog.h src/source/addemployeedialog.cpp
    src/header/editemployeedialog.h src/source/editemployeedialog.cpp
    src/header/userprofileeditdialog.h src/source/userprofileeditdialog.cpp
    src/header/ticketcard.h src/source/ticketcard.cpp
    src/header/prioritydelegate.h src/source/prioritydelegate.cpp

    ui/mainwindow.ui
    ui/loginwindow.ui
    ui/myticketstab.ui
    ui/myprojectstab.ui
    ui/profiletab.ui
    ui/createticketdialog.ui
    ui/createprojectdialog.ui
    ui/editemployeedialog.ui
    ui/userprofileeditdialog.ui
    ui/ticketcard.ui
    ui/tickettable.ui

    model/header/user.h model/source/user.cpp
    model/header/ticket.h model/source/ticket.cpp

    utils/header/utils.h utils/source/utils.cpp
    utils/header/dbmanager.h utils/source/dbmanager.cpp
    utils/header/storagemanager.h utils/source/storagemanager.cpp
    utils/header/mailservice.h utils/source/mailservice.cpp
    utils/header/mycertverifier.h utils/source/mycertverifier.cpp

    resources/resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TicketSystem
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(TicketSystem SHARED ${PROJECT_SOURCES})
    else()
        add_executable(TicketSystem ${PROJECT_SOURCES})
    endif()
endif()

target_include_directories(TicketSystem PRIVATE
    src/
    src/header/
    src/source/
    model/header/
    model/source/
    utils/header/
    utils/source/
    resources/
    ui/
    ${CMAKE_CURRENT_SOURCE_DIR}  # корневая директория проекта
)

target_link_libraries(TicketSystem
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::PrintSupport
    vmime
    ssl
    crypto
    ws2_32
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TicketSystem)
endif()

set_target_properties(TicketSystem PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

include(GNUInstallDirs)
install(TARGETS TicketSystem
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TicketSystem)
endif()

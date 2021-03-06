include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(searchplugintest_SRCS
    searchplugintest.cpp
    ../searchplugin.cpp
    ../../agent/emailindexer.cpp
    ../../agent/calendarindexer.cpp
    ../../agent/contactindexer.cpp
    ../../agent/abstractindexer.cpp
    ../../agent/akonotesindexer.cpp
    ../../search/pimsearchstore.cpp
    ../../search/email/emailsearchstore.cpp
    ../../search/email/agepostingsource.cpp
    ../../search/contact/contactsearchstore.cpp
    ../../search/calendar/calendarsearchstore.cpp
    ../../search/note/notesearchstore.cpp
)

add_definitions(-DBALOO_NO_PLUGINS=TRUE)

kde4_add_unit_test(searchplugintest NOGUI ${searchplugintest_SRCS})
target_link_libraries(searchplugintest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_KABC_LIBS}
    ${XAPIAN_LIBRARIES}
    ${KDEPIMLIBS_KPIMUTILS_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    baloocore
    balooxapian
)

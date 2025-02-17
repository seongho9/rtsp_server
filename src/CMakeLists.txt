
add_executable(main
    main.cpp
    server/RTSPListener.cpp
    session/RTSPFileSession.cpp session/RTSPFileSessionGst.cpp 
    session/RTSPFileSessionGstOption.cpp session/RTSPFileSessionGstDescribe.cpp session/RTSPFileSessionGstSetup.cpp session/RTSPFileSessionGstPlay.cpp session/RTSPFileSessionGstPause.cpp session/RTSPFileSessionGstTeardown.cpp
)

message(STATUS "boost path ${Boost_INCLUDE_DIRS}")
target_link_libraries(
    main
    ${Boost_LIBRARIES}
    -lpthread
    gstreamer-1.0 gobject-2.0 glib-2.0 gstapp-1.0 gstsdp-1.0 gstrtp-1.0
)
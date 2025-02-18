
add_executable(main
    main.cpp
    server/RTSPListener.cpp
    session/RTSPSession.cpp session/RTSPSessionImpl.cpp 
    handler/RTSPSessionHandlerGstFile.cpp 
    handler/RTSPSessionHandlerGstFileDescribe.cpp   handler/RTSPSessionHandlerGstFileOption.cpp handler/RTSPSessionHandlerGstFilePause.cpp
    handler/RTSPSessionHandlerGstFilePlay.cpp       handler/RTSPSessionHandlerGstFileSetup.cpp  handler/RTSPSessionHandlerGstFileTeardown.cpp
    )

message(STATUS "boost path ${Boost_INCLUDE_DIRS}")
target_link_libraries(
    main
    ${Boost_LIBRARIES}
    -lpthread
    gstreamer-1.0 gobject-2.0 glib-2.0 gstapp-1.0 gstsdp-1.0 gstrtp-1.0
)
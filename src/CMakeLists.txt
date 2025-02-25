
add_executable(main
    main.cpp
    server/RTSPListener.cpp
    session/RTSPSession.cpp session/RTSPSessionImpl.cpp 

    handler/gst_file/RTSPSessionHandlerGstFile.cpp
    handler/gst_file/RTSPSessionHandlerGstFileDescribe.cpp   handler/gst_file/RTSPSessionHandlerGstFileOption.cpp handler/gst_file/RTSPSessionHandlerGstFilePause.cpp
    handler/gst_file/RTSPSessionHandlerGstFilePlay.cpp       handler/gst_file/RTSPSessionHandlerGstFileSetup.cpp  handler/gst_file/RTSPSessionHandlerGstFileTeardown.cpp

    handler/gst_live/RTSPSessionHandlerGstLive.cpp           handler/gst_live/MulticastStream.cpp                 handler/gst_live/MulticastStreamV4L2.cpp
    handler/gst_live/MulticastStreamV4L2MakeSdp.cpp          handler/gst_live/MulticastStreamV4L2PlayStream.cpp   handler/gst_live/MulticastStreamV4L2SetStream.cpp
    )

add_library(hydra SHARED
    server/RTSPListener.cpp
    session/RTSPSession.cpp session/RTSPSessionImpl.cpp 

    handler/gst_file/RTSPSessionHandlerGstFile.cpp
    handler/gst_file/RTSPSessionHandlerGstFileDescribe.cpp   handler/gst_file/RTSPSessionHandlerGstFileOption.cpp handler/gst_file/RTSPSessionHandlerGstFilePause.cpp
    handler/gst_file/RTSPSessionHandlerGstFilePlay.cpp       handler/gst_file/RTSPSessionHandlerGstFileSetup.cpp  handler/gst_file/RTSPSessionHandlerGstFileTeardown.cpp

    handler/gst_live/RTSPSessionHandlerGstLive.cpp           handler/gst_live/MulticastStream.cpp                 handler/gst_live/MulticastStreamV4L2.cpp
    handler/gst_live/MulticastStreamV4L2MakeSdp.cpp          handler/gst_live/MulticastStreamV4L2PlayStream.cpp   handler/gst_live/MulticastStreamV4L2SetStream.cpp
    )

message(STATUS "boost path ${Boost_INCLUDE_DIRS}")
target_link_libraries(
    main
    ${Boost_LIBRARIES}
    -lpthread
    gstreamer-1.0 gobject-2.0 glib-2.0 gstapp-1.0 gstsdp-1.0 gstrtp-1.0
)
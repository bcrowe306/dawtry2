cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(dawtry2 VERSION 0.1.0 LANGUAGES C CXX)
add_executable(dawtry2 ${CMAKE_SOURCE_DIR}/src/main.cpp)

# src includes
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/LabSound)

target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/lib/LabSound/third_party/libnyquist/lib/liblibnyquist_d.a)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib/LabSound/include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/bin/LabSound_d.framework/LabSound_d)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/bin/libLabSoundMiniAudio_d.a)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/bin/libLabSoundRtAudio_d.a)



# RTMidi
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/rtmidi)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib/rtmidi)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/lib/rtmidi/librtmidi.dylib)



if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE
  "-framework CoreMidi"
  "-framework Cocoa"
  "-framework Accelerate"
  "-framework AudioToolbox"
  "-framework AudioUnit"
  "-framework CoreServices"
  "-framework CoreAudio"
  "-framework Carbon"
  "-framework CoreFoundation"
    "-framework IOKit"
  )
endif()
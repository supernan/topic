INCLUDE_DIRECTORIES("../include")
LINK_DIRECTORIES(
	"/home/zhounan/project/cpp/topic/lib"
	)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g  -fopenmp -Wall -O2")  
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -fopenmp -Wall -O2")
SET(CMAKE_EXE_LINKER_FLAGS "-lpthread -lrt -ltinyxml -lglog -lNLPIR")
SET(SEC_LIST
	test.cpp
	util.cpp
	plsa.cpp
	subtopic.cpp
	)
ADD_EXECUTABLE(test ${SEC_LIST})
TARGET_LINK_LIBRARIES(test libtinyxml.so libNLPIR.so libglog.a pthread)
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#TARGET_LINK_LIBRARIES(main ${PROJECT_SOURCE_DIR}/libs/libboost_regex.a)I

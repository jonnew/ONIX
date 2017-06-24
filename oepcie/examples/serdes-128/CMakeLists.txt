# 128 channel headstage test
add_executable(serdes-128 main.cpp)
#target_link_libraries(serdes-128 oepcie)

# Installation
install (TARGETS serdes-128 DESTINATION ./ COMPONENT examples)

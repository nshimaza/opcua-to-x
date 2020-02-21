cmake_minimum_required(VERSION 3.0)
project(opcua-to-x)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    # set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

find_package(open62541 REQUIRED COMPONENTS FullNamespace)

set(INIH_DIR ${CMAKE_CURRENT_BINARY_DIR}/../inih)
set(COMPANION_NODESET_DIR ${CMAKE_CURRENT_BINARY_DIR}/../UA-Nodeset)

# Generate types and namespace for DI
ua_generate_nodeset_and_datatypes(
    NAME "di"
    FILE_CSV "${open62541_NODESET_DIR}/DI/OpcUaDiModel.csv"
    FILE_BSD "${open62541_NODESET_DIR}/DI/Opc.Ua.Di.Types.bsd"
    NAMESPACE_IDX 2
    FILE_NS "${open62541_NODESET_DIR}/DI/Opc.Ua.Di.NodeSet2.xml"
    INTERNAL
)

# generate PLCopen namespace which is using DI
ua_generate_nodeset_and_datatypes(
    NAME "plc"
    FILE_NS "${open62541_NODESET_DIR}/PLCopen/Opc.Ua.Plc.NodeSet2.xml"
    # PLCopen depends on the di nodeset, which must be generated before
    DEPENDS "di"
    INTERNAL
)

# generate Robotics namespace which is using DI
ua_generate_nodeset_and_datatypes(
    NAME "robot"
    FILE_NS "${COMPANION_NODESET_DIR}/Robotics/Opc.Ua.Robotics.NodeSet2.xml"
    # Robotics depends on the di nodeset, which must be generated before
    DEPENDS "di"
    INTERNAL
)

add_executable(opcua-to-x src/main.c src/async_loop.c src/hexdump.c src/mvar.c src/robot.c src/util.c ${INIH_DIR}/ini.c
    ${UA_NODESET_DI_SOURCES} ${UA_NODESET_PLC_SOURCES} ${UA_NODESET_ROBOT_SOURCES})
add_dependencies(opcua-to-x open62541-generator-ns-plc open62541-generator-ns-robot)
target_include_directories(opcua-to-x PRIVATE ${INIH_DIR} ${CMAKE_CURRENT_BINARY_DIR}/src_generated)
target_link_libraries(opcua-to-x PRIVATE open62541::open62541)
target_link_libraries(opcua-to-x PRIVATE uv)

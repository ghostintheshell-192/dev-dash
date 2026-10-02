# Unit tests (Catch2 v3). Links the services static library — no graphics
# stack — so service logic is exercised in isolation on real tmpdirs
# (concrete-services-on-tmpdir testing approach, ADR-010).

add_executable(dev-dash-tests
    test_apply_engine.cpp
    test_class_diagram_generator.cpp
    test_config_resolver.cpp
    test_cpp_class_extractor.cpp
    test_document_loader.cpp
    test_snapshot_service.cpp
)

target_link_libraries(dev-dash-tests
    PRIVATE
        dev-dash-warnings
        dev-dash-services
        Catch2::Catch2WithMain
)

# Catch.cmake (catch_discover_tests) ships under Catch2's extras/ directory.
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(dev-dash-tests)

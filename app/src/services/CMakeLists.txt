# Services layer (ADR-010) built as a static library so both the application
# and the test target (dev-dash-tests) can link the domain logic without
# pulling in the ImGui/SDL/Vulkan graphics stack. The layer is graphics-free
# by construction: nothing under services/ includes imgui/SDL/vulkan.

add_library(dev-dash-services STATIC
    document_loader.h
    document_loader.cpp
    config_file_scanner.h
    config_file_scanner.cpp
    settings_parser.h
    settings_parser.cpp
    adapter_utils.h
    claude_md_adapter.h
    claude_md_adapter.cpp
    rules_adapter.h
    rules_adapter.cpp
    memory_adapter.h
    memory_adapter.cpp
    skills_adapter.h
    skills_adapter.cpp
    agents_adapter.h
    agents_adapter.cpp
    mcp_adapter.h
    mcp_adapter.cpp
    hooks_adapter.h
    hooks_adapter.cpp
    config_resolver.h
    config_resolver.cpp
    diff_engine.h
    diff_engine.cpp
    apply_engine.h
    apply_engine.cpp
    promote_engine.h
    promote_engine.cpp
    snapshot_service.h
    snapshot_service.cpp
    scaffold_repository.h
    scaffold_repository.cpp
)

# Lets consumers include via "services/..." and "core/..." (the layer's own
# files use file-relative "../core/..." includes, which keep working too).
target_include_directories(dev-dash-services PUBLIC ${CMAKE_SOURCE_DIR}/src)

# nlohmann_json is used by SettingsParser; PUBLIC so the JSON headers reach
# any consumer that transitively pulls a service header including it.
target_link_libraries(dev-dash-services PUBLIC nlohmann_json::nlohmann_json)

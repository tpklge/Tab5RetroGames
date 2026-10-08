file(GLOB_RECURSE ARCADE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/core/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/display/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/audio/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/game_manager/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/storage/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ui/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/games/*/*.cpp")
idf_component_register(
    SRCS "main.cpp" ${ARCADE_SOURCES}
    INCLUDE_DIRS "." "core" "display" "input" "audio" "game_manager" "storage" "ui"
    PRIV_INCLUDE_DIRS "../components/m5_tab5_component/src/drivers"
    REQUIRES m5_tab5_component m5_tab5_keyboard_component nvs_flash
             esp_driver_ledc esp_driver_i2s esp_timer espressif__i2c_bus fatfs sdmmc esp_driver_sdmmc
)

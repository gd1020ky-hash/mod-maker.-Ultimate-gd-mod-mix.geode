cmake_minimum_required(VERSION 3.25)

# =========================================================================
# 🌐 ULTIMATE GD MOD MIX - CROSS-PLATFORM BUILD PIPELINE
# =========================================================================
project(ultimate_gd_mod_mix VERSION 1.0.0 LANGUAGES CXX)

# Include the main Geode SDK build frameworks
include(FetchContent)
FetchContent_Declare(
    GeodeSDK
    GIT_REPOSITORY https://github.com
    GIT_TAG v5.10.1
)
FetchContent_MakeAvailable(GeodeSDK)

# Declare your mod structure binary name
add_geode_mod(ultimate_gd_mod_mix)

# Track the project source compilation files
target_sources(ultimate_gd_mod_mix PRIVATE
    src/main.cpp
)

# Standard Geode dependency configurations
setup_geode_mod(ultimate_gd_mod_mix)

cmake_minimum_required(VERSION 3.25)

# =========================================================================
# 🌐 ULTIMATE GD MOD MIX - REPAIRED CROSS-PLATFORM BUILD PIPELINE
# =========================================================================
project(ultimate_gd_mod_mix VERSION 1.0.0 LANGUAGES CXX)

# Find and load the pre-installed Geode SDK configurations provided by the build runner
find_package(Geode REQUIRED)

# Declare your mod structure binary name
add_geode_mod(ultimate_gd_mod_mix)

# Track the project source compilation files
target_sources(ultimate_gd_mod_mix PRIVATE
    src/main.cpp
)

# Standard Geode dependency configurations
setup_geode_mod(ultimate_gd_mod_mix)

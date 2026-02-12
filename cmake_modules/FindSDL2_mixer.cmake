




# set(SDL2_MIXER_SEARCH_PATHS ${SDL2_PATH})

# find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
#         PATH_SUFFIXES include/SDL2 include
#         PATHS ${SDL2_MIXER_SEARCH_PATHS}
# )

# find_library(SDL2_MIXER_LIBRARY
#         NAMES SDL2_mixer
#         PATH_SUFFIXES lib
#         PATHS ${SDL2_MIXER_SEARCH_PATHS}
# )

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(SDL2_mixer REQUIRED_VARS SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PATH_SUFFIXES lib/x64 lib64 lib)
else()
    set(PATH_SUFFIXES lib/x86 lib)
endif()

find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
    HINTS ${SDL2_PATH}
    PATH_SUFFIXES include/SDL2 include
)

find_library(SDL2_MIXER_LIBRARY
    NAMES SDL2_mixer
    HINTS ${SDL2_PATH}
    PATH_SUFFIXES ${PATH_SUFFIXES}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer REQUIRED_VARS SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)
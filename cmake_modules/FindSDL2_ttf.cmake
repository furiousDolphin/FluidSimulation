




# set(SDL2_TTF_SEARCH_PATHS ${SDL2_PATH})

# find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
#         PATH_SUFFIXES include/SDL2 include
#         PATHS ${SDL2_TTF_SEARCH_PATHS}
# )

# find_library(SDL2_TTF_LIBRARY
#         NAMES SDL2_ttf
#         PATH_SUFFIXES lib
#         PATHS ${SDL2_TTF_SEARCH_PATHS}
# )

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(SDL2_ttf REQUIRED_VARS SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PATH_SUFFIXES lib/x64 lib64 lib)
else()
    set(PATH_SUFFIXES lib/x86 lib)
endif()

find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
    HINTS ${SDL2_PATH}
    PATH_SUFFIXES include/SDL2 include
)

find_library(SDL2_TTF_LIBRARY
    NAMES SDL2_ttf
    HINTS ${SDL2_PATH}
    PATH_SUFFIXES ${PATH_SUFFIXES}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_ttf REQUIRED_VARS SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)
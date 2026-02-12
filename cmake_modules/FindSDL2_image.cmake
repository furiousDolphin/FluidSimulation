




# set(SDL2_IMAGE_SEARCH_PATHS ${SDL2_PATH})

# find_path(SDL2_IMAGE_INCLUDE_DIR SDL_image.h
#         PATH_SUFFIXES include/SDL2 include
#         PATHS ${SDL2_IMAGE_SEARCH_PATHS}
# )

# find_library(SDL2_IMAGE_LIBRARY
#         NAMES SDL2_image
#         PATH_SUFFIXES lib
#         PATHS ${SDL2_IMAGE_SEARCH_PATHS}
# )

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(SDL2_image REQUIRED_VARS SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)

# Wykrywanie architektury 64-bitowej
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PATH_SUFFIXES lib/x64 lib64 lib)
else()
    set(PATH_SUFFIXES lib/x86 lib)
endif()

# Szukanie nagłówka
find_path(SDL2_IMAGE_INCLUDE_DIR SDL_image.h
    HINTS ${SDL2_PATH} ${SDL2_IMAGE_PATH}
    PATH_SUFFIXES include/SDL2 include
)

# Szukanie biblioteki .lib (zwróć uwagę na NAMES - to klucz do MSVC)
find_library(SDL2_IMAGE_LIBRARY
    NAMES SDL2_image SDL2_image-static
    HINTS ${SDL2_PATH} ${SDL2_IMAGE_PATH}
    PATH_SUFFIXES ${PATH_SUFFIXES}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_image REQUIRED_VARS SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)
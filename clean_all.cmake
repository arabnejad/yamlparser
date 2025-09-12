# clean_all.cmake — remove all CMake build directories under the source tree
#
# Safety measures:
# - Never delete the source root (even if it contains CMakeCache.txt).
# - Only remove directories that contain a 'CMakeFiles' subdirectory.
# - Only remove directories whose names match common build-dir patterns.

if(NOT DEFINED ROOT)
  message(FATAL_ERROR "ROOT variable must be provided (the project source directory).")
endif()

# Normalize ROOT
get_filename_component(ROOT "${ROOT}" ABSOLUTE)

# Warn if someone created an in-source build (we won't remove the root)
if(EXISTS "${ROOT}/CMakeCache.txt")
  message(WARNING
    "CMakeCache.txt found in the source root:\n"
    "  ${ROOT}\n"
    "This indicates an in-source build. 'clean_all' will NOT delete the source tree.\n"
    "Please remove 'CMakeCache.txt' and the 'CMakeFiles/' dir manually, or reconfigure out-of-source."
  )
endif()


# ------------------------------------------------------------------
# Remove CMake build directories
# ------------------------------------------------------------------

# Find all CMakeCache.txt files under the source tree (recursive)
file(GLOB_RECURSE _caches RELATIVE "${ROOT}" "${ROOT}/*/CMakeCache.txt")

set(_build_dirs "")
foreach(_cache ${_caches})
  get_filename_component(_dir "${ROOT}/${_cache}" DIRECTORY)

  # Skip the source root entirely
  if(_dir STREQUAL "${ROOT}")
    continue()
  endif()

  # Must look like a CMake build directory
  if(NOT EXISTS "${_dir}/CMakeFiles")
    continue()
  endif()

  # Only delete common build dir names. Uncomment to be stricter.
  get_filename_component(_name "${_dir}" NAME)
  if(NOT (_name STREQUAL "build" OR _name MATCHES "^cmake-build" OR _name MATCHES "^[Bb]uild-"))
    continue()
  endif()

  list(APPEND _build_dirs "${_dir}")
endforeach()

list(REMOVE_DUPLICATES _build_dirs)

if(_build_dirs)
  message(STATUS "Removing the following CMake build directories:")
  foreach(_d ${_build_dirs})
    message(STATUS "  ${_d}")
    file(REMOVE_RECURSE "${_d}")
  endforeach()
else()
  message(STATUS "No removable build directories found under: ${ROOT}")
endif()

# ------------------------------------------------------------------
# Also remove in-source sample output dirs (executables)
# ------------------------------------------------------------------
set(_in_source_bins
  "${ROOT}/limitation/bin"
  "${ROOT}/sample_usage/bin"
)

foreach(_dir ${_in_source_bins})
  # Safety: only delete paths under the project root, and avoid ROOT itself
  if(_dir STREQUAL "${ROOT}")
    message(WARNING "Refusing to remove source root: ${_dir}")
  elseif(EXISTS "${_dir}")
    message(STATUS "Removing in-source bin dir: ${_dir}")
    file(REMOVE_RECURSE "${_dir}")
  endif()
endforeach()


message(STATUS "clean_all: Done.")
message(STATUS "")
message(STATUS "✅ All build directories removed.")
message(STATUS "👉 Next: run  cmake -S . -B build  to (re)configure before building.")

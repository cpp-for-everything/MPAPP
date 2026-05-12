# MPAPP tooling.
#
# Each tool lives in its own subdirectory and is built only when
# MPAPP_BUILD_TOOLS is enabled at the root level. Tools target the host
# platform; they are not cross-compiled along with the runtime.

add_subdirectory(mpapp)

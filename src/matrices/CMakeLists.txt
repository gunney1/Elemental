# Add the subdirectories
add_subdirectory(deterministic)
add_subdirectory(random)

# Propagate the files up the tree
set(SOURCES "${SOURCES}" "${THIS_DIR_SOURCES}" PARENT_SCOPE)

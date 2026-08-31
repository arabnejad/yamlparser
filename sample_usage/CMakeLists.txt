set(YAMLPARSER_EXAMPLES
  anchors_merge
  app_config
  arrays_sequences
  basic_config
  complex_data
  data_types
  multiline_strings
  nested_arrays
  nested_maps
  nested_structures
)

set(YAMLPARSER_EXAMPLE_TARGETS "")
foreach(example_name IN LISTS YAMLPARSER_EXAMPLES)
  set(example_target yamlparser_example_${example_name})
  add_executable(${example_target} src/${example_name}.cpp)
  target_link_libraries(${example_target} PRIVATE yamlparser::yamlparser)
  target_compile_features(${example_target} PRIVATE cxx_std_14)
  list(APPEND YAMLPARSER_EXAMPLE_TARGETS ${example_target})
endforeach()

add_custom_target(examples DEPENDS ${YAMLPARSER_EXAMPLE_TARGETS})

add_custom_target(run_examples
  COMMAND ${CMAKE_COMMAND} -E echo "Running YAML parser examples"
  DEPENDS examples
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

foreach(example_name IN LISTS YAMLPARSER_EXAMPLES)
  add_custom_command(TARGET run_examples POST_BUILD
    COMMAND $<TARGET_FILE:yamlparser_example_${example_name}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endforeach()

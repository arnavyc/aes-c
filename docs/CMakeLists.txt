find_package(Doxygen)
find_package(Sphinx)

if (DOXYGEN_FOUND AND SPHINX_FOUND)
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs/doxygen)
  set(DOXYGEN_GENERATE_XML YES)

  doxygen_add_docs(docs-doxygen ${PROJECT_SOURCE_DIR}/include/ay)

  set(DOXY_XML_OUT ${DOXYGEN_OUTPUT_DIRECTORY}/xml/index.xml)

  # add_custom_command(OUTPUT ${DOXY_XML_OUT} DEPENDS docs-d)
  set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
  set(SPHINX_BUILD ${CMAKE_BINARY_DIR}/docs/sphinx)

  if (POETRY_PROGRAM)
    set(SPHINX_EXECUTOR ${POETRY_PROGRAM} run)
  endif ()

  set(${PROJECT_NAME}_SPHINX_FORMATS
      "html;man"
      CACHE STRING "Formats to generate docs"
  )

  add_custom_target(docs)

  foreach (format IN LISTS ${PROJECT_NAME}_SPHINX_FORMATS)
    set(sphinx_versioned_formats "html")

    if (format IN_LIST sphinx_versioned_formats)
      add_custom_target(
        docs-${format}
        # Run sphinx-multiversion
        COMMAND
          ${SPHINX_EXECUTOR} sphinx-multiversion
          -Dbreathe_projects.aes-c=${DOXYGEN_OUTPUT_DIRECTORY}/xml
          ${SPHINX_SOURCE} ${SPHINX_BUILD}/${format}
        # Copy gh-page-redirect.html to index.html
        COMMAND
          ${CMAKE_COMMAND} -E copy_if_different
          ${SPHINX_SOURCE}/gh-page-redirect.html
          ${SPHINX_BUILD}/${format}/index.html
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating documentation with Sphinx - ${format} format"
        DEPENDS docs-doxygen
      )
    else ()
      add_custom_target(
        docs-${format}
        COMMAND
          ${SPHINX_EXECUTOR} ${SPHINX_EXECUTABLE} -b ${format}
          # Tell Breathe where to find the Doxygen output
          -Dbreathe_projects.aes-c=${DOXYGEN_OUTPUT_DIRECTORY}/xml
          ${SPHINX_SOURCE} ${SPHINX_BUILD}/${format}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating documentation with Sphinx - ${format} format"
        DEPENDS docs-doxygen
      )
    endif ()
    add_dependencies(docs docs-${format})
  endforeach ()
endif ()

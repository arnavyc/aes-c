function (target_add_compiler_options target)
  cmake_parse_arguments("" "" "PREFIX;TARGET_TYPE" "CONFIGS;LANGUAGES" ${ARGN})
  if (NOT DEFINED _CONFIGS)
    set(_CONFIGS Debug Release)
  endif ()

  if (NOT DEFINED _LANGUAGES)
    set(_LANGUAGES C CXX)
  endif ()

  foreach (lang IN LISTS _LANGUAGES)
    set(var_name ${_PREFIX}_${lang}_FLAGS)
    set(${var_name}
        ""
        CACHE STRING "${lang} flags to set in ${_PREFIX}"
    )

    if (NOT "${${var_name}}" STREQUAL "")
      target_compile_options(
        ${target} ${_TARGET_TYPE} "$<$<COMPILE_LANGUAGE:${i}>:${${var_name}}>"
      )
    endif ()

    foreach (config IN LISTS _CONFIGS)
      string(TOUPPER ${config} config_upper)
      set(var_name ${_PREFIX}_${lang}_FLAGS_${config_upper})
      set(${var_name}
          ""
          CACHE STRING
                "${lang} flags to set in ${_PREFIX} - ${config} configuration"
      )

      if (NOT "${${var_name}}" STREQUAL "")
        target_compile_options(
          ${target}
          ${_TARGET_TYPE}
          "$<$<AND:$<COMPILE_LANGUAGE:${lang}>,$<CONFIG:${config}>>:${${var_name}}>"
        )
      endif ()
    endforeach ()
  endforeach ()
endfunction ()

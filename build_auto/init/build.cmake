# building
if(WIN32)
	add_definitions(-DNOMINMAX)
endif()

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

add_compile_options(
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wpedantic>
	$<$<CXX_COMPILER_ID:MSVC>:/Wall>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
	$<$<CXX_COMPILER_ID:MSVC>:/WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error=unknown-pragmas>
	$<$<CXX_COMPILER_ID:MSVC>:/wd4068> # unknown pragma
	$<$<CXX_COMPILER_ID:MSVC>:/wd4464> # relative include path contains '..'
	$<$<CXX_COMPILER_ID:MSVC>:/wd4514> # unreferenced inline function has been removed
	$<$<CXX_COMPILER_ID:MSVC>:/wd4710> # function not inlined
	$<$<CXX_COMPILER_ID:MSVC>:/wd5030> # attribute is not recognized

	# pragma warning seems to be broken for below warnings
	$<$<CXX_COMPILER_ID:MSVC>:/wd4868> # compiler may not enforce left-to-right evaluation order in braced initializer list
	$<$<CXX_COMPILER_ID:MSVC>:/wd5045> # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified

	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-finput-charset=UTF-8>
	$<$<CXX_COMPILER_ID:MSVC>:/source-charset:UTF-8>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fexec-charset=UTF-8>
	$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:UTF-8>
)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	add_compile_options(
		-Weverything
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-c++98-c++11-c++14-compat
		-Wno-c++20-compat
		-Wno-return-std-move-in-c++11
		-Wno-missing-variable-declarations
		-Wno-ctad-maybe-unsupported
		-Wno-error=unknown-argument
		-Wno-error=unknown-warning-option
		-Wno-error=unknown-attributes
	)

	if(WIN32)
		add_compile_options(-Wno-error=nonportable-system-include-path)
	endif()
endif()

if(ARTCCEL_PROFILE_COMPILATION)
	message(STATUS "Compilation profiling is enabled")
	add_compile_options($<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-ftime-report>)

	if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		add_compile_options(-ftime-trace)
	endif()

	# MSVC: vcperf
endif()

add_link_options(
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/DEBUG>
	$<$<CXX_COMPILER_ID:MSVC>:/WX>
)

function(enable_ipo_if_supported)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)

	if(IPO_SUPPORTED)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
		message(STATUS "Interprocedural optimization is enabled")
	else()
		message(WARNING "Interprocedural optimization is not supported: ${IPO_OUTPUT}")
	endif()
endfunction()

enable_ipo_if_supported()

function(try_match_pic_and_reuse_precompile_headers_for_executable TARGET OTHER_TARGET)
	include(CheckPIESupported)
	check_pie_supported(LANGUAGES ${ENABLED_LANGUAGES})
	get_target_property(LINK_PIC "${OTHER_TARGET}" POSITION_INDEPENDENT_CODE)

	if(LINK_PIC)
		set(LINK_PIE_SUPPORTED true)

		foreach(LANGUAGE IN LISTS ENABLED_LANGUAGES)
			if(NOT "CMAKE_${LANGUAGE}_LINK_PIE_SUPPORTED")
				set(LINK_PIE_SUPPORTED false)
				break()
			endif()
		endforeach()

		if(LINK_PIE_SUPPORTED)
			# the below line adds '-fPIE' instead of '-fPIC' for executables
			# set_target_properties("${TARGET}" PROPERTIES POSITION_INDEPENDENT_CODE "${LINK_PIE_SUPPORTED}")
			if(NOT WIN32) # unsupported
				target_compile_options("${TARGET}" PUBLIC $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fPIC>)
			endif()

			target_precompile_headers("${TARGET}" REUSE_FROM "${OTHER_TARGET}")
			return()
		endif()
	else()
		set(LINK_NO_PIE_SUPPORTED true)

		foreach(LANGUAGE IN LISTS ENABLED_LANGUAGES)
			if(NOT "CMAKE_${LANGUAGE}_LINK_NO_PIE_SUPPORTED")
				set(LINK_NO_PIE_SUPPORTED false)
				break()
			endif()
		endforeach()

		if(LINK_NO_PIE_SUPPORTED)
			set_target_properties("${TARGET}" PROPERTIES POSITION_INDEPENDENT_CODE false)
			target_precompile_headers("${TARGET}" REUSE_FROM "${OTHER_TARGET}")
			return()
		endif()
	endif()

	message(WARNING "Failed to reuse precompile headers from '${OTHER_TARGET}' with '${TARGET}'")
endfunction()

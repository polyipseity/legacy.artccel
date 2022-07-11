# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# tools
# clang-tidy
# workaround: make clang-tidy include non-default system headers
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
list(INSERT CMAKE_C_STANDARD_INCLUDE_DIRECTORIES 0 ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
list(INSERT CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 0 ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_C_CLANG_TIDY clang-tidy
	-p "${CMAKE_BINARY_DIR}/compile_commands.json"
	"-line-filter=[\
{\"name\": \"config.h\", \"lines\": [[65535, 1]]},\
{\"name\": \"export.h\", \"lines\": [[65535, 1]]},\
{\"name\": \".h\"},\
{\"name\": \".c\"},\
]"
	"--config-file=${ROOT_SOURCE_DIR}/.clang-tidy")
set(CMAKE_CXX_CLANG_TIDY clang-tidy
	-p "${CMAKE_BINARY_DIR}/compile_commands.json"
	"-line-filter=[\
{\"name\": \"config.h\", \"lines\": [[65535, 1]]},\
{\"name\": \"export.h\", \"lines\": [[65535, 1]]},\
{\"name\": \".h\"},\
{\"name\": \".hpp\"},\
{\"name\": \".c\"},\
{\"name\": \".cpp\"}\
]"
	"--config-file=${ROOT_SOURCE_DIR}/.clang-tidy")

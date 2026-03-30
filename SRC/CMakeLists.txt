add_executable(Projet)

file(GLOB OTHER_SOURCES "*.cpp")
list(REMOVE_ITEM OTHER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp")


target_sources(Projet
	PRIVATE
		Main.cpp
		${OTHER_SOURCES}
		../DEP/GLAD/src/glad.c
	)

target_include_directories(Projet
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		../DEP/GLAD/include
	)

target_link_libraries(Projet
	PRIVATE
		glfw
		OpenGL::GL
		Threads::Threads
		OpenMP::OpenMP_CXX
	)



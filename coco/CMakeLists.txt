set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
	PUBLIC FILE_SET headers TYPE HEADERS FILES
		Dac.hpp
		DacDummy.hpp
	PRIVATE
		DacDummy.cpp
)

if(${PLATFORM} STREQUAL "native" OR ${PLATFORM} STREQUAL "emu")
	# native platform (Windows, MacOS, Linux)
	target_sources(${PROJECT_NAME}
		PUBLIC FILE_SET platform_headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/native FILES
			native/coco/platform/Dac_cout.hpp
		PRIVATE
			native/coco/platform/Dac_cout.cpp
	)
elseif(${PLATFORM} MATCHES "^nrf52" OR ${PLATFORM} MATCHES "^stm32c0")
	# does not have a DAC
elseif(${PLATFORM} MATCHES "^stm32")
	target_sources(${PROJECT_NAME}
		PUBLIC FILE_SET platform_headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/stm32 FILES
			stm32/coco/platform/DacDevice_DAC_DMA.hpp
			stm32/coco/platform/Dac_DAC.hpp
		PRIVATE
			stm32/coco/platform/DacDevice_DAC_DMA.cpp
			stm32/coco/platform/Dac_DAC.cpp
	)
endif()

target_link_libraries(${PROJECT_NAME}
	coco::coco
	coco-loop::coco-loop
	coco-device::coco-device
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		..
)

# install the library
install(TARGETS ${PROJECT_NAME}
	FILE_SET headers DESTINATION include/coco
	FILE_SET platform_headers DESTINATION include
)

# demo app
ADD_EXECUTABLE(nnstreamer_example_decoder_image_labelling nnstreamer_example_decoder_image_labelling.c)

TARGET_LINK_LIBRARIES(nnstreamer_example_decoder_image_labelling ${apppkgs_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(nnstreamer_example_decoder_image_labelling PUBLIC ${apppkgs_INCLUDE_DIRS})
TARGET_COMPILE_OPTIONS(nnstreamer_example_decoder_image_labelling PUBLIC ${apppkgs_CFLAGS_OTHER})

IF (INSTALL_EXAMPLE_APP)
	INSTALL(TARGETS nnstreamer_example_decoder_image_labelling RUNTIME DESTINATION ${EXAMPLE_EXEC_PREFIX})
ENDIF (INSTALL_EXAMPLE_APP)

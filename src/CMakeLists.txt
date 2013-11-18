# Create an executable called "indexer".
# The extension is already found. Any number of sources could be listed here.
add_executable (indexer
	indexer.cxx
	config.cxx
	data_source.cxx
	next_data_source.cxx
    index_manager.cxx
    analysis/rgb_histogram.cxx
    analysis/luv_histogram.cxx
    analysis/edge_direction_histogram.cxx
    analysis/color_coherence_vector.cxx
    analysis/surf_vector.cxx
)
add_dependencies (indexer soci)
add_dependencies (indexer sqlite)

target_link_libraries (indexer ${OpenCV_LIBS})
target_link_libraries (indexer ${Boost_LIBRARIES})
target_link_libraries (indexer ${JsonCpp_LIBRARIES})
target_link_libraries (indexer ${Uri_LIBRARIES})
target_link_libraries (indexer ${SOCI_LIBRARIES})
target_link_libraries (indexer ${SQLite3_LIBRARIES})
target_link_libraries (indexer ${CMAKE_DL_LIBS})

# Create an executable called "matcher".
# The extension is already found. Any number of sources could be listed here.
add_executable (matcher
    matcher.cxx
    matcher_service.cxx
    matcher_service_connection.cxx
    config.cxx
    index_manager.cxx)
add_dependencies (matcher soci)
add_dependencies (matcher sqlite)
add_dependencies (matcher b64)

target_link_libraries (matcher ${OpenCV_LIBS})
target_link_libraries (matcher ${Boost_LIBRARIES})
target_link_libraries (matcher ${JsonCpp_LIBRARIES})
target_link_libraries (matcher ${Uri_LIBRARIES})
target_link_libraries (matcher ${SOCI_LIBRARIES})
target_link_libraries (matcher ${SQLite3_LIBRARIES})
target_link_libraries (matcher ${CMAKE_DL_LIBS})
target_link_libraries (matcher ${Libb64_LIBRARIES})

set_target_properties (matcher
    PROPERTIES
    COMPILE_FLAGS -DBUFFERSIZE=16777216)


#pragma once

#include <string>
#include <fstream>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "graph.h"
#include "router.h"

#include <transport_catalogue.pb.h>

namespace serialization {
	
	std::string ParseFilename(json::Document& document);
	
	void SaveTo(const std::string& filename, transport_catalogue::TransportCatalogue& tc, json::Document& document);
	
	void LoadFrom(const std::string &filename, output::OutputParams& params);

} // namespace serialization
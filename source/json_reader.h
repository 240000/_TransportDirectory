#pragma once

#include "transport_router.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "graph.h"
#include "router.h"

#include <sstream>
#include <vector>
#include <string>
#include <string_view>
#include <iomanip>
#include <map>

namespace json_reader {
	
	json::Document LoadJSON(std::istream& input);
	
	void FillTransportCatalogue(json::Document& document, transport_catalogue::TransportCatalogue& tc);
	
	map_renderer::RenderSettings ParseRenderSettings(json::Document& document);
	
	transport_router::RoutingSettings ParseRoutingSettings(json::Document& document);
	
	svg::Color ColorFromJSON(const json::Node* color);

} // end namespace json_reader



namespace output {
	
	struct OutputParams {
		json::Document document;
		map_renderer::MapRenderer mr;
		transport_catalogue::TransportCatalogue tc;
		std::map<std::string_view ,transport_router::WaitingStages> edges_ids;
		transport_router::RoutingSettings rs;
		graph::DirectedWeightedGraph<transport_router::Weight> tc_graph;
	};
	
	void Output(OutputParams& output_params, graph::Router<transport_router::Weight>& router, std::ostream& output);
	
	json::Node GetBusAsNode(const std::string& name, int id, transport_catalogue::TransportCatalogue& tc);
	
	json::Node GetStopAsNode(const std::string& name, int id, transport_catalogue::TransportCatalogue& tc);
	
	json::Node GetMapAsNode(json::Document& document, int id, transport_catalogue::TransportCatalogue& tc,
	                        map_renderer::MapRenderer& mr);

	json::Node GetRouteAsNode(std::optional<typename graph::Router<transport_router::Weight>::RouteInfo>& info, int id, graph::DirectedWeightedGraph<transport_router::Weight>& tc_graph);
	
	json::Node GetErrorMessageAsNode(int id);
	
} // end namespace output





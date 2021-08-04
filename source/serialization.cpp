#include "serialization.h"

using namespace std;

namespace serialization {
	
	string ParseFilename(json::Document& document) {
		const json::Dict *settings = &document.GetRoot().AsDict().at("serialization_settings"s).AsDict();
		return settings->at("file"s).AsString();
	}
	
	void SerializeTransportCatalogue(transport_catalogue_serialize::TransportCatalogue & tcs, transport_catalogue::TransportCatalogue& tc) {
		
		for(auto& [_, stop_ptr]: tc.GetStopsAsList()) {
			transport_catalogue_serialize::Stop* stop_s = tcs.add_stops_base();
			stop_s->set_name(stop_ptr->name);
			stop_s->mutable_coordinates()->set_lat(stop_ptr->coordinates.lat);
			stop_s->mutable_coordinates()->set_lng(stop_ptr->coordinates.lng);
			for(auto& bus : stop_ptr->buses) {
				stop_s->add_buses(bus);
			}
			for(auto& [stop, dist] : stop_ptr->distances_to_stops) {
				transport_catalogue_serialize::DistsToStops* dists_to_stops = stop_s->add_distances_to_stops();
				dists_to_stops->set_stop(stop);
				dists_to_stops->set_distance(dist);
			}
		}
		for(auto& [_, bus_ptr]: tc.GetBusesAsList()) {
			transport_catalogue_serialize::Bus* bus_s = tcs.add_buses_base();
			bus_s->set_name(bus_ptr->name);
			bus_s->set_is_roundtrip(bus_ptr->is_roundtrip);
			for(auto& stop : bus_ptr->route) {
				bus_s->add_route(stop->name);
			}
		}
	}
	
	void FillProtoColor(transport_catalogue_serialize::Color* proto_color_ptr, svg::Color color) {
		if(std::holds_alternative<std::string>(color)) {
			proto_color_ptr->set_is_string(true);
			proto_color_ptr->set_is_rgb(false);
			proto_color_ptr->set_is_rgba(false);
			proto_color_ptr->set_color(get<string>(color));
		}
		else if(std::holds_alternative<svg::Rgb>(color)) {
			proto_color_ptr->set_is_string(false);
			proto_color_ptr->set_is_rgb(true);
			proto_color_ptr->set_is_rgba(false);
			proto_color_ptr->set_red(get<svg::Rgb>(color).red);
			proto_color_ptr->set_green(get<svg::Rgb>(color).green);
			proto_color_ptr->set_blue(get<svg::Rgb>(color).blue);
		}
		else if(std::holds_alternative<svg::Rgba>(color)) {
			proto_color_ptr->set_is_string(false);
			proto_color_ptr->set_is_rgb(false);
			proto_color_ptr->set_is_rgba(true);
			proto_color_ptr->set_red(get<svg::Rgba>(color).red);
			proto_color_ptr->set_green(get<svg::Rgba>(color).green);
			proto_color_ptr->set_blue(get<svg::Rgba>(color).blue);
			proto_color_ptr->set_opacity(get<svg::Rgba>(color).opacity);
		}
	}
	
	void SerializeRenderSettings(transport_catalogue_serialize::RenderSettings* s_render_settings, json::Document& document) {
		
		map_renderer::RenderSettings render_settings = json_reader::ParseRenderSettings(document);
		
		s_render_settings->set_width(render_settings.width);
		s_render_settings->set_height(render_settings.height);
		s_render_settings->set_padding(render_settings.padding);
		
		s_render_settings->set_line_width(render_settings.line_width);
		s_render_settings->set_stop_radius(render_settings.stop_radius);
		
		s_render_settings->set_bus_label_font_size(render_settings.bus_label_font_size);
		s_render_settings->set_bus_label_offset_dx(render_settings.bus_label_offset_dx);
		s_render_settings->set_bus_label_offset_dy(render_settings.bus_label_offset_dy);
		
		s_render_settings->set_stop_label_font_size(render_settings.stop_label_font_size);
		s_render_settings->set_stop_label_offset_dx(render_settings.stop_label_offset_dx);
		s_render_settings->set_stop_label_offset_dy(render_settings.stop_label_offset_dy);
		
		FillProtoColor(s_render_settings->mutable_underlayer_color(), render_settings.underlayer_color);
		
		s_render_settings->set_underlayer_width(render_settings.underlayer_width);
		
		for(auto& color : render_settings.color_palette) {
			transport_catalogue_serialize::Color* s_color = s_render_settings->add_color_palette();
			FillProtoColor(s_color, color);
		}
	}
	
	void SerializeRoutingSettings(transport_catalogue_serialize::RoutingSettings* s_routing_settings, json::Document& document) {
		
		transport_router::RoutingSettings routing_settings = json_reader::ParseRoutingSettings(document);
		
		s_routing_settings->set_bus_velocity(routing_settings.bus_velocity);
		s_routing_settings->set_bus_wait_time(routing_settings.bus_wait_time);
	}
	
	void SerializeWeight(transport_catalogue_serialize::Weight* s_weight, const transport_router::Weight& weight) {
		switch (weight.type) {
				case transport_router::ItemsType::BUS :
					s_weight->set_type(transport_catalogue_serialize::ItemsType::BUS);
					break;
				case transport_router::ItemsType::WAIT :
					s_weight->set_type(transport_catalogue_serialize::ItemsType::WAIT);
					break;
				case transport_router::ItemsType::DEFAULT :
					s_weight->set_type(transport_catalogue_serialize::ItemsType::DEFAULT);
					break;
			}
			s_weight->set_time(weight.time);
		if(weight.stop_name.has_value()) {
			s_weight->set_stop_name(*weight.stop_name.value());
		}
		if(weight.bus_name.has_value()) {
			s_weight->set_bus_name(*weight.bus_name.value());
		}
			s_weight->set_span_count(weight.span_count);
	}
	
	void SerializeDirectedWeightedGraph(transport_catalogue_serialize::DirectedWeightedGraph* s_graph,
										const graph::DirectedWeightedGraph<transport_router::Weight>& graph) {
		for(int i = 0; i < graph.GetEdgeCount(); ++i) {
			transport_catalogue_serialize::Edge *s_edge = s_graph->add_edges();
			s_edge->set_from(graph.GetEdge(i).from);
			s_edge->set_to(graph.GetEdge(i).to);
			SerializeWeight(s_edge->mutable_weight(), graph.GetEdge(i).weight);
		}
		for(int l = 0; l < graph.GetVertexCount(); ++l) {
			transport_catalogue_serialize::RepeatedIncidenceLists* s_incidence_lists = s_graph->add_incidence_lists();
			int m = 0;
			for(auto& edge : graph.GetIncidentEdges(l)) {
				s_incidence_lists->add_rep_incidence_lists(edge);
				m++;
			}
		}
	}
		
		void SaveTo(const string &filename, transport_catalogue::TransportCatalogue& tc, json::Document& document) {
		
		ofstream output(filename, ios::binary);
		transport_catalogue_serialize::TransportCatalogue tcs;
		
		map<string_view ,transport_router::WaitingStages> edges_ids;
		transport_router::RoutingSettings rs = json_reader::ParseRoutingSettings(document);
		graph::DirectedWeightedGraph<transport_router::Weight> tc_graph = transport_router::BuildGraph( tc, edges_ids, rs);
		graph::Router<transport_router::Weight> router(tc_graph);
		
		SerializeTransportCatalogue(tcs, tc);
		SerializeRenderSettings(tcs.mutable_render_settings(), document);
		SerializeRoutingSettings(tcs.mutable_routing_settings(), document);
		SerializeDirectedWeightedGraph(tcs.mutable_graph(), router.GetGraph());
		tcs.SerializePartialToOstream(&output);
	}
	
	void DeserializeTransportCatalogue(transport_catalogue_serialize::TransportCatalogue& tcs, transport_catalogue::TransportCatalogue& tc) {

		int stops_base_size = tcs.stops_base_size();
		int buses_base_size = tcs.buses_base_size();
		
		for(int i = 0; i < stops_base_size; ++i) {
			string name = tcs.stops_base(i).name();
			geo::Coordinates coordinates = {tcs.stops_base(i).coordinates().lat(),
			                                tcs.stops_base(i).coordinates().lng()};
			unordered_map<std::string, double, std::hash<std::string>> distances_to_stops;
			int dist_to_stops_size = tcs.stops_base(i).distances_to_stops_size();
			for(int j = 0; j < dist_to_stops_size; ++j) {
				distances_to_stops.insert({tcs.stops_base(i).distances_to_stops(j).stop(),
				                           tcs.stops_base(i).distances_to_stops(j).distance()});
			}
			tc.AddStop(name, coordinates, distances_to_stops);
		}
		for(int i = 0; i < buses_base_size; ++i) {
			string name = tcs.buses_base(i).name();
			bool is_roundtrip = tcs.buses_base(i).is_roundtrip();
			std::deque<std::string_view> route;
			int route_size = tcs.buses_base(i).route_size();
			for(int j = 0; j < route_size; ++j) {
				if(transport_catalogue::domain::Stop* stop_ptr = tc.IsStop(tcs.buses_base(i).route(j))) {
					route.push_back(stop_ptr->name);
				}
			}
			tc.AddBus(name, route, is_roundtrip);
		}
	}
	
	void ParseProtoColor(const transport_catalogue_serialize::Color& proto_color, svg::Color& color) {
		if(proto_color.is_string()) {
			color = proto_color.color();
		}
		else if(proto_color.is_rgb()) {
			color = svg::Rgb(proto_color.red(), proto_color.green(), proto_color.blue());
		}
		else if(proto_color.is_rgba()) {
			color = svg::Rgba(proto_color.red(), proto_color.green(), proto_color.blue(), proto_color.opacity());
		}
	}
	
	void DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& s_render_settings, map_renderer::MapRenderer& mr) {

		map_renderer::RenderSettings render_settings;
		
		render_settings.width = s_render_settings.width();
		render_settings.height = s_render_settings.height();
		render_settings.padding = s_render_settings.padding();
		
		render_settings.line_width = s_render_settings.line_width();
		render_settings.stop_radius = s_render_settings.stop_radius();
		
		render_settings.bus_label_font_size = s_render_settings.bus_label_font_size();
		render_settings.bus_label_offset_dx = s_render_settings.bus_label_offset_dx();
		render_settings.bus_label_offset_dy = s_render_settings.bus_label_offset_dy();
		
		render_settings.stop_label_font_size = s_render_settings.stop_label_font_size();
		render_settings.stop_label_offset_dx = s_render_settings.stop_label_offset_dx();
		render_settings.stop_label_offset_dy = s_render_settings.stop_label_offset_dy();
		
		ParseProtoColor(s_render_settings.underlayer_color(), render_settings.underlayer_color);
		
		render_settings.underlayer_width = s_render_settings.underlayer_width();
		
		int color_palette_size = s_render_settings.color_palette_size();
		
		for(int i = 0; i < color_palette_size; ++i) {
			svg::Color color;
			ParseProtoColor(s_render_settings.color_palette(i), color);
			render_settings.color_palette.push_back(color);
		}
		
		render_settings.is_empty = false;
		
		mr.SetRenderSettings(render_settings);
		
	}
	
	void DeserializeRoutingSettings(const transport_catalogue_serialize::RoutingSettings& s_routing_settings,
	                                transport_router::RoutingSettings& routing_settings) {
		
		routing_settings.bus_velocity = s_routing_settings.bus_velocity();
		routing_settings.bus_wait_time = s_routing_settings.bus_wait_time();
	}
	
	void DeserializeWeight(const transport_catalogue_serialize::Weight& s_weight, transport_router::Weight& weight,
	                       transport_catalogue::TransportCatalogue& tc) {
		
			switch(s_weight.type()) {
				case transport_catalogue_serialize::ItemsType::BUS :
					weight.type = transport_router::ItemsType::BUS;
					break;
				case transport_catalogue_serialize::ItemsType::WAIT :
					weight.type = transport_router::ItemsType::WAIT;
					break;
				default:
					weight.type = transport_router::ItemsType::DEFAULT;
					break;
			}
			weight.time = s_weight.time();
			weight.stop_name = &tc.IsStop(s_weight.stop_name())->name;
			weight.bus_name = &tc.IsBus(s_weight.bus_name())->name;
			weight.span_count = s_weight.span_count();
		}
	
	void DeserializeDirectedWeightedGraph(const transport_catalogue_serialize::DirectedWeightedGraph& s_graph,
	                                      graph::DirectedWeightedGraph<transport_router::Weight>& g,
	                                      transport_catalogue::TransportCatalogue& tc) {
		graph::DirectedWeightedGraph<transport_router::Weight> graph(s_graph.incidence_lists_size());
		for(int i = 0; i < s_graph.edges_size(); ++i) {
			transport_router::Weight weight;
			DeserializeWeight(s_graph.edges(i).weight(), weight, tc);
			graph.AddEdge({s_graph.edges(i).from(), s_graph.edges(i).to(), weight});
		}
	}
	
	void LoadFrom(const string &filename, output::OutputParams& params) {
		ifstream input(filename, ios::binary);
		transport_catalogue_serialize::TransportCatalogue tcs;
		tcs.ParseFromIstream(&input);
		
		DeserializeTransportCatalogue(tcs, params.tc);
		DeserializeRenderSettings(tcs.render_settings(), params.mr);
		DeserializeRoutingSettings(tcs.routing_settings(), params.rs);
		DeserializeDirectedWeightedGraph(tcs.graph(), params.tc_graph, params.tc);
	}
	
} // end namespace serialization
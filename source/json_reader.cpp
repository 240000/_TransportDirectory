#include "json_reader.h"

using namespace std;

namespace json_reader {
	
	json::Document LoadJSON(std::istream& input) {
		return json::Load(input);
	}
	
	void FillTransportCatalogue(json::Document& document, transport_catalogue::TransportCatalogue& tc) {
		for (const json::Node &stop : document.GetRoot().AsDict().at("base_requests"s).AsArray()) {
			if (stop.AsDict().at("type"s).AsString() == "Stop"sv) {
				string name = stop.AsDict().at("name"s).AsString();
				geo::Coordinates coordinates = {stop.AsDict().at("latitude"s).AsDouble(),
				                                stop.AsDict().at("longitude"s).AsDouble()};
				unordered_map<std::string, double, std::hash<std::string>> distances_to_stops;
				for(auto& [to_stop, dist] : stop.AsDict().at("road_distances"s).AsDict()){
					distances_to_stops.insert({to_stop, dist.AsDouble()});
				}
				tc.AddStop(name, coordinates, distances_to_stops);
			}
		}
		for (const json::Node &bus : document.GetRoot().AsDict().at("base_requests"s).AsArray()) {
			if (bus.AsDict().at("type"s).AsString() == "Bus"s) {
				string name = bus.AsDict().at("name"s).AsString();
				bool is_roundtrip = bus.AsDict().at("is_roundtrip"s).AsBool();
				std::deque<std::string_view> route;
				for(auto& stop : bus.AsDict().at("stops"s).AsArray()) {
					if(transport_catalogue::domain::Stop* stop_ptr = tc.IsStop(stop.AsString())) {
						route.push_back(stop_ptr->name);
					}
				}
				tc.AddBus(name, route, is_roundtrip);
			}
		}
		
	}
	
	map_renderer::RenderSettings ParseRenderSettings(json::Document& document) {
		map_renderer::RenderSettings rs;
		if(document.GetRoot().AsDict().count("render_settings"s) != 0) {
			const json::Dict *settings = &document.GetRoot().AsDict().at("render_settings"s).AsDict();
			rs.width = settings->at("width"s).AsDouble();
			rs.height = settings->at("height"s).AsDouble();
			rs.padding = settings->at("padding"s).AsDouble();
			rs.line_width = settings->at("line_width"s).AsDouble();
			rs.stop_radius = settings->at("stop_radius"s).AsDouble();
			rs.bus_label_font_size = settings->at("bus_label_font_size"s).AsInt();
			rs.bus_label_offset_dx = settings->at("bus_label_offset"s).AsArray().front().AsDouble();
			rs.bus_label_offset_dy = settings->at("bus_label_offset"s).AsArray().back().AsDouble();
			rs.stop_label_font_size = settings->at("stop_label_font_size"s).AsInt();
			rs.stop_label_offset_dx = settings->at("stop_label_offset"s).AsArray().front().AsDouble();
			rs.stop_label_offset_dy = settings->at("stop_label_offset"s).AsArray().back().AsDouble();
			rs.underlayer_color = ColorFromJSON(&settings->at("underlayer_color"s));
			rs.underlayer_width = settings->at("underlayer_width"s).AsDouble();
			if (settings->at("color_palette"s).IsArray()) {
				const json::Array *colors = &settings->at("color_palette"s).AsArray();
				for (auto &color : *colors) {
					rs.color_palette.emplace_back(ColorFromJSON(&color));
				}
			}
			rs.is_empty = false;
		}
		return rs;
	}
	
	transport_router::RoutingSettings ParseRoutingSettings(json::Document& document) {
		if(document.GetRoot().AsDict().count("routing_settings"s) != 0) {
			const map<string, json::Node> &dict = document.GetRoot().AsDict().at("routing_settings"s).AsDict();
			double bus_wait_time = dict.at("bus_wait_time").AsDouble();
			double bus_velocity = dict.at("bus_velocity").AsDouble();
			return {bus_wait_time, bus_velocity};
		}
		return {};
	}
	
	svg::Color ColorFromJSON(const json::Node* color) {
		svg::Color c;
		if(color->IsString()) {
			c = color->AsString();
		}
		else if(color->IsArray()) {
			const json::Array *color_array = &color->AsArray();
			if(color_array->size() == 3) {
				c =  svg::Rgb{color_array->at(0).AsInt(), color_array->at(1).AsInt(), color_array->at(2).AsInt() };
			}
			else if(color_array->size() == 4) {
				c =  svg::Rgba{color_array->at(0).AsInt(), color_array->at(1).AsInt(),
				               color_array->at(2).AsInt(), color_array->at(3).AsDouble() };
			}
		}
		return c;
	}
	
} // end namespace json_reader



namespace output {
	
	void Output(OutputParams& output_params, graph::Router<transport_router::Weight>& router, std::ostream& output) {
	
		
		json::Array response;
		
		for (const json::Node &r : output_params.document.GetRoot().AsDict().at("stat_requests"s).AsArray()) {
			const json::Dict *request_ptr = &r.AsDict();
			if (request_ptr->at("type"s).AsString() == "Bus"s) {
				response.push_back(std::move(GetBusAsNode(request_ptr->at("name"s).AsString()
						, request_ptr->at("id").AsInt()
						, output_params.tc
				                             )
				                   )
				);
			}
			else if (request_ptr->at("type"s).AsString() == "Stop"s) {
				response.push_back(std::move(GetStopAsNode(request_ptr->at("name"s).AsString()
						, request_ptr->at("id").AsInt()
						, output_params.tc
				                             )
				                   )
				);
			}
			else if (request_ptr->at("type"s).AsString() == "Map"s) {
				response.push_back(std::move(GetMapAsNode(output_params.document, request_ptr->at("id").AsInt(),
																output_params.tc, output_params.mr)));
			}
			
			else if (request_ptr->at("type"s).AsString() == "Route"s) {
				size_t vertex_from = output_params.edges_ids.at(request_ptr->at("from"s).AsString()).start_waiting;
				size_t vertex_to = output_params.edges_ids.at(request_ptr->at("to"s).AsString()).start_waiting;
				auto info = router.BuildRoute(vertex_from, vertex_to);
				response.push_back(std::move(GetRouteAsNode( info
						, request_ptr->at("id").AsInt()
						, output_params.tc_graph
				                             )
				                   )
				);
			}
		}
		json::Print(
				json::Document{
						json::Builder{}
								.Value(response)
								.Build()
				},
				output
		);
		output << endl;
	}
	
	json::Node GetBusAsNode(const string& name, int id, transport_catalogue::TransportCatalogue& tc) {
		if (transport_catalogue::domain::Bus* bus_ptr = tc.IsBus(name)) {
			if(bus_ptr->is_response_empty || tc.buses_response[bus_ptr->name].is_empty) {
				tc.GetRouteInfo(name);
			}
			return json::Node{
					json::Builder{}
							.StartDict()
							.Key("curvature"s).Value(tc.buses_response[bus_ptr->name].curvature)
							.Key("request_id"s).Value(id)
							.Key("route_length"s).Value(tc.buses_response[bus_ptr->name].real_route_length)
							.Key("stop_count"s).Value(tc.buses_response[bus_ptr->name].stops_count)
							.Key("unique_stop_count"s).Value(tc.buses_response[bus_ptr->name].unique_stop_count)
							.EndDict()
							.Build()
			};
		}
		else {
			return GetErrorMessageAsNode(id);
		}
	}
	
	json::Node GetStopAsNode(const string& name, int id, transport_catalogue::TransportCatalogue& tc) {
		if (transport_catalogue::domain::Stop* stop_ptr = tc.IsStop(name)) {
			if(stop_ptr->is_response_empty || tc.stops_response[stop_ptr->name].is_empty) {
				tc.SearchBusesByStop(name);
				for (std::string bus : tc.stops_response[stop_ptr->name].sort_buses) {
					tc.stops_response[stop_ptr->name].buses_array.push_back(bus);
				}
				tc.stops_response[stop_ptr->name].is_empty = false;
				stop_ptr->is_response_empty = false;
			}
			return json::Node{
					json::Builder{}
							.StartDict()
							.Key("buses"s).Value(tc.stops_response[stop_ptr->name].buses_array)
							.Key("request_id"s).Value(id)
							.EndDict()
							.Build()
			};
		}
		else {
			return GetErrorMessageAsNode(id);
		}
	}
	json::Node GetMapAsNode(json::Document& document, int id, transport_catalogue::TransportCatalogue& tc,
	                        map_renderer::MapRenderer& mr) {
		if(mr.cache.string_empty) {
			if(mr.IsRenderSettingsEmpty()) {
				mr.FillMapRenderer(move(json_reader::ParseRenderSettings(document)), tc.GetBusesAsList(), tc.GetStopsAsList());
			}
			else {
				mr.FillMapRenderer(tc.GetBusesAsList(), tc.GetStopsAsList());
			}
			mr.RenderAsString();
		}
		return json::Node{
				json::Builder{}
						.StartDict()
						.Key("map"s).Value(mr.cache.string_response.str())
						.Key("request_id"s).Value(id)
						.EndDict()
						.Build()
		};
	}
	
	
	json::Node GetRouteAsNode(std::optional<typename graph::Router<transport_router::Weight>::RouteInfo>& info, int id, graph::DirectedWeightedGraph<transport_router::Weight>& tc_graph) {
		
		if(info.has_value()) {
			
			double total_time = 0.0;
			json::Array arr;
			for (auto &item : info->edges) {
				auto edge = tc_graph.GetEdge(item);
				switch (edge.weight.type) {
					case transport_router::WAIT :
						arr.push_back({ json::Builder{}
								                .StartDict()
								                .Key("type"s).Value("Wait"s)
								                .Key("stop_name"s).Value(*edge.weight.stop_name.value())
								                .Key("time"s).Value(edge.weight.time)
								                .EndDict()
								                .Build()
						              });
						break;
					case transport_router::BUS :
						arr.push_back({ json::Builder{}
								                .StartDict()
								                .Key("type"s).Value("Bus"s)
								                .Key("bus"s).Value(*edge.weight.bus_name.value())
								                .Key("span_count"s).Value(edge.weight.span_count)
								                .Key("time"s).Value(edge.weight.time)
								                .EndDict()
								                .Build()
						              });
						break;
					case transport_router::DEFAULT :
						break;
				}
				total_time += edge.weight.time;
			}
			
			return json::Node{
					json::Builder{}
							.StartDict()
							.Key("request_id"s).Value(id)
							.Key("total_time"s).Value(total_time)
							.Key("items"s).Value(arr)
							.EndDict()
							.Build()
				
			};
		}
		else {
			return GetErrorMessageAsNode(id);
		}
	}
	
	json::Node GetErrorMessageAsNode(int id) {
		return json::Node{
				json::Builder{}
						.StartDict()
						.Key("request_id"s).Value(id)
						.Key("error_message"s).Value("not found"s)
						.EndDict()
						.Build()
		};
	}
	
} // end namespace output
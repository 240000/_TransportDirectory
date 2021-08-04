#pragma once

#include "geo.h"
#include "json.h"

#include <deque>
#include <string_view>
#include <unordered_map>
#include <set>
#include <string>

namespace transport_catalogue {

	namespace domain {
		
		struct StopQueryResponse {
			bool is_empty = true;
			std::set<std::string> sort_buses;
			json::Array buses_array;
		};
		struct BusQueryResponse {
			bool is_empty = true;
			double curvature = 0.0;
			double real_route_length = 0.0;
			int stops_count = 0;
			int unique_stop_count = 0;
		};
		
		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
			std::deque<std::string> buses;
			std::unordered_map<std::string, double, std::hash<std::string>> distances_to_stops;
			bool is_response_empty = true;
		};
		
		struct Bus {
			std::string name;
			std::deque<domain::Stop *> route;
			bool is_roundtrip;
			bool is_response_empty = true;
		};
		
		
	} // end namespace domain
	
} //namespace transport_catalogue
#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <deque>
#include <map>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {
	
	class TransportCatalogue
	{
	public:
		void AddBus(std::string& bus_name, std::deque<std::string_view>& stops, bool is_roundtrip);
		void AddStop(std::string& stop_name, geo::Coordinates& coordinates, std::unordered_map<std::string, double, std::hash<std::string>>& distances_to_stops);
		const std::set<std::string>& SearchBusesByStop(std::string_view stop);
		domain::Stop* IsStop(std::string_view stop);
		domain::Bus* IsBus(std::string_view bus);
		void ComputeAllDistance(domain::Bus *bus_ptr);
		domain::BusQueryResponse GetRouteInfo(std::string_view bus);
		
		const std::map<std::string_view, domain::Bus*>& GetBusesAsList();
		const std::map<std::string_view, domain::Stop*>& GetStopsAsList();
		
		std::map<std::string_view, domain::BusQueryResponse> buses_response;
		std::map<std::string_view, domain::StopQueryResponse> stops_response;
		
	private:
		std::map<std::string_view, domain::Bus*> buses_;
		std::map<std::string_view, domain::Stop*> stops_;
		std::deque<domain::Bus> buses_base_;
		std::deque<domain::Stop> stops_base_;
	};
	
	
} // end transport_catalogue
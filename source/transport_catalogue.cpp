#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue
{
	
	void TransportCatalogue::AddBus(std::string& bus_name, std::deque<std::string_view>& stops, bool is_roundtrip) {
		buses_base_.push_front({ move(bus_name), {}, is_roundtrip, {} });
		for(auto& stop : stops) {
			if(domain::Stop* stop_ptr = IsStop(stop)) {
				buses_base_.front().route.push_back(stop_ptr);
				stop_ptr->buses.push_back(buses_base_.front().name);
			}
		}
		buses_[buses_base_.front().name] = &buses_base_.front();
	}
	
	void TransportCatalogue::AddStop(std::string& stop_name, geo::Coordinates& coordinates,
	                                 std::unordered_map<std::string, double, hash<string>>& distances_to_stops)
	{
		stops_base_.push_front({ move(stop_name), coordinates, {}, move(distances_to_stops), {} });
		stops_[stops_base_.front().name] = &stops_base_.front();
	}
	
	const set<string>& TransportCatalogue::SearchBusesByStop(string_view stop)
	{
		domain::Stop *stop_ptr = IsStop(stop);
		stops_response[stop_ptr->name].sort_buses = {stop_ptr->buses.begin(), stop_ptr->buses.end()};
		return stops_response[stop_ptr->name].sort_buses;
	}
	
	domain::Stop *TransportCatalogue::IsStop(string_view stop)
	{
		auto it = stops_.find(stop);
		if (it != stops_.end()) {
			return it->second;
		}
		return nullptr;
	}
	
	domain::Bus *TransportCatalogue::IsBus(string_view bus)
	{
		auto it = buses_.find(bus);
		if (it != buses_.end()) {
			return it->second;
		}
		return nullptr;
	}
	
	void TransportCatalogue::ComputeAllDistance(domain::Bus *bus_ptr)
	{
		double real_route_length = 0.0;
		double geo_route_length = 0.0;
		if(!bus_ptr->route.empty()) {
			for (auto it = next(bus_ptr->route.begin()); it != bus_ptr->route.end(); ++it) {
				if (stops_[(*prev(it))->name]->distances_to_stops.count(stops_[(*it)->name]->name) != 0) {
					real_route_length += stops_[(*prev(it))->name]->distances_to_stops.at(stops_[(*it)->name]->name);
				}
				else {
					real_route_length += stops_[(*it)->name]->distances_to_stops.at(stops_[(*prev(it))->name]->name);
				}
				if (bus_ptr->route.size() > 1) {
					geo_route_length += geo::ComputeDistance(stops_[(*prev(it))->name]->coordinates, stops_[(*it)->name]->coordinates);
				}
			}
			if(!bus_ptr->is_roundtrip) {
				for (auto it = next(bus_ptr->route.rbegin()); it != bus_ptr->route.rend(); ++it) {
					if (stops_[(*prev(it))->name]->distances_to_stops.count(stops_[(*it)->name]->name) != 0) {
						real_route_length += stops_[(*prev(it))->name]->distances_to_stops.at(stops_[(*it)->name]->name);
					}
					else {
						real_route_length += stops_[(*it)->name]->distances_to_stops.at(stops_[(*prev(it))->name]->name);
					}
					if (bus_ptr->route.size() > 1) {
						geo_route_length += geo::ComputeDistance(stops_[(*prev(it))->name]->coordinates, stops_[(*it)->name]->coordinates);
					}
				}
			}
		}
		if (bus_ptr->route.size() > 1) {
			buses_response[bus_ptr->name].real_route_length = real_route_length;
			buses_response[bus_ptr->name].curvature = real_route_length / geo_route_length;
		}
	}
	
	domain::BusQueryResponse TransportCatalogue::GetRouteInfo(string_view bus) {
		domain::Bus *bus_ptr = IsBus(bus);
		buses_response[bus_ptr->name].stops_count = buses_[bus_ptr->name]->route.size();
		if(!bus_ptr->is_roundtrip) {
			buses_response[bus_ptr->name].stops_count += buses_[bus_ptr->name]->route.size() - 1;
		}
		set<domain::Stop*> temp{buses_[bus_ptr->name]->route.begin(), buses_[bus_ptr->name]->route.end()};
		buses_response[bus_ptr->name].unique_stop_count = temp.size();
		ComputeAllDistance(bus_ptr);
		buses_response[bus_ptr->name].is_empty = false;
		bus_ptr->is_response_empty = false;
		return buses_response[bus_ptr->name];
	}
	
	const std::map<std::string_view, domain::Bus*>& TransportCatalogue::GetBusesAsList() {
		return buses_;
	}
	const std::map<std::string_view, domain::Stop*>& TransportCatalogue::GetStopsAsList() {
		return stops_;
	}
	
} // end namespace transport_catalogue
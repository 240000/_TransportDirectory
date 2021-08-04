#include "transport_router.h"

using namespace std;

namespace transport_router {
	
	bool operator<(const Weight &lhs, const Weight &rhs) {
		return lhs.time < rhs.time;
	}
	bool operator>(const Weight &lhs, const Weight &rhs) {
		return !(lhs < rhs);
	}
	bool operator==(const Weight &lhs, const Weight &rhs) {
		return lhs.time == rhs.time;
	}
	bool operator!=(const Weight &lhs, const Weight &rhs) {
		return !(lhs == rhs);
	}
	
	Weight operator+(const Weight &lhs, const Weight &rhs) {
		Weight w = lhs;
		w.time += rhs.time;
		return w;
	}
	Weight &operator+=(Weight &lhs, const Weight &rhs) {
		lhs.time += rhs.time;
		return lhs;
	}
	
	template <typename StopIterator>
	double DistanceAdding(StopIterator prev_stop, StopIterator current_stop, double dist) {
		if ((*prev_stop)->distances_to_stops.count((*current_stop)->name) != 0) {
			dist += (*prev_stop)->distances_to_stops.at((*current_stop)->name);
		}
		else {
			dist += (*current_stop)->distances_to_stops.at((*prev_stop)->name);
		}
		return dist;
	}
	
	double TimeCalculating(double dist, double bus_velocity) {
		const int meters_in_km = 1000;
		const int minutes_in_hour = 60;
		
		return dist / meters_in_km / bus_velocity * minutes_in_hour;
	}
	
	graph::DirectedWeightedGraph<Weight> BuildGraph(transport_catalogue::TransportCatalogue &tc, std::map<std::string_view, WaitingStages> &edges_ids, RoutingSettings rs) {
		auto &stops_as_list = tc.GetStopsAsList();
		graph::DirectedWeightedGraph<Weight> tc_graph(stops_as_list.size() * 2);
		
		size_t id = 0;
		for (auto &stop : stops_as_list) {
			edges_ids[stop.second->name].start_waiting = id++;
			edges_ids[stop.second->name].finish_waiting = id++;
			tc_graph.AddEdge({ edges_ids[stop.second->name].start_waiting
					                 , edges_ids[stop.second->name].finish_waiting
					                 , Weight()
					                   .SetType(ItemsType::WAIT)
					                   .SetTime(rs.bus_wait_time)
					                   .SetStopName(&stop.second->name)
			                 });
		}
		
		auto &buses_as_list = tc.GetBusesAsList();
		for (auto &bus : buses_as_list) {
			double distance_from_begin = 0.0;
			int span_count_from_begin = 0;
			for (auto i_stop_it = next(bus.second->route.begin()); i_stop_it != bus.second->route.end(); ++i_stop_it) {
				distance_from_begin = DistanceAdding(prev(i_stop_it), i_stop_it, distance_from_begin);
				double time_from_begin = TimeCalculating(distance_from_begin, rs.bus_velocity);
				++span_count_from_begin;
				tc_graph.AddEdge({edges_ids.at(bus.second->route.front()->name).finish_waiting
						                 , edges_ids.at((*i_stop_it)->name).start_waiting
						                 , Weight()
						                  .SetType(ItemsType::BUS)
						                  .SetTime(time_from_begin)
						                  .SetBusName(&bus.second->name)
						                  .SetSpanCount(span_count_from_begin)
				                 });
				double distance_from_current = 0.0;
				int span_count_from_current = 0;
				for (auto j_stop_it = next(i_stop_it); j_stop_it != bus.second->route.end(); ++j_stop_it) {
					distance_from_current = DistanceAdding(prev(j_stop_it), j_stop_it, distance_from_current);
					double time_from_current = TimeCalculating(distance_from_current, rs.bus_velocity);
					++span_count_from_current;
					tc_graph.AddEdge({edges_ids.at((*i_stop_it)->name).finish_waiting
							                 , edges_ids.at((*j_stop_it)->name).start_waiting
							                 , Weight()
							                  .SetType(ItemsType::BUS)
							                  .SetTime(time_from_current)
							                  .SetBusName(&bus.second->name)
							                  .SetSpanCount(span_count_from_current)
					                 });
				}
			}
			if (!bus.second->is_roundtrip) {
				double distance_from_end = 0.0;
				int span_count_from_end = 0;
				for (auto i_stop_it = next(bus.second->route.rbegin()); i_stop_it != bus.second->route.rend(); ++i_stop_it) {
					distance_from_end = DistanceAdding(prev(i_stop_it), i_stop_it, distance_from_end);
					double time_from_end = TimeCalculating(distance_from_end, rs.bus_velocity);
					++span_count_from_end;
					tc_graph.AddEdge({edges_ids.at(bus.second->route.back()->name).finish_waiting
							                 , edges_ids.at((*i_stop_it)->name).start_waiting
							                 , Weight()
							                  .SetType(ItemsType::BUS)
							                  .SetTime(time_from_end)
							                  .SetBusName(&bus.second->name)
							                  .SetSpanCount(span_count_from_end)
					                 });
					double distance_from_reverse_current = 0.0;
					int span_count_from_reverse = 0;
					for (auto j_stop_it = next(i_stop_it); j_stop_it != bus.second->route.rend(); ++j_stop_it) {
						distance_from_reverse_current = DistanceAdding(prev(j_stop_it), j_stop_it, distance_from_reverse_current);
						double time_from_reverse_current = TimeCalculating(distance_from_reverse_current, rs.bus_velocity);
						++span_count_from_reverse;
						tc_graph.AddEdge({edges_ids.at((*i_stop_it)->name).finish_waiting
								                 , edges_ids.at((*j_stop_it)->name).start_waiting
								                 ,Weight()
								                  .SetType(ItemsType::BUS)
								                  .SetTime(time_from_reverse_current)
								                  .SetBusName(&bus.second->name)
								                  .SetSpanCount(span_count_from_reverse)
						                 });
					}
				}
			}
		}
		
		return tc_graph;
	}
	
} // end namespace transport_router
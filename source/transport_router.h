#pragma once

#include "graph.h"
#include "transport_catalogue.h"

#include <string_view>
#include <string>
#include <map>


namespace transport_router {
	
	struct RoutingSettings {
		double bus_wait_time = 0.0;
		double bus_velocity = 0.0;
	};
	
	enum ItemsType {
		WAIT,
		BUS,
		DEFAULT
	};
	
	struct Weight {
		
		ItemsType type;
		double time = 0.0;
		std::optional<std::string*> stop_name;
		std::optional<std::string*> bus_name;
		int span_count = 0;
		
		Weight SetType(ItemsType _type) {
			type = _type;
			return *this;
		}
		Weight SetTime(double _time) {
			time = _time;
			return *this;
		}
		Weight SetStopName(std::string* _stop_name) {
			stop_name = _stop_name;
			return *this;
		}
		Weight SetBusName(std::string* _bus_name) {
			bus_name = _bus_name;
			return *this;
		}
		
		Weight SetSpanCount(int _span_count) {
			span_count = _span_count;
			return *this;
		}
	};
	
	bool operator<(const Weight &lhs, const Weight &rhs);
	bool operator>(const Weight &lhs, const Weight &rhs);
	bool operator==(const Weight &lhs, const Weight &rhs);
	bool operator!=(const Weight &lhs, const Weight &rhs);
	
	Weight operator+(const Weight &lhs, const Weight &rhs);
	Weight &operator+=(Weight &lhs, const Weight &rhs);
	
	struct WaitingStages {
		size_t start_waiting = 0;
		size_t finish_waiting = 0;
	};
	
	graph::DirectedWeightedGraph<Weight> BuildGraph(transport_catalogue::TransportCatalogue& tc, std::map<std::string_view , WaitingStages>& edges_ids, RoutingSettings rs);
	
} // end namespace transport_router
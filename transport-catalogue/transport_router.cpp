#include "transport_router.h"

using namespace graph;
using namespace transport_router;

bool transport_router::operator<(const RouteData& left, const RouteData& right) {
	return left.total_time_ < right.total_time_;
}

RouteData transport_router::operator+(const RouteData& left, const RouteData& right) {
	RouteData result;
	result.total_time_ = left.total_time_ + right.total_time_;
	return result;
}

bool transport_router::operator>(const RouteData& left, const RouteData& right) {
	return left.total_time_ > right.total_time_;
}

TransportRouter::TransportRouter(transport_catalogue::TransportCatalogue& TC, RouterSettings& router_settings)
	: transport_catalogue_(TC), graph_(TC.GetStops().size()), router_settings_(router_settings)
{
	InitializeIdContainers();
	InitializeGraph();
	router_ = std::make_unique<graph::Router<RouteData>>(graph_);
}

std::optional<std::vector<RouterInfo>> TransportRouter::FindFastestRoute(const std::string from, const std::string to) const {
	if (from == to) {
		return std::vector<RouterInfo>{};
	}
	auto from_id = ids_to_stops.at(from);
	auto to_id = ids_to_stops.at(to);
	auto route = router_->BuildRoute(from_id, to_id);
	if (!route) {
		return std::nullopt;
	}

	std::vector<RouterInfo> result;
	for (auto edge_id : route->edges) {
		const auto& edge = graph_.GetEdge(edge_id);
		RouterInfo route_info;
		route_info.bus_name = edge.weight.bus_name_;
		route_info.stop_from = stops_to_id.at(edge.from)->name_;
		route_info.stop_to = stops_to_id.at(edge.to)->name_;
		route_info.span_count = edge.weight.span_count_;
		route_info.total_time = edge.weight.total_time_;
		result.push_back(route_info);
	}
	return result;
}

graph::Edge<RouteData> TransportRouter::ConstructEdgeFromBus(const transport_catalogue::Bus& bus, size_t stop_from_index, size_t stop_to_index, double total_time) {
	graph::Edge<RouteData> edge;
	edge.from = ids_to_stops.at(bus.stops_of_bus_.at(static_cast<size_t>(stop_from_index))->name_);
	edge.to = ids_to_stops.at(bus.stops_of_bus_.at(static_cast<size_t>(stop_to_index))->name_);
	edge.weight.bus_name_ = bus.name_;
	edge.weight.span_count_ = static_cast<int>(stop_to_index - stop_from_index);
	edge.weight.total_time_ = total_time;
	return edge;
}

double TransportRouter::ComputeTimeFromBus(const transport_catalogue::Bus& bus, int stop_from_index, int stop_to_index) {
	auto distance = transport_catalogue_.GetDistanceBetweenStops(bus.stops_of_bus_.at(static_cast<size_t>(stop_from_index))->name_, bus.stops_of_bus_.at(static_cast<size_t>(stop_to_index))->name_);
	return distance / router_settings_.bus_velocity_;
}

void TransportRouter::InitializeIdContainers() {
	size_t id_num = 0;
	const auto& stops = transport_catalogue_.GetStops();
	stops_to_id.reserve(stops.size());
	ids_to_stops.reserve(stops.size());
	for (const auto& stop : stops) {
		stops_to_id.insert({ id_num, &stop });
		ids_to_stops.insert({ stop.name_, id_num });
		++id_num;
	}
}

void TransportRouter::InitializeGraph() {
	for (const auto& bus : transport_catalogue_.GetBuses()) {
		int stops_count = bus.stops_of_bus_.size();
		for (int i = 0; i < stops_count - 1; ++i) {
			double total_time = router_settings_.bus_wait_time_;
			double total_time_backward = router_settings_.bus_wait_time_;
			for (int j = i + 1; j < stops_count; ++j) {
				total_time += ComputeTimeFromBus(bus, j - 1, j);
				graph::Edge<RouteData> edge = ConstructEdgeFromBus(bus, i, j, total_time);
				graph_.AddEdge(edge);

				if (!bus.is_looped_) {
					int i_reverse = stops_count - 1 - i;
					int j_reverse = stops_count - 1 - j;
					total_time_backward += ComputeTimeFromBus(bus, j_reverse + 1, j_reverse);
					graph::Edge<RouteData> edge = ConstructEdgeFromBus(bus, i_reverse, j_reverse, total_time_backward);
					graph_.AddEdge(edge);
				}
			}
		}
	}
}

const RouterSettings TransportRouter::GetRouterSettings() const {
	return router_settings_;
}
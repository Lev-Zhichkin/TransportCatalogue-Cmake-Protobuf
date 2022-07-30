#include "domain.h"

namespace domain
{
	StopStat::StopStat(const std::string_view stop_name, const std::set<std::string_view>& buses)
		: stop_name(stop_name), buses(buses) {}

	BusStat::BusStat(const std::string_view bus_num, const int stops_on_route, const int unique_stops, const int64_t route_length, const double curvature)
		: bus_num(bus_num), stops_on_route(stops_on_route), unique_stops(unique_stops), route_length(route_length), curvature(curvature) {}

} // namespace domain
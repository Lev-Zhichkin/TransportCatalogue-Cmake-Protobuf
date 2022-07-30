#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <string>
#include <set>
#include <unordered_set>
#include <vector>

namespace domain
{
	struct RoutingSettings
	{
		int bus_wait_time = 0;
		double bus_velocity = 0.0;
	};

	struct StopStat
	{
		StopStat(const std::string_view stop_name, const std::set<std::string_view>& buses);
		std::string stop_name;
		std::set<std::string_view> buses;
	};

	struct BusStat
	{
		BusStat(const std::string_view bus_num, const int stops_on_route, const int unique_stops, const int64_t route_length, const double curvature);
		std::string bus_num;
		int stops_on_route;
		int unique_stops;
		int64_t route_length;
		double curvature;
	};

} // namespace domain
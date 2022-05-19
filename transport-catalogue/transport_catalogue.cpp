#include "transport_catalogue.h"

using namespace transport_catalogue;

void TransportCatalogue::AddStop(string& name, double latitude, double longitude) {
	Stop stop(name, latitude, longitude);
	stops_.insert(std::move(stop));
	buses_to_stops[FindStop(name).name_];
}

void TransportCatalogue::AddBus(string& name, std::vector<const Stop*>& stops_of_bus, bool is_looped) {
	Bus bus(name, stops_of_bus, is_looped);
	buses_.insert(std::move(bus));
	const Bus* from_buses_ = &FindBus(name);
	stops_to_buses[from_buses_->name_];
	for (const Stop* stop : from_buses_->stops_of_bus_) {
		buses_to_stops[stop->name_].insert(from_buses_->name_);
		stops_to_buses[from_buses_->name_].insert(stop->name_);
	}
}

const Stop& TransportCatalogue::FindStop(const string& name) {
	for (const Stop& stop : stops_) {
		if (stop.name_ == name) {
			return stop;
		}
	}
	static Stop stop("Error");
	return stop;
}

const Bus& TransportCatalogue::FindBus(const string& name) {
	for (const Bus& bus : buses_) {
		if (bus.name_ == name) {
			return bus;
		}
	}
	static Bus bus("Error");
	return bus;
}

void TransportCatalogue::SetBusInfo(const string& name) {

	BusInfo bi;
	const Bus& bus = FindBus(name);

	if (bus.is_looped_) {
		bus_infos[bus.name_] = transport_catalogue::SetBusInfo_parts::create_looped_bus_info(*this, bi, bus);
	}
	else {
		bus_infos[bus.name_] = transport_catalogue::SetBusInfo_parts::create_turning_bus_info(*this, bi, bus);
	}

}

objects::BusInfo& SetBusInfo_parts::create_looped_bus_info(transport_catalogue::TransportCatalogue& transport_catalogue, objects::BusInfo& bi, const objects::Bus& bus) {

	bi.stops_num_ = bus.stops_of_bus_.size();

	{
		std::vector<const Stop*> check_vec = bus.stops_of_bus_;
		std::sort(check_vec.begin(), check_vec.end());
		check_vec.erase(unique(check_vec.begin(), check_vec.end()), check_vec.end());
		bi.unique_stops_num_ = check_vec.size();
	}

	double distance = 0;
	for (auto stop_it = bus.stops_of_bus_.begin(); ; ) {
		Stop from = **stop_it;
		++stop_it;
		if (stop_it == bus.stops_of_bus_.end()) { break; }
		Stop to = **stop_it;
		distance += geo::ComputeDistance(from.coordinates, to.coordinates);
		if (stop_it == --bus.stops_of_bus_.end()) {
			Stop loop_end = **bus.stops_of_bus_.begin();
			distance += geo::ComputeDistance(to.coordinates, loop_end.coordinates);
		}
	}
	bi.distance_length_ = distance;

	double real_distance = 0;
	for (size_t i = 0; ; ++i) {
		if (i == bus.stops_of_bus_.size() - 1) {
			real_distance += transport_catalogue.GetDistanceBetweenStops(bus.stops_of_bus_[bus.stops_of_bus_.size() - 1], bus.stops_of_bus_[0]);
			break;
		}
		real_distance += transport_catalogue.GetDistanceBetweenStops(bus.stops_of_bus_[i], bus.stops_of_bus_[i + 1]);
	}
	bi.real_distance_length_ = real_distance;

	bi.curvature_ = real_distance / distance;

	return bi;
}

objects::BusInfo& SetBusInfo_parts::create_turning_bus_info(transport_catalogue::TransportCatalogue& transport_catalogue, objects::BusInfo& bi, const objects::Bus& bus) {

	bi.stops_num_ = (bus.stops_of_bus_.size() * 2) - 1;

	{
		std::vector<const Stop*> check_vec = bus.stops_of_bus_;
		std::sort(check_vec.begin(), check_vec.end());
		check_vec.erase(unique(check_vec.begin(), check_vec.end()), check_vec.end());
		bi.unique_stops_num_ = check_vec.size();
	}

	double distance = 0;
	for (auto stop_it = bus.stops_of_bus_.begin(); ; ) {
		Stop from = **stop_it;
		++stop_it;
		if (stop_it == bus.stops_of_bus_.end()) { break; }
		Stop to = **stop_it;
		distance += geo::ComputeDistance(from.coordinates, to.coordinates);
	}
	distance *= 2.;
	bi.distance_length_ = distance;

	double real_distance = 0;
	for (size_t i = 0; ; ++i) {
		if (i == bus.stops_of_bus_.size() - 1) {
			break;
		}
		real_distance += transport_catalogue.GetDistanceBetweenStops(bus.stops_of_bus_[i], bus.stops_of_bus_[i + 1]);
	}
	for (size_t i = bus.stops_of_bus_.size() - 1; ; --i) {
		if (i == 0) {
			break;
		}
		real_distance += transport_catalogue.GetDistanceBetweenStops(bus.stops_of_bus_[i], bus.stops_of_bus_[i - 1]);
	}
	bi.real_distance_length_ = real_distance;

	bi.curvature_ = real_distance / distance;

	return bi;
}

BusInfo TransportCatalogue::GetBusInfo(const string& name) {
	//BusInfo bi;
	return bus_infos.at(name);
}

std::unordered_map<std::string_view, std::set<std::string_view>>& TransportCatalogue::GetBTS() {
	return buses_to_stops;
}

std::unordered_map<std::string_view, std::set<std::string_view>>& TransportCatalogue::GetStopsToBuses() {
	return stops_to_buses;
}

void TransportCatalogue::SetDistanceBetweenStops(string stop_name, string next_stop_name, double distance) {
	const Stop* stop = &FindStop(stop_name);
	const Stop* next_stop = &FindStop(next_stop_name);
	stop_pair_to_distance_.insert_or_assign(std::make_pair(stop, next_stop), distance);
}

double TransportCatalogue::GetDistanceBetweenStops(std::string stop_name, std::string next_stop_name) {
	auto stops = std::make_pair(&FindStop(stop_name), &FindStop(next_stop_name));
	double result = 0;
	if (stop_pair_to_distance_.count(stops)) {
		result = stop_pair_to_distance_.at(stops);
	}
	else {
		result = stop_pair_to_distance_.at(std::make_pair(&FindStop(next_stop_name), &FindStop(stop_name)));
	}
	return result;
}

double TransportCatalogue::GetDistanceBetweenStops(const Stop* stop, const Stop* next_stop) {
	auto stops = std::make_pair(stop, next_stop);
	double result = 0;
	if (stop_pair_to_distance_.count(stops)) {
		result = stop_pair_to_distance_.at(stops);
	}
	else {
		if (stop == next_stop) {
			return 0.;
		}
		result = stop_pair_to_distance_.at(std::make_pair(next_stop, stop));
	}
	return result;
}

const std::unordered_set<Stop, StopHasher>& TransportCatalogue::GetStops() const {
	return stops_;
}

const std::unordered_set<Bus, BusHasher>& TransportCatalogue::GetBuses() const {
	return buses_;
}
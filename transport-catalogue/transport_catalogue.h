#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <string_view>

#include "geo.h"

#include "domain.h"

namespace transport_catalogue {


	using std::string;

	namespace objects {


		struct Stop {
			Stop(string name, double latitude, double longitude);

			Stop(string name = "Error");


			string name_;
			geo::Coordinates coordinates;

			bool operator==(const Stop& other) const;

			bool operator<(const Stop& other) const;

		};

		struct Bus {
			Bus(string name, std::vector<const Stop*> stops_of_bus, bool is_looped);

			Bus(string name = "Error");

			string name_;
			std::vector<const Stop*> stops_of_bus_;
			bool is_looped_;

			bool operator==(const Bus& other) const;

			bool operator<(const Bus& other) const;

		};

		struct BusInfo {
			size_t stops_num_;
			int unique_stops_num_;
			double distance_length_;
			double real_distance_length_ = 0;
			double curvature_;
		};

		class StopHasher {
		public:
			size_t operator()(const Stop& stop) const;

		private:
			std::hash<string> hasher_;
		};

		class BusHasher {
		public:
			size_t operator()(const Bus& bus) const;

		private:
			std::hash<string> hasher_;
		};

		class BusPtrHasher {
		public:
			size_t operator()(const Bus* bus) const;

		private:
			std::hash<string> hasher_;
		};

		struct StopsHasher {
			size_t operator()(const std::pair<const Stop*, const Stop*>& two_stops) const;

		private:
			std::hash<const void*> hasher_;

		};

		struct PairStopsHasher {
			size_t operator() (const std::pair<const transport_catalogue::objects::Stop*, const transport_catalogue::objects::Stop*>& stops) const;
		private:
			std::hash<const void*> pair_hasher_;
		};

	}

	using namespace objects;

	class TransportCatalogue {

	public:

		void AddStop(string& name, double latitude, double longitude);

		void AddStopSerialization(std::string_view name, double latitude, double longitude);

		void AddBus(string& name, std::vector<const Stop*>& stops_of_bus, bool is_looped);

		void AddBusSerialization(const std::string_view name, const std::vector<std::string_view>& stops_of_bus, const bool is_looped);

		const Stop& FindStop(const string& name) const;

		const Bus& FindBus(const string& name) const;

		void SetBusInfo(const string& name);

		void SetBusInfo(const Bus& bus);

		void SetDistanceBetweenStops(std::string stop_name, std::string next_stop_name, double distance);

	public:

		BusInfo GetBusInfo(const string& name);

		std::unordered_map<std::string_view, std::set<std::string_view>>& GetBTS();

		std::unordered_map<std::string_view, std::set<std::string_view>>& GetStopsToBuses();

		double GetDistanceBetweenStops(std::string stop_name, std::string next_stop_name) const;

		double GetDistanceBetweenStops(const Stop* stop, const Stop* next_stop) const;

		const std::unordered_set<Stop, StopHasher>& GetStops() const;

		const std::unordered_set<Bus, BusHasher>& GetBuses() const;

		const std::unordered_map<const std::pair<const Stop*, const Stop*>, double, StopsHasher>  GetAllDistances() const;

	private:
		std::unordered_set<Stop, StopHasher> stops_;
		std::unordered_set<Bus, BusHasher> buses_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops;
		std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses;
		std::unordered_map<const std::pair<const Stop*, const Stop*>, double, StopsHasher> stop_pair_to_distance_;
		std::unordered_map<std::string_view, BusInfo> bus_infos;
	};

	namespace SetBusInfo_parts {
		objects::BusInfo& create_looped_bus_info(transport_catalogue::TransportCatalogue& transport_catalogue, objects::BusInfo& bi, const objects::Bus& bus);
		objects::BusInfo& create_turning_bus_info(transport_catalogue::TransportCatalogue& transport_catalogue, objects::BusInfo& bi, const objects::Bus& bus);
	}

}



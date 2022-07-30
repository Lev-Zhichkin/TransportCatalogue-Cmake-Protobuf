#pragma once
#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <utility>
#include <variant>
#include <string>
#include <tuple>
#include <vector>
#include <map>
#include <sstream>

namespace map_renderer {

	inline const double EPSILON = 1e-6;

	class MapRenderer {
	public:
		struct Settings {
			double width = 0;
			double height = 0;
			double padding = 0;
			double stop_radius = 0;
			double line_width = 0;
			int bus_label_font_size = 0;
			svg::Point bus_label_offset = svg::Point(0, 0);
			int stop_label_font_size = 20;
			svg::Point stop_label_offset = svg::Point(0, -0);
			//std::variant<std::string, svg::Rgb, svg::Rgba> underlayer_color = svg::Rgba{ 0, 0, 0, 1 };
			svg::Color underlayer_color = svg::Rgba{ 0, 0, 0, 1 };
			double underlayer_width = 0;
			//std::vector<std::variant<std::string, svg::Rgb, svg::Rgba>> color_palette = { svg::Rgb{ 0, 0, 0 } };
			std::vector<svg::Color> color_palette;
		};

	private:
		struct routes_comparator {
			bool operator() (std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>> a, std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>> b) const {
				return a.first->name_ < b.first->name_;
			}
		};

		struct stops_comparator {
			bool operator() (transport_catalogue::Stop a, transport_catalogue::Stop b) const {
				return a.name_ < b.name_;
			}
		};

	public:
		void RenderMap(transport_catalogue::TransportCatalogue& transport_catalogue, std::ostringstream& ostrm);
		Settings settings;

	private:
		svg::Document doc;

		double min_lng = 0;
		double min_lat = 0;
		double max_lng = 0;
		double max_lat = 0;
		double zoom_coef = 0;

		bool IsZero(double value);

		void DrawLines(std::set<std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>, routes_comparator>& routes);
		void SetLinesParameters(svg::Polyline& route_line, int& color_num);
		void SetLinesCoordinates(std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>& route, svg::Polyline& route_line);

		void DrawBusNames(std::set<std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>, routes_comparator>& routes);
		void SetBusNamesSettings(std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>& route, svg::Text& route_underlayer_name, svg::Text& route_name, int& color_num);
		void SetBusNamesCoordinates(std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>& route, svg::Text& route_underlayer_name, svg::Text& route_name);

		void DrawStops(transport_catalogue::TransportCatalogue& transport_catalogue);

		void DrawStopNames(transport_catalogue::TransportCatalogue& transport_catalogue);
		void SetBusNamesSettings(transport_catalogue::Stop& stop, svg::Text& stop_underlayer_name, svg::Text& stop_name);
		void SetBusNamesCoordinates(transport_catalogue::Stop& stop, svg::Text& stop_underlayer_name, svg::Text& stop_name);

	};

}
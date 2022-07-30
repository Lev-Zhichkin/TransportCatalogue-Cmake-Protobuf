#include "map_renderer.h"

using namespace map_renderer;

void MapRenderer::RenderMap(transport_catalogue::TransportCatalogue& transport_catalogue, std::ostringstream& ostrm) {

	// Вычисляются переменные, которые далее будут использоваться для отрисовки маршрутов
	const std::unordered_set<transport_catalogue::Stop, transport_catalogue::StopHasher>& stops = transport_catalogue.GetStops();
	std::unordered_set<transport_catalogue::Stop, transport_catalogue::StopHasher> unempty_stops;
	for (const transport_catalogue::Stop& stop : stops) {
		if (!transport_catalogue.GetBTS()[stop.name_].empty()) {
			unempty_stops.insert(stop);
		}
	}


	const auto [left_it, right_it]
		= std::minmax_element(unempty_stops.begin(), unempty_stops.end(), [](auto lhs, auto rhs) {
		return lhs.coordinates.lng < rhs.coordinates.lng;
			});
	min_lng = left_it->coordinates.lng;
	max_lng = right_it->coordinates.lng;

	const auto [bottom_it, top_it]
		= std::minmax_element(unempty_stops.begin(), unempty_stops.end(), [](auto lhs, auto rhs) {
		return lhs.coordinates.lat < rhs.coordinates.lat;
			});
	min_lat = bottom_it->coordinates.lat;
	max_lat = top_it->coordinates.lat;

	double width_zoom_coef = 0;
	if (!IsZero(max_lng - min_lng)) {
		width_zoom_coef = (settings.width - 2 * settings.padding) / (max_lng - min_lng);
	}

	double height_zoom_coef = 0;
	if (!IsZero(max_lat - min_lat)) {
		height_zoom_coef = (settings.height - 2 * settings.padding) / (max_lat - min_lat);
	}

	if (width_zoom_coef != 0 && height_zoom_coef != 0) {
		zoom_coef = std::min(width_zoom_coef, height_zoom_coef);
	}
	else if (width_zoom_coef != 0) {
		zoom_coef = width_zoom_coef;
	}
	else if (height_zoom_coef != 0) {
		zoom_coef = height_zoom_coef;
	}

	// Создать и вычислить распределение маршрутов на карте
	std::set<std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>, routes_comparator> routes;
	for (const transport_catalogue::Bus& bus : transport_catalogue.GetBuses()) {
		if (!bus.stops_of_bus_.empty()) {
			const transport_catalogue::Bus* bus_ptr = &bus;
			const std::vector<const transport_catalogue::Stop*>& stops = bus.stops_of_bus_;
			routes.insert(std::pair{ bus_ptr, stops });
		}
	}


	// Отрисовка линий по возрастанию названия маршрута
	DrawLines(routes);
	// Отрисовка названий маршрутов(автобусов) по возрастанию названия маршрута(автобуса)
	DrawBusNames(routes);
	// Отрисовка кругов(остановок) по возрастанию маршрута, без повторений одной и той же остановки, по порядку возрастания названия маршрута(автобуса)
	DrawStops(transport_catalogue);
	// Отрисовка названий остановок в лексикографическом порядке, без повторений одного и того же названия
	DrawStopNames(transport_catalogue);

	doc.Render(ostrm);
}


bool MapRenderer::IsZero(double value) {
	return std::abs(value) < EPSILON;
}

void MapRenderer::DrawLines(std::set<std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>, routes_comparator>& routes) {
	int color_num = 0;
	for (std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>> route : routes) {
		if (color_num == (int)settings.color_palette.size()) { color_num = 0; }
		svg::Polyline route_line;

		MapRenderer::SetLinesParameters(route_line, color_num);
		MapRenderer::SetLinesCoordinates(route, route_line);

		++color_num;
	}
}

void MapRenderer::SetLinesParameters(svg::Polyline& route_line, int& color_num) {
	// Установить параметры линий
	if (std::holds_alternative<std::string>(settings.color_palette[color_num])) {
		route_line.SetStrokeColor(std::get<std::string>(settings.color_palette[color_num]));
	}
	else if (std::holds_alternative<svg::Rgb>(settings.color_palette[color_num])) {
		route_line.SetStrokeColor(std::get<svg::Rgb>(settings.color_palette[color_num]));
	}
	else if (std::holds_alternative<svg::Rgba>(settings.color_palette[color_num])) {
		route_line.SetStrokeColor(std::get<svg::Rgba>(settings.color_palette[color_num]));
	}
	route_line.SetFillColor("none");
	route_line.SetStrokeWidth(settings.line_width);
	route_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	route_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRenderer::SetLinesCoordinates(std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>& route, svg::Polyline& route_line) {
	// Отрисовать кольцевой маршрут 
	if (route.first->is_looped_) {
		for (const transport_catalogue::Stop* stop : route.second) {
			double x = (stop->coordinates.lng - min_lng) * zoom_coef + settings.padding;
			double y = (max_lat - stop->coordinates.lat) * zoom_coef + settings.padding;
			route_line.AddPoint(svg::Point(x, y));
		}
		doc.Add(route_line);
	}
	// Отрисовать некольцевой маршрут
	else if (!route.first->is_looped_) {
		for (std::vector<const transport_catalogue::Stop*>::iterator stop_it = route.second.begin(); stop_it != route.second.end(); ++stop_it) {
			const transport_catalogue::Stop* stop = *stop_it;
			double x = (stop->coordinates.lng - min_lng) * zoom_coef + settings.padding;
			double y = (max_lat - stop->coordinates.lat) * zoom_coef + settings.padding;
			route_line.AddPoint(svg::Point(x, y));
		}
		for (std::vector<const transport_catalogue::Stop*>::reverse_iterator stop_it = ++route.second.rbegin(); stop_it != route.second.rend(); ++stop_it) {
			const transport_catalogue::Stop* stop = *stop_it;
			double x = (stop->coordinates.lng - min_lng) * zoom_coef + settings.padding;
			double y = (max_lat - stop->coordinates.lat) * zoom_coef + settings.padding;
			route_line.AddPoint(svg::Point(x, y));
		}
		doc.Add(route_line);
	}
}

void MapRenderer::DrawBusNames(std::set<std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>, routes_comparator>& routes) {
	int color_num = 0;
	for (std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>> route : routes) {
		if (color_num == (int)settings.color_palette.size()) { color_num = 0; }
		svg::Text route_underlayer_name;
		svg::Text route_name;

		MapRenderer::SetBusNamesSettings(route, route_underlayer_name, route_name, color_num);
		MapRenderer::SetBusNamesCoordinates(route, route_underlayer_name, route_name);

		++color_num;
	}
}

void MapRenderer::SetBusNamesSettings(std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>& route, svg::Text& route_underlayer_name, svg::Text& route_name, int& color_num) {
	// Установить содержимое подложки и названия
	route_underlayer_name.SetData(route.first->name_);
	route_name.SetData(route.first->name_);
	// Установить цвет подложки
	if (std::holds_alternative<std::string>(settings.underlayer_color)) {
		route_underlayer_name.SetFillColor(std::get<std::string>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgb>(settings.underlayer_color)) {
		route_underlayer_name.SetFillColor(std::get<svg::Rgb>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgba>(settings.underlayer_color)) {
		route_underlayer_name.SetFillColor(std::get<svg::Rgba>(settings.underlayer_color));
	}
	if (std::holds_alternative<std::string>(settings.underlayer_color)) {
		route_underlayer_name.SetStrokeColor(std::get<std::string>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgb>(settings.underlayer_color)) {
		route_underlayer_name.SetStrokeColor(std::get<svg::Rgb>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgba>(settings.underlayer_color)) {
		route_underlayer_name.SetStrokeColor(std::get<svg::Rgba>(settings.underlayer_color));
	}
	// Установить цвет названия
	if (std::holds_alternative<std::string>(settings.color_palette[color_num])) {
		route_name.SetFillColor(std::get<std::string>(settings.color_palette[color_num]));
	}
	else if (std::holds_alternative<svg::Rgb>(settings.color_palette[color_num])) {
		route_name.SetFillColor(std::get<svg::Rgb>(settings.color_palette[color_num]));
	}
	else if (std::holds_alternative<svg::Rgba>(settings.color_palette[color_num])) {
		route_name.SetFillColor(std::get<svg::Rgba>(settings.color_palette[color_num]));
	}
	// Установить остальные параметры подложки
	route_underlayer_name.SetStrokeWidth(settings.underlayer_width);
	route_underlayer_name.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	route_underlayer_name.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	// Установить общие для обоих объектов параметры
	route_underlayer_name.SetFontSize(settings.bus_label_font_size);
	route_name.SetFontSize(settings.bus_label_font_size);
	route_underlayer_name.SetFontFamily("Verdana");
	route_name.SetFontFamily("Verdana");
	route_underlayer_name.SetFontWeight("bold");
	route_name.SetFontWeight("bold");
}

void MapRenderer::SetBusNamesCoordinates(std::pair<const transport_catalogue::Bus*, std::vector<const transport_catalogue::Stop*>>& route, svg::Text& route_underlayer_name, svg::Text& route_name) {
	// Установить расположение подложки и названия 
	if (route.first->is_looped_) {
		const transport_catalogue::Stop* first = *route.second.begin();
		double x = (first->coordinates.lng - min_lng) * zoom_coef + settings.padding;
		double y = (max_lat - first->coordinates.lat) * zoom_coef + settings.padding;
		route_underlayer_name.SetPosition(svg::Point(x, y));
		route_underlayer_name.SetOffset(settings.bus_label_offset);
		doc.Add(route_underlayer_name);
		route_name.SetPosition(svg::Point(x, y));
		route_name.SetOffset(settings.bus_label_offset);
		doc.Add(route_name);
	}
	else if (!route.first->is_looped_) {
		const transport_catalogue::Stop* first = *route.second.begin();
		double x = (first->coordinates.lng - min_lng) * zoom_coef + settings.padding;
		double y = (max_lat - first->coordinates.lat) * zoom_coef + settings.padding;
		route_underlayer_name.SetPosition(svg::Point(x, y));
		route_underlayer_name.SetOffset(settings.bus_label_offset);
		doc.Add(route_underlayer_name);
		route_name.SetPosition(svg::Point(x, y));
		route_name.SetOffset(settings.bus_label_offset);
		doc.Add(route_name);

		//svg::Text route_underlayer_name_last = svg::Text(route_underlayer_name);
		//svg::Text route_name_last = svg::Text(route_name);
		const transport_catalogue::Stop* last = *--route.second.end();
		if (first != last) {
			x = (last->coordinates.lng - min_lng) * zoom_coef + settings.padding;
			y = (max_lat - last->coordinates.lat) * zoom_coef + settings.padding;
			route_underlayer_name.SetPosition(svg::Point(x, y));
			route_underlayer_name.SetOffset(settings.bus_label_offset);
			doc.Add(route_underlayer_name);
			route_name.SetPosition(svg::Point(x, y));
			route_name.SetOffset(settings.bus_label_offset);
			doc.Add(route_name);
		}
	}
}

void MapRenderer::DrawStops(transport_catalogue::TransportCatalogue& transport_catalogue) {
	// Чтобы не допустить повторной отрисовки остановки придётся итерироваться через buses_to_stops, поэтому routes не нужен - buses_to_stops заменяет его
	std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops = transport_catalogue.GetBTS();

	const std::unordered_set<transport_catalogue::Stop, transport_catalogue::StopHasher>& stops = transport_catalogue.GetStops();
	std::set<transport_catalogue::Stop, stops_comparator> sorted_stops(stops.begin(), stops.end());
	for (transport_catalogue::Stop stop : sorted_stops) {
		if (!buses_to_stops[stop.name_].empty()) {
			double x = (stop.coordinates.lng - min_lng) * zoom_coef + settings.padding;
			double y = (max_lat - stop.coordinates.lat) * zoom_coef + settings.padding;
			doc.Add(svg::Circle().SetCenter(svg::Point(x, y)).SetRadius(settings.stop_radius).SetFillColor("white"));
		}
	}
}

void MapRenderer::DrawStopNames(transport_catalogue::TransportCatalogue& transport_catalogue) {
	// Чтобы не допустить повторной отрисовки остановки придётся итерироваться через buses_to_stops, поэтому routes не нужен - buses_to_stops заменяет его
	std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops = transport_catalogue.GetBTS();

	const std::unordered_set<transport_catalogue::Stop, transport_catalogue::StopHasher>& stops = transport_catalogue.GetStops();
	std::set<transport_catalogue::Stop, stops_comparator> sorted_stops(stops.begin(), stops.end());

	for (transport_catalogue::Stop stop : sorted_stops) {
		if (!buses_to_stops[stop.name_].empty()) {
			svg::Text stop_underlayer_name;
			svg::Text stop_name;

			MapRenderer::SetBusNamesSettings(stop, stop_underlayer_name, stop_name);
			MapRenderer::SetBusNamesCoordinates(stop, stop_underlayer_name, stop_name);

		}
	}
}

void MapRenderer::SetBusNamesSettings(transport_catalogue::Stop& stop, svg::Text& stop_underlayer_name, svg::Text& stop_name) {
	// Установить содержимое подложки и названия
	stop_underlayer_name.SetData(stop.name_);
	stop_name.SetData(stop.name_);
	// Установить цвет названия
	stop_name.SetFillColor("black");
	// Установить цвет подложки
	if (std::holds_alternative<std::string>(settings.underlayer_color)) {
		stop_underlayer_name.SetFillColor(std::get<std::string>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgb>(settings.underlayer_color)) {
		stop_underlayer_name.SetFillColor(std::get<svg::Rgb>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgba>(settings.underlayer_color)) {
		stop_underlayer_name.SetFillColor(std::get<svg::Rgba>(settings.underlayer_color));
	}
	if (std::holds_alternative<std::string>(settings.underlayer_color)) {
		stop_underlayer_name.SetStrokeColor(std::get<std::string>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgb>(settings.underlayer_color)) {
		stop_underlayer_name.SetStrokeColor(std::get<svg::Rgb>(settings.underlayer_color));
	}
	else if (std::holds_alternative<svg::Rgba>(settings.underlayer_color)) {
		stop_underlayer_name.SetStrokeColor(std::get<svg::Rgba>(settings.underlayer_color));
	}
	// Установить остальные параметры подложки
	stop_underlayer_name.SetStrokeWidth(settings.underlayer_width);
	stop_underlayer_name.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	stop_underlayer_name.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	// Установить общие для обоих объектов параметры
	stop_underlayer_name.SetFontSize(settings.stop_label_font_size);
	stop_name.SetFontSize(settings.stop_label_font_size);
	stop_underlayer_name.SetFontFamily("Verdana");
	stop_name.SetFontFamily("Verdana");
	stop_underlayer_name.SetFontWeight("");
	stop_name.SetFontWeight("");
}

void MapRenderer::SetBusNamesCoordinates(transport_catalogue::Stop& stop, svg::Text& stop_underlayer_name, svg::Text& stop_name) {
	// Установить расположение подложки и названия 
	double x = (stop.coordinates.lng - min_lng) * zoom_coef + settings.padding;
	double y = (max_lat - stop.coordinates.lat) * zoom_coef + settings.padding;
	stop_underlayer_name.SetPosition(svg::Point(x, y));
	stop_underlayer_name.SetOffset(settings.stop_label_offset);
	doc.Add(stop_underlayer_name);
	stop_name.SetPosition(svg::Point(x, y));
	stop_name.SetOffset(settings.stop_label_offset);
	doc.Add(stop_name);
}
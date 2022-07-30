#include "serialization.h"

namespace serialize
{
	// Serialize

	void Serializer::Serialize(const std::string& filename)
	{
		std::ofstream out(filename, std::ios::binary);
		SerializeStop(trans_cat_ser_);
		SerializeDistance(trans_cat_ser_);
		SerializeBus(trans_cat_ser_);
		SerealizeRenderSettings(trans_cat_ser_);
		SerealizeRoutingSettings(trans_cat_ser_);
		trans_cat_ser_.SerializeToOstream(&out);
	}

	// Serialize private

	void Serializer::SerializeStop(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser)
	{
		for (const auto& stop : trans_cat_.GetStops())
		{
			transport_catalogue_serialize::Stop buf_stop;
			transport_catalogue_serialize::Coordinates buf_coordinates;
			buf_coordinates.set_lat(stop.coordinates.lat);
			buf_coordinates.set_lng(stop.coordinates.lng);
			buf_stop.set_name(stop.name_);
			*buf_stop.mutable_coordinates() = buf_coordinates;
			*trans_cat_ser.add_stops() = buf_stop;
		}
	}

	void Serializer::SerializeBus(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser)
	{
		for (const auto& bus : trans_cat_.GetBuses())
		{
			transport_catalogue_serialize::Bus buf_bus;
			buf_bus.set_bus_num(bus.name_);
			buf_bus.set_is_circular(bus.is_looped_);
			for (auto stop : bus.stops_of_bus_)
			{
				transport_catalogue_serialize::Stop buf_stop;
				transport_catalogue_serialize::Coordinates buf_coordinates;
				buf_coordinates.set_lat(stop->coordinates.lat);
				buf_coordinates.set_lng(stop->coordinates.lng);
				buf_stop.set_name(stop->name_);
				*buf_stop.mutable_coordinates() = buf_coordinates;
				*buf_bus.add_stops() = buf_stop;
			}
			*trans_cat_ser.add_buses() = buf_bus;
		}
	}

	void Serializer::SerializeDistance(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser)
	{
		for (const auto& distance : trans_cat_.GetAllDistances())
		{
			transport_catalogue_serialize::Distance buf_distance;
			buf_distance.set_from(distance.first.first->name_);
			buf_distance.set_to(distance.first.second->name_);
			buf_distance.set_distance(distance.second);
			*trans_cat_ser.add_distances() = buf_distance;
		}
	}
	
	void Serializer::SerealizeRenderSettings(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser)
	{
		//map_renderer_.settings;
		transport_catalogue_serialize::RenderSettings buf_render_settings;
		transport_catalogue_serialize::Point buf_point;
		buf_render_settings.set_width(map_renderer_.settings.width);
		buf_render_settings.set_height(map_renderer_.settings.height);
		buf_render_settings.set_padding(map_renderer_.settings.padding);
		buf_render_settings.set_line_width(map_renderer_.settings.line_width);
		buf_render_settings.set_stop_radius(map_renderer_.settings.stop_radius);
		buf_render_settings.set_bus_label_font_size(map_renderer_.settings.bus_label_font_size);
		buf_render_settings.set_stop_label_font_size(map_renderer_.settings.stop_label_font_size);
		buf_render_settings.set_underlayer_width(map_renderer_.settings.underlayer_width);

		buf_point.set_x(map_renderer_.settings.bus_label_offset.x);
		buf_point.set_y(map_renderer_.settings.bus_label_offset.y);
		*buf_render_settings.mutable_bus_label_offset() = buf_point;

		buf_point.set_x(map_renderer_.settings.stop_label_offset.x);
		buf_point.set_y(map_renderer_.settings.stop_label_offset.y);
		*buf_render_settings.mutable_stop_label_offset() = buf_point;

		*buf_render_settings.mutable_underlayer_color() = SerealizeColor(map_renderer_.settings.underlayer_color);

		for (const auto& color : map_renderer_.settings.color_palette)
		{
			*buf_render_settings.add_color_palette() = SerealizeColor(color);
		}

		*trans_cat_ser.mutable_render_settings() = buf_render_settings;
	}

	transport_catalogue_serialize::Color Serializer::SerealizeColor(const svg::Color& color)
	{
		transport_catalogue_serialize::Color buf_color;
		transport_catalogue_serialize::Rgba buf_rgba;

		if (std::holds_alternative<svg::Rgb>(color))
		{
			svg::Rgb rgb = std::get<svg::Rgb>(color);
			buf_rgba.set_blue(rgb.blue);
			buf_rgba.set_green(rgb.green);
			buf_rgba.set_red(rgb.red);
			*buf_color.mutable_rgba() = buf_rgba;
		}
		else if (std::holds_alternative<svg::Rgba>(color))
		{
			svg::Rgba rgba = std::get<svg::Rgba>(color);
			buf_rgba.set_blue(rgba.blue);
			buf_rgba.set_green(rgba.green);
			buf_rgba.set_red(rgba.red);
			buf_rgba.set_opacity(rgba.opacity);
			buf_color.set_is_rgba(true);
			*buf_color.mutable_rgba() = buf_rgba;
		}
		else if (std::holds_alternative<std::string>(color))
		{
			//buf_color.set_name(std::get<std::string>(color));
			std::string name = std::get<std::string>(color);
			*buf_color.mutable_name() = name;
		}
		return buf_color;
	}

	void Serializer::SerealizeRoutingSettings(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser)
	{
		transport_catalogue_serialize::RoutingSettings buf_routing_settings;
		auto routing_settings = trans_rote_.GetRouterSettings();
		buf_routing_settings.set_bus_velocity(routing_settings.bus_velocity_);
		buf_routing_settings.set_bus_wait_time(routing_settings.bus_wait_time_);
		*trans_cat_ser.mutable_routing_settings() = buf_routing_settings;
	}

	//Deserializer

	void Deserializer::DeserializeToRequestHandler(std::istream& input, std::ostream& output) {
		transport_catalogue::TransportCatalogue trans_cat;
		map_renderer::MapRenderer map_renderer;
		const auto dict = json::Load(input).GetRoot().AsMap();
		const auto serialization_settings = dict.find("serialization_settings");

		DeserializeCatalogAndSetMapSettings(trans_cat, map_renderer, serialization_settings->second.AsMap().at("file").AsString());

		for (const transport_catalogue::Bus& bus : trans_cat.GetBuses()) {
			trans_cat.SetBusInfo(bus);
		}

		transport_router::RouterSettings settings = DeserealizeRouterSettings(serialization_settings->second.AsMap().at("file").AsString());
		const auto stat_requests = dict.find("stat_requests");
		if (stat_requests != dict.end())
		{
			request_handler::RequestHandler request_handler(trans_cat, map_renderer, settings);
			request_handler.ProcessRequest(const_cast<json::Array&>(stat_requests->second.AsArray()), output);
		}
	}

	void Deserializer::DeserializeCatalogAndSetMapSettings(transport_catalogue::TransportCatalogue& trans_cat, map_renderer::MapRenderer& map_renderer, const std::string& filename)
	{
		std::ifstream in(filename, std::ios::binary);
		transport_catalogue_serialize::TransportCatalogue trans_cat_ser;
		trans_cat_ser.ParseFromIstream(&in);
		// trans_cat
		for (int i = 0; i < trans_cat_ser.stops().size(); ++i)
		{
			auto stop_ser = trans_cat_ser.stops(i);
			trans_cat.AddStopSerialization(stop_ser.name(), stop_ser.coordinates().lat(), stop_ser.coordinates().lng());
		}
		for (int i = 0; i < trans_cat_ser.buses().size(); ++i)
		{
			auto bus_ser = trans_cat_ser.buses(i);
			std::vector<std::string_view> stops;
			for (const auto& stop_ser : bus_ser.stops())
			{
				stops.push_back(stop_ser.name());
			}
			trans_cat.AddBusSerialization(bus_ser.bus_num(), stops, bus_ser.is_circular());
		}
		for (int i = 0; i < trans_cat_ser.distances_size(); ++i)
		{
			const auto deserialized_distance = DeserializeDistance(trans_cat_ser.distances(i), trans_cat);
			trans_cat.SetDistanceBetweenStops(deserialized_distance.first.first->name_, deserialized_distance.first.second->name_, deserialized_distance.second);
		}

		// map_renderer
		auto settings(DeserealizeRenderSettings(trans_cat_ser.render_settings()));
		map_renderer.settings = settings;
	}

	transport_router::RouterSettings Deserializer::DeserealizeRouterSettings(const std::string& filename)
	{
		std::ifstream in(filename, std::ios::binary);
		transport_catalogue_serialize::TransportCatalogue trans_cat_ser;
		trans_cat_ser.ParseFromIstream(&in);
		transport_router::RouterSettings router_settings;
		auto routing_settings_ser = trans_cat_ser.routing_settings();
		router_settings.bus_wait_time_ = routing_settings_ser.bus_wait_time();
		router_settings.bus_velocity_ = routing_settings_ser.bus_velocity();

		return router_settings;
	}

	//Deserializer private

	std::pair<std::pair<const transport_catalogue::Stop*, const transport_catalogue::Stop*>, int> Deserializer::DeserializeDistance(const transport_catalogue_serialize::Distance& distance_ser, const transport_catalogue::TransportCatalogue& trans_cat)
	{
		const transport_catalogue::Stop* stop_from = &trans_cat.FindStop(distance_ser.from());
		const transport_catalogue::Stop* stop_to = &trans_cat.FindStop(distance_ser.to());
		int64_t distance = distance_ser.distance();
		return std::make_pair(std::make_pair(stop_from, stop_to), distance);
	}

	map_renderer::MapRenderer::Settings Deserializer::DeserealizeRenderSettings(const transport_catalogue_serialize::RenderSettings& render_settings_ser)
	{
		map_renderer::MapRenderer::Settings render_settings;

		render_settings.width = render_settings_ser.width();
		render_settings.height = render_settings_ser.height();
		render_settings.padding = render_settings_ser.padding();
		render_settings.line_width = render_settings_ser.line_width();
		render_settings.stop_radius = render_settings_ser.stop_radius();
		render_settings.bus_label_font_size = render_settings_ser.bus_label_font_size();
		render_settings.stop_label_font_size = render_settings_ser.stop_label_font_size();
		render_settings.underlayer_width = render_settings_ser.underlayer_width();

		render_settings.bus_label_offset = { render_settings_ser.bus_label_offset().x(), render_settings_ser.bus_label_offset().y() };
		render_settings.stop_label_offset = { render_settings_ser.stop_label_offset().x(), render_settings_ser.stop_label_offset().y() };

		render_settings.underlayer_color = DserealizeColor(render_settings_ser.underlayer_color());

		for (const auto& color_ser : render_settings_ser.color_palette())
		{
			render_settings.color_palette.push_back(DserealizeColor(color_ser));
		}

		return render_settings;
	}

	svg::Color Deserializer::DserealizeColor(const transport_catalogue_serialize::Color& color_ser)
	{
		if (!color_ser.name().empty())
		{
			return color_ser.name();
		}
		else if (color_ser.is_rgba())
		{
			return svg::Rgba(color_ser.rgba().red(), color_ser.rgba().green(), color_ser.rgba().blue(), color_ser.rgba().opacity());
		}
		return svg::Rgb(color_ser.rgba().red(), color_ser.rgba().green(), color_ser.rgba().blue());
	}
} // namespace serialize
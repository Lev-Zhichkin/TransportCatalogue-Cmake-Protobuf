#include "json_reader.h"


using namespace transport_catalogue;
using namespace map_renderer;
using namespace json;
using namespace std::literals;
using namespace request_handler;
using namespace json_reader;

JsonReader::JsonReader(transport_catalogue::TransportCatalogue& catalog, map_renderer::MapRenderer& map_renderer) 
: catalog_(catalog), map_renderer_(map_renderer)
{
}

void JsonReader::Reader(std::istream& input, std::ostream& output) {
    //Timer timer("READER");
    Dict root = Load(input).GetRoot().AsMap();
    Array content_base = root.at("base_requests"s).AsArray();
    Dict render_info = root.at("render_settings").AsMap();
    Array content_state = root.at("stat_requests"s).AsArray();

    CompleteCatalog(content_base);
    Render(render_info);

    RequestHandler request_handler(catalog_, map_renderer_);
    request_handler.ProcessRequest(content_state, output);
}

////////// base_requests //////////
void JsonReader::CompleteCatalog(Array& value) {
    //Timer timer("COMPLETING CATALOG");

    std::vector<json::Node> bus_descriptions;
    std::map<std::string, Dict> road_distances;
    std::vector<std::string> bus_names;
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            AddStop(description.AsMap());
            road_distances[description.AsMap().at("name"s).AsString()] = description.AsMap().at("road_distances"s).AsMap();
        }
        // сохранить описания автобусов и отдельно - их имена
        else if (description.AsMap().at("type"s) == "Bus"s) {
            bus_descriptions.push_back(description.AsMap());
            bus_names.push_back(description.AsMap().at("name"s).AsString());
        }
    }
    // добавить автобусы, после того, как были добавлены все остановки
    for (Node& description : bus_descriptions) {
        AddBus(description.AsMap());
    }
    // теперь очередь добавления дистанций между остановками
    for (std::pair<const std::string, Dict>& distance : road_distances) {
        for (std::pair<std::string, Node> d : distance.second) {
            catalog_.SetDistanceBetweenStops(distance.first, d.first, d.second.AsInt());
        }
    }
    // и теперь последнее - создать и сохранить BusInfo для каждого автобуса
    for (std::string bus_name : bus_names) {
        catalog_.SetBusInfo(bus_name);
    }

}

void JsonReader::AddStop(const Dict& stop) {
    std::string name;
    double latitude;
    double longitude;

    name = stop.at("name"s).AsString();
    latitude = stop.at("latitude"s).AsDouble();
    longitude = stop.at("longitude"s).AsDouble();

    catalog_.AddStop(name, latitude, longitude);
}

void JsonReader::AddBus(const Dict& bus) {
    std::string name;
    Array stope;
    bool is_roundtrip;

    name = bus.at("name"s).AsString();

    stope = bus.at("stops"s).AsArray();
    std::vector<const Stop*> stops;
    for (auto& kek : stope) {
        stops.push_back(&catalog_.FindStop(kek.AsString()));
    }

    bus.at("is_roundtrip"s).AsBool() ? is_roundtrip = true : is_roundtrip = false;

    catalog_.AddBus(name, stops, is_roundtrip);
}
//-------- base_requests //--------

void JsonReader::Render(const Dict& value) {

    RenderSimpleSettings(value);

    RenderArraySettings(value);

    RenderVariantSettings(value);

}

void JsonReader::RenderSimpleSettings(const Dict& value) {
    map_renderer_.settings.width = value.at("width").AsDouble();
    map_renderer_.settings.height = value.at("height").AsDouble();
    map_renderer_.settings.padding = value.at("padding").AsDouble();
    map_renderer_.settings.stop_radius = value.at("stop_radius").AsDouble();
    map_renderer_.settings.line_width = value.at("line_width").AsDouble();
    map_renderer_.settings.bus_label_font_size = value.at("bus_label_font_size").AsInt();
    map_renderer_.settings.stop_label_font_size = value.at("stop_label_font_size").AsInt();
    map_renderer_.settings.underlayer_width = value.at("underlayer_width").AsDouble();
}

void JsonReader::RenderArraySettings(const Dict& value) {
    // Bus label offset
    Array bus_label_offset_arr = value.at("bus_label_offset").AsArray();
    svg::Point bus_label_offset;
    {
        std::pair<double, double> to_point;
        bool b = false;
        for (Node num : bus_label_offset_arr) {
            if (!b) { to_point.first = num.AsDouble(); }
            else { to_point.second = num.AsDouble(); }
            b = true;
        }
        bus_label_offset.x = to_point.first;
        bus_label_offset.y = to_point.second;
    }
    map_renderer_.settings.bus_label_offset = bus_label_offset;

    // Stop label offset
    Array stop_label_offset_arr = value.at("stop_label_offset").AsArray();
    svg::Point stop_label_offset;
    {
        std::pair<double, double> to_point;
        bool b = false;
        for (Node num : stop_label_offset_arr) {
            if (!b) { to_point.first = num.AsDouble(); }
            else { to_point.second = num.AsDouble(); }
            b = true;
        }
        stop_label_offset.x = to_point.first;
        stop_label_offset.y = to_point.second;
    }
    map_renderer_.settings.stop_label_offset = stop_label_offset;
}

void JsonReader::RenderVariantSettings(const Dict& value) {
    // Underlayer color
    if (value.at("underlayer_color").IsString()) {
        std::string underlayer_color = value.at("underlayer_color").AsString();
        map_renderer_.settings.underlayer_color = underlayer_color;
    }
    else if (value.at("underlayer_color").IsArray()) {
        Array underlayer_color = value.at("underlayer_color").AsArray();
        if (underlayer_color.size() == 3) {
            std::vector<int> to_rgb;
            for (Node node : underlayer_color) {
                to_rgb.push_back(node.AsInt());
            }
            svg::Rgb rgb(to_rgb[0], to_rgb[1], to_rgb[2]);
            map_renderer_.settings.underlayer_color = rgb;
        }
        else if (underlayer_color.size() == 4) {
            std::vector<int> to_rgb;
            double to_rgba;
            for (Node node : underlayer_color) {
                if (node.IsPureDouble()) {
                    to_rgba = node.AsDouble();
                }
                else {
                    to_rgb.push_back(node.AsInt());
                }
            }
            svg::Rgba rgba(to_rgb[0], to_rgb[1], to_rgb[2], to_rgba);
            map_renderer_.settings.underlayer_color = rgba;
        }
    }

    // Color palette
    std::vector<std::variant<std::string, svg::Rgb, svg::Rgba>> color_palette;
    Array color_palette_arr = value.at("color_palette").AsArray();
    for (Node node : color_palette_arr) {
        if (node.IsString()) {
            std::string color_string = node.AsString();
            color_palette.push_back(color_string);
        }
        else if (node.IsArray()) {
            Array color_arr = node.AsArray();
            if (color_arr.size() == 3) {
                std::vector<int> to_rgb;
                for (Node node : color_arr) {
                    to_rgb.push_back(node.AsInt());
                }
                svg::Rgb rgb(to_rgb[0], to_rgb[1], to_rgb[2]);
                color_palette.push_back(rgb);
            }
            else if (color_arr.size() == 4) {
                std::vector<int> to_rgb;
                double to_rgba;
                for (Node node : color_arr) {
                    if (node.IsPureDouble()) {
                        to_rgba = node.AsDouble();
                    }
                    else {
                        to_rgb.push_back(node.AsInt());
                    }
                }
                svg::Rgba rgba(to_rgb[0], to_rgb[1], to_rgb[2], to_rgba);
                color_palette.push_back(rgba);
            }
        }
    }
    map_renderer_.settings.color_palette = color_palette;
}
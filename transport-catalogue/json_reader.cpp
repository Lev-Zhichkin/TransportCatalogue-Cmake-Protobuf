#include "reguest_handler.h"
#include "json_reader.h"


using namespace transport_catalogue;
using namespace json;
using namespace std::literals;
using namespace reguest_handler;

void json_reader::Reader(TransportCatalogue& catalog, MapRenderer& map_renderer, std::istream& input, std::ostream& output) {
    //Timer timer("READER");
    Dict root = Load(input).GetRoot().AsMap();
    Array content_base = root.at("base_requests"s).AsArray();
    Dict render_info = root.at("render_settings").AsMap();
    Array content_state = root.at("stat_requests"s).AsArray();

    CompleteCatalog(catalog, content_base);
    Render(map_renderer, render_info);
    ProcessRequest(catalog, map_renderer, content_state, output);
}

////////// base_requests //////////
void json_reader::CompleteCatalog(TransportCatalogue& catalog, Array& value) {
    //Timer timer("COMPLETING CATALOG");

    std::vector<json::Node> bus_descriptions;
    std::map<std::string, Dict> road_distances;
    std::vector<std::string> bus_names;
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            AddStop(catalog, description.AsMap());
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
        AddBus(catalog, description.AsMap());
    }
    // теперь очередь добавления дистанций между остановками
    for (std::pair<const std::string, Dict>& distance : road_distances) {
        for (std::pair<std::string, Node> d : distance.second) {
            catalog.SetDistanceBetweenStops(distance.first, d.first, d.second.AsInt());
        }
    }
    // и теперь последнее - создать и сохранить BusInfo для каждого автобуса
    for (std::string bus_name : bus_names) {
        catalog.SetBusInfo(bus_name);
    }

}

void json_reader::AddStop(TransportCatalogue& catalog, const Dict& stop) {
    std::string name;
    double latitude;
    double longitude;

    name = stop.at("name"s).AsString();
    latitude = stop.at("latitude"s).AsDouble();
    longitude = stop.at("longitude"s).AsDouble();

    catalog.AddStop(name, latitude, longitude);
}

void json_reader::AddBus(TransportCatalogue& catalog, const Dict& bus) {
    std::string name;
    Array stope;
    bool is_roundtrip;

    name = bus.at("name"s).AsString();

    stope = bus.at("stops"s).AsArray();
    std::vector<const Stop*> stops;
    for (auto& kek : stope) {
        stops.push_back(&catalog.FindStop(kek.AsString()));
    }

    bus.at("is_roundtrip"s).AsBool() ? is_roundtrip = true : is_roundtrip = false;

    catalog.AddBus(name, stops, is_roundtrip);
}
//-------- base_requests //--------

////////// stat_requests //////////
void json_reader::ProcessRequest(TransportCatalogue& catalog, MapRenderer& map_renderer, Array& value, std::ostream& output) {
    //Timer timer("PROCESSING REGUEST");

    std::cout << "[" << std::endl;

    bool f = false;
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            reguest_handler::PrintStopInformation(catalog, description.AsMap(), output);
        }
        else if (description.AsMap().at("type"s) == "Bus"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            reguest_handler::PrintBusInformation(catalog, description.AsMap(), output);
        }
        else if (description.AsMap().at("type"s) == "Map"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            reguest_handler::PrintMap(catalog, map_renderer, description.AsMap(), output);
        }
        f = true;
    }

    std::cout << "]" << std::endl;

}

void json_reader::Render(MapRenderer& map_renderer, const Dict& value) {

    RenderSimpleSettings(map_renderer, value);

    RenderArraySettings(map_renderer, value);

    RenderVariantSettings(map_renderer, value);

}

void json_reader::RenderSimpleSettings(MapRenderer& map_renderer, const Dict& value) {
    map_renderer.settings.width = value.at("width").AsDouble();
    map_renderer.settings.height = value.at("height").AsDouble();
    map_renderer.settings.padding = value.at("padding").AsDouble();
    map_renderer.settings.stop_radius = value.at("stop_radius").AsDouble();
    map_renderer.settings.line_width = value.at("line_width").AsDouble();
    map_renderer.settings.bus_label_font_size = value.at("bus_label_font_size").AsInt();
    map_renderer.settings.stop_label_font_size = value.at("stop_label_font_size").AsInt();
    map_renderer.settings.underlayer_width = value.at("underlayer_width").AsDouble();
}

void json_reader::RenderArraySettings(MapRenderer& map_renderer, const Dict& value) {
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
    map_renderer.settings.bus_label_offset = bus_label_offset;

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
    map_renderer.settings.stop_label_offset = stop_label_offset;
}

void json_reader::RenderVariantSettings(MapRenderer& map_renderer, const Dict& value) {
    // Underlayer color
    if (value.at("underlayer_color").IsString()) {
        std::string underlayer_color = value.at("underlayer_color").AsString();
        map_renderer.settings.underlayer_color = underlayer_color;
    }
    else if (value.at("underlayer_color").IsArray()) {
        Array underlayer_color = value.at("underlayer_color").AsArray();
        if (underlayer_color.size() == 3) {
            std::vector<int> to_rgb;
            for (Node node : underlayer_color) {
                to_rgb.push_back(node.AsInt());
            }
            svg::Rgb rgb(to_rgb[0], to_rgb[1], to_rgb[2]);
            map_renderer.settings.underlayer_color = rgb;
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
            map_renderer.settings.underlayer_color = rgba;
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
    map_renderer.settings.color_palette = color_palette;
}
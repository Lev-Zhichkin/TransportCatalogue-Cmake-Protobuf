#include "reguest_handler.h"


using namespace transport_catalogue;
using namespace json;
using namespace std::literals;
using namespace reguest_handler;

/*void Reader(TransportCatalogue& catalog, MapRenderer& map_renderer, std::istream& input, std::ostream& output) {
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
void CompleteCatalog(TransportCatalogue& catalog, Array& value) {
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

void AddStop(TransportCatalogue& catalog, const Dict& stop) {
    std::string name;
    double latitude;
    double longitude;

    name = stop.at("name"s).AsString();
    latitude = stop.at("latitude"s).AsDouble();
    longitude = stop.at("longitude"s).AsDouble();

    catalog.AddStop(name, latitude, longitude);
}

void AddBus(TransportCatalogue& catalog, const Dict& bus) {
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
void ProcessRequest(TransportCatalogue& catalog, MapRenderer& map_renderer, Array& value, std::ostream& output) {
    //Timer timer("PROCESSING REGUEST");

    std::cout << "[" << std::endl;

    bool f = false;
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            PrintStopInformation(catalog, description.AsMap(), output);
        }
        else if (description.AsMap().at("type"s) == "Bus"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            PrintBusInformation(catalog, description.AsMap(), output);
        }
        else if (description.AsMap().at("type"s) == "Map"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            PrintMap(catalog, map_renderer, description.AsMap(), output);
        }
        f = true;
    }

    std::cout << "]" << std::endl;

} */

void reguest_handler::PrintStopInformation(TransportCatalogue& catalog, const Dict& value, std::ostream& output) {
    //Timer timer("STOP INFORMATION PRINTING");

    int id;
    std::string name;

    id = value.at("id"s).AsInt();
    name = value.at("name"s).AsString();
    Array bus_names;

    Stop stop = catalog.FindStop(name);

    //тут как бы для меня самое не понятное - как вывести инфу в консоль используя json
    if (stop.name_ == "Error"s) {
        //типа вывести это
        Print(json::Document(json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build()), output);
    }
    else {
        std::set<std::string_view> buses = catalog.GetBTS().at(name);
        for (auto& bus : buses) {
            bus_names.push_back(std::string(bus));
        }

        //и вывести это
        Print(json::Document(json::Builder{}.StartDict().Key("buses"s).Value(bus_names).Key("request_id"s).Value(id).EndDict().Build()), output);
    }
}

void reguest_handler::PrintBusInformation(TransportCatalogue& catalog, const Dict& value, std::ostream& output) {
    //Timer timer("BUS INFORMATION PRINTING");

    std::string name;
    double curvature;
    int id;
    double route_length;
    size_t stop_count;
    size_t unique_stop_count;

    name = value.at("name"s).AsString();
    id = value.at("id"s).AsInt();

    Bus route = catalog.FindBus(name);
    //тут как бы для меня самое не понятное - как вывести инфу в консоль используя json
    if (route.name_ == "Error"s) {
        //типа вывести это
        Print(json::Document(json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build()), output);
        return;
    }

    BusInfo info = catalog.GetBusInfo(value.at("name"s).AsString());
    curvature = info.curvature_;
    route_length = info.real_distance_length_;
    stop_count = info.stops_num_;
    unique_stop_count = info.unique_stops_num_;

    Print(json::Document(json::Builder{}.StartDict().Key("curvature"s).Value(curvature).Key("request_id").Value(id).Key("route_length"s).Value(route_length).Key("stop_count"s).Value((int)stop_count).Key("unique_stop_count"s).Value((int)unique_stop_count).EndDict().Build()), output);

}
//-------- stat_requests //--------
/*
////////// render_requests //////////
void Render(MapRenderer& map_renderer, const Dict& value) {

    RenderSimpleSettings(map_renderer, value);

    RenderArraySettings(map_renderer, value);

    RenderVariantSettings(map_renderer, value);

}

void RenderSimpleSettings(MapRenderer& map_renderer, const Dict& value) {
    map_renderer.settings.width = value.at("width").AsDouble();
    map_renderer.settings.height = value.at("height").AsDouble();
    map_renderer.settings.padding = value.at("padding").AsDouble();
    map_renderer.settings.stop_radius = value.at("stop_radius").AsDouble();
    map_renderer.settings.line_width = value.at("line_width").AsDouble();
    map_renderer.settings.bus_label_font_size = value.at("bus_label_font_size").AsInt();
    map_renderer.settings.stop_label_font_size = value.at("stop_label_font_size").AsInt();
    map_renderer.settings.underlayer_width = value.at("underlayer_width").AsDouble();
}

void RenderArraySettings(MapRenderer& map_renderer, const Dict& value) {
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

void RenderVariantSettings(MapRenderer& map_renderer, const Dict& value) {
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
} */

void reguest_handler::PrintMap(transport_catalogue::TransportCatalogue& transport_catalogue, MapRenderer& map_renderer, const Dict& value, std::ostream& output) {
    int id = value.at("id"s).AsInt();
    std::ostringstream svg;
    map_renderer.RenderMap(transport_catalogue, svg);
    Print(json::Document(Builder{}.StartDict().Key("map"s).Value(svg.str()).Key("request_id"s).Value(id).EndDict().Build()), output);
}
//-------- render_requests //--------
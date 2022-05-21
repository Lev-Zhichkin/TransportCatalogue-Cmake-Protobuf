#include "reguest_handler.h"


using namespace transport_catalogue;
using namespace json;
using namespace std::literals;
using namespace reguest_handler;

void reguest_handler::PrintStopInformation(TransportCatalogue& catalog, const Dict& value, std::ostream& output) {
    //Timer timer("STOP INFORMATION PRINTING");

    int id;
    std::string name;

    id = value.at("id"s).AsInt();
    name = value.at("name"s).AsString();
    Array bus_names;

    Stop stop = catalog.FindStop(name);

    //тут как бы дл€ мен€ самое не пон€тное - как вывести инфу в консоль использу€ json
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
    //тут как бы дл€ мен€ самое не пон€тное - как вывести инфу в консоль использу€ json
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

void reguest_handler::PrintMap(transport_catalogue::TransportCatalogue& transport_catalogue, MapRenderer& map_renderer, const Dict& value, std::ostream& output) {
    int id = value.at("id"s).AsInt();
    std::ostringstream svg;
    map_renderer.RenderMap(transport_catalogue, svg);
    Print(json::Document(Builder{}.StartDict().Key("map"s).Value(svg.str()).Key("request_id"s).Value(id).EndDict().Build()), output);
}
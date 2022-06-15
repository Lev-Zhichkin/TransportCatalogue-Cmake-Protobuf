#include "request_handler.h"


using namespace transport_catalogue;
using namespace map_renderer;
using namespace json;
using namespace std::literals;
using namespace request_handler;

request_handler::RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& catalog, map_renderer::MapRenderer& map_renderer, transport_router::RouterSettings router_settings)
: catalog_(catalog), map_renderer_(map_renderer), transport_router_(catalog, router_settings)
{
}

void RequestHandler::ProcessRequest(Array& value, std::ostream& output) {
    //Timer timer("PROCESSING REGUEST");

    std::cout << "[" << std::endl;

    bool f = false;
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            RequestHandler::PrintStopInformation(description.AsMap(), output);
        }
        else if (description.AsMap().at("type"s) == "Bus"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            RequestHandler::PrintBusInformation(description.AsMap(), output);
        }
        else if (description.AsMap().at("type"s) == "Map"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            RequestHandler::PrintMap(description.AsMap(), output);
        }
        else if (description.AsMap().at("type"s) == "Route"s) {
            if (f == true) {
                std::cout << "," << std::endl;
            }
            RequestHandler::PrintRoute(description.AsMap(), output);
        }
        f = true;
    }

    std::cout << "]" << std::endl;

}

void RequestHandler::PrintStopInformation(const Dict& value, std::ostream& output) {
    //Timer timer("STOP INFORMATION PRINTING");

    int id;
    std::string name;

    id = value.at("id"s).AsInt();
    name = value.at("name"s).AsString();
    Array bus_names;

    Stop stop = catalog_.FindStop(name);

    if (stop.name_ == "Error"s) {
        //типа вывести это
        Print(json::Document(json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build()), output);
    }
    else {
        std::set<std::string_view> buses = catalog_.GetBTS().at(name);
        for (auto& bus : buses) {
            bus_names.push_back(std::string(bus));
        }

        Print(json::Document(json::Builder{}.StartDict().Key("buses"s).Value(bus_names).Key("request_id"s).Value(id).EndDict().Build()), output);
    }
}

void RequestHandler::PrintBusInformation(const Dict& value, std::ostream& output) {
    //Timer timer("BUS INFORMATION PRINTING");

    std::string name;
    double curvature;
    int id;
    double route_length;
    size_t stop_count;
    size_t unique_stop_count;

    name = value.at("name"s).AsString();
    id = value.at("id"s).AsInt();

    Bus route = catalog_.FindBus(name);
    if (route.name_ == "Error"s) {
        Print(json::Document(json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build()), output);
        return;
    }

    BusInfo info = catalog_.GetBusInfo(value.at("name"s).AsString());
    curvature = info.curvature_;
    route_length = info.real_distance_length_;
    stop_count = info.stops_num_;
    unique_stop_count = info.unique_stops_num_;

    Print(json::Document(json::Builder{}.StartDict().Key("curvature"s).Value(curvature).Key("request_id"s).Value(id).Key("route_length"s).Value(route_length).Key("stop_count"s).Value((int)stop_count).Key("unique_stop_count"s).Value((int)unique_stop_count).EndDict().Build()), output);
}

void RequestHandler::PrintMap(const Dict& value, std::ostream& output) {
    int id = value.at("id"s).AsInt();
    std::ostringstream svg;
    map_renderer_.RenderMap(catalog_, svg);
    Print(json::Document(json::Builder{}.StartDict().Key("map"s).Value(svg.str()).Key("request_id"s).Value(id).EndDict().Build()), output);
}

void RequestHandler::PrintRoute(const Dict& value, std::ostream& output) {
    std::string from = value.at("from"s).AsString();
    std::string to = value.at("to"s).AsString();
    int id = value.at("id"s).AsInt();

    std::optional<std::vector<transport_router::RouterInfo>> edges_infos = transport_router_.FindFastestRoute(from, to);
    if (!edges_infos) {
        Print(json::Document(json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build()), output);
        return;
    }

    double total_time = 0;
    int wait_time = transport_router_.GetRouterSettings().bus_wait_time_;
    Array items;
    for (transport_router::RouterInfo edge : edges_infos.value()) {
        json::Dict wait_item = json::Builder{}.StartDict().Key("type"s).Value("Wait"s).Key("stop_name"s).Value(std::string(edge.stop_from)).Key("time"s).Value(wait_time).EndDict().Build().AsMap();
        items.push_back(wait_item);

        total_time += edge.total_time;
        json::Dict bus_item = json::Builder{}.StartDict().Key("type"s).Value("Bus"s).Key("bus"s).Value(std::string(edge.bus_name)).Key("span_count"s).Value(edge.span_count).Key("time"s).Value(edge.total_time - wait_time).EndDict().Build().AsMap();
        items.push_back(bus_item);
    }

    Print(json::Document(json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("total_time"s).Value(total_time).Key("items"s).Value(items).EndDict().Build().AsMap()), output);
}
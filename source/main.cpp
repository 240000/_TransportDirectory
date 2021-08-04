#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "map_renderer.h"
#include "serialization.h"



using namespace std;
void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        // make base here
	
            json::Document document;
	document = json_reader::LoadJSON(cin);
	transport_catalogue::TransportCatalogue tc;
	json_reader::FillTransportCatalogue(document, tc);
	serialization::SaveTo(serialization::ParseFilename(document), tc, document);

    } else if (mode == "process_requests"sv) {

        // process requests here
	
        output::OutputParams params;
	params.document = json_reader::LoadJSON(cin);
	serialization::LoadFrom(serialization::ParseFilename(params.document), params);
	params.tc_graph = transport_router::BuildGraph(params.tc, params.edges_ids, params.rs);
	graph::Router<transport_router::Weight> router(params.tc_graph);
	output::Output(params, router, cout);

    } else {
        PrintUsage();
        return 1;
    }
}
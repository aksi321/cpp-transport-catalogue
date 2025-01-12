#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

using namespace catalogue;
namespace utility{
void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);

}

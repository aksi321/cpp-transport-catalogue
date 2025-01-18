#include "stat_reader.h"
#include <iostream>

namespace utility{

void PrintBus(const TransportCatalogue& tansport_catalogue, std::string_view left, std::string_view rigth, std::ostream& output  ){
    BusCounted rez;
    rez = tansport_catalogue.GetBusStatistics(rigth);
    if (!rez.IsEmpty()){
            output<<left<<" "<<rigth<<": "<<rez.amount<<" stops on route, "<<rez.unique<<" unique stops, "<<rez.length<<" route length";
    } else {
        output<<left<<" "<<rigth<<": not found";
    }

}

void PrintStop(const TransportCatalogue& tansport_catalogue, std::string_view left, std::string_view rigth, std::ostream& output  ){
    BusesForStop  answer =tansport_catalogue.GetBusesForStop(rigth);
        output<<left<<" "<<rigth<<":";
        if (answer.request_status == "not found"){
            output<<" not found";
        } else if (answer.request_status == "empty"){
            output<< " no buses";
        } else if (answer.request_status == "found"){
            output<<" buses";
            for (auto s : answer.buses){
                output<<" "<<s;
            }
        }
}

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {   
    std::string_view answer;                  
    size_t pos = request.find(' '); 
    std::string_view left = request.substr(0,pos);    
    std::string_view rigth = request.substr(pos+1);    
    if (left == "Bus"){
        PrintBus(tansport_catalogue, left, rigth, output);    
    } else if (left == "Stop"){
        PrintStop(tansport_catalogue, left, rigth, output);
    } 
    output<<std::endl;
}

}

#include "stat_reader.h"
#include <iostream>

namespace utility{

void PrintBus(const TransportCatalogue& tansport_catalogue, std::string_view left, std::string_view rigth, std::ostream& output  ){
    BusCounted rez;
    rez = tansport_catalogue.ReturnBus(rigth);
    if (!rez.IsEmpty()){
            output<<left<<" "<<rigth<<": "<<rez.amount<<" stops on route, "<<rez.unique<<" unique stops, "<<rez.length<<" route length";
    } else {
        output<<left<<" "<<rigth<<": not found";
    }

}

void PrintStop(const TransportCatalogue& tansport_catalogue, std::string_view left, std::string_view rigth, std::ostream& output  ){
    AllBussForStop  answer =tansport_catalogue.ReturnStop(rigth);
        output<<left<<" "<<rigth<<":";
        if (answer.reqest == "not answer"){
            output<<" not found";
        } else if (answer.reqest == "emty"){
            output<< " no buses";
        } else if (answer.reqest == "full"){
            output<<" buses";
            for (auto s : answer.collect){
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

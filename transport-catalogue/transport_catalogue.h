#pragma once

#include <deque>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <map>
#include "geo.h"
#include <iostream>

namespace catalogue{

	struct Stop{
		std::string name;
		geo_math::Coordinates coordinates;

	};

	struct Bus{
		std::string name;
		std::vector<std::string> stops;
	};

	struct BusesForStop {
		std::string request_status;
		std::set<std::string_view> buses;
	};


	struct BusCounted{
		size_t amount=0;
		size_t unique=0;
		double length=0;

		bool IsEmpty(){
			if( amount==0 && unique==0 && length==0){
				return true;
			}
			return false;
		}
	};




using namespace geo_math;

class TransportCatalogue {

public:

	void AddStop(std::string id , double lat, double lag);
	void AddBus(std::string id, std::vector<std::string_view> stops);
	BusCounted GetBusStatistics(std::string_view id)const;
	BusesForStop GetBusesForStop(std::string_view id )const;
	const Stop* GetStop(std::string_view name) const;
	const Bus* GetBus(std::string_view name) const;

private:

	BusCounted CountStation(std::string_view id ) const;
	void LinkBusToStops(std::string_view id );
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Stop*> stops_ptr_;
	std::unordered_map<std::string_view, Bus*> bus_ptr_;
	std::unordered_map<std::string,std::set<std::string_view>> bases_for_stops_;  

};
}

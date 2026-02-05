#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Country_Data.hpp"

int main(){
    
    Country_Data country_data;
    std::string input = "";
    while (std::cin >> input && input != "EXIT"){
        std::string country_name;
        std::string series_code;
        int year = 0;
        double datum = 0;
        if (input == "LOAD_P2"){
            std::cin >> country_name;
            country_data.load(country_name);
        } else if (input == "UPDATE_P2"){
            std::cin >> series_code;
            std::cin >> year;
            std::cin >> datum;
            country_data.update(series_code, year, datum);
        } else if (input == "PRINT_P2"){
            std::cin >> series_code;
            country_data.printSeries(series_code);
        } else if (input == "LIST_P2"){
            country_data.listSeries();
        } else if (input == "ADD_P2"){
            std::cin >> series_code;
            std::cin >> year;
            std::cin >> datum;
            country_data.addSeriesElement(series_code, year, datum);
        } else if (input == "DELETE_P2"){
            std::cin >> series_code;
            country_data.deleteSeries(series_code);
        } else if (input == "BIGGEST_P2"){
            country_data.seriesWithBiggestMean();
        } else if (input == "TS_P2"){
            std::cin >> series_code;
            country_data.seriesSizeCapacity(series_code);
        }
    }
}
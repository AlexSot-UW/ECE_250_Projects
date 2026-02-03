#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Time_Series.hpp"

int main(){
    
    Time_Series time_series;
    std::string input = "";
    while (std::cin >> input && input != "EXIT"){
        std::string filename;
        int year = 0;
        double datum = 0;
        if (input == "LOAD_P1"){
            std::cin >> filename;
            time_series.load(filename);
        } else if (input == "UPDATE_P1"){
            std::cin >> year;
            std::cin >> datum;
            time_series.update(year, datum);
        } else if (input == "PRINT_P1"){
            time_series.print();
        } else if (input == "ADD_P1"){
            std::cin >> year;
            std::cin >> datum;
            time_series.add(year, datum);
        } else if (input == "MEAN_P1"){
            time_series.mean_p1();
        } else if (input == "MONOTONIC_P1"){
            time_series.is_monotonic();
        } else if (input == "FIT_P1"){
            double m = 0;
            double b = 0;
            time_series.best_fit(m, b);
        }
    }
}
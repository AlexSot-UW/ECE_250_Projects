#ifndef COUNTRY_DATA_H
#define COUNTRY_DATA_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Time_Series.hpp"

class Time_Series;

class Country_Data {
private:
    std::string DATA_FILE_NAME;
    int MIN_ARRAY_SIZE;
    std::string country_name;
    std::string country_code;

    Time_Series* country_data;

    std::size_t array_size;
    unsigned int last_idx;

public:
    Country_Data();
    ~Country_Data();
    
    void load(std::string country_name);
    void addSeries(std::istringstream& series);
    void listSeries();
    bool checkAndResizeArray();
    void resizeArray(size_t& new_size);
    void addSeriesElement(std::string series_code, int year, double datum);
    void update(std::string series_code, int year, double datum);
    void printSeries(std::string series_code);
    void deleteSeries(std::string series_code);
    void seriesWithBiggestMean();
    void seriesSizeCapacity(std::string series_code);
    int returnSeriesIdx(std::string series_code);
};

#endif
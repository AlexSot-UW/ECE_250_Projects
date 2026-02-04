#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include "Country_Data.hpp"
#include "Time_Series.hpp"

Country_Data::Country_Data():
    MIN_ARRAY_SIZE(2),
    DATA_FILE_NAME("lab2_multidata.csv"),
    country_name(""),
    country_code(""),
    country_data(nullptr),
    array_size(0),
    last_idx(0)
{}

void Country_Data::load(std::string c_name){
    delete[] country_data;
    country_data = nullptr;
    country_name = c_name;

    std::ifstream file(DATA_FILE_NAME);
    std::string line;
    std::string name = "";

    bool end_flag = false;

    last_idx = 0;
    array_size = MIN_ARRAY_SIZE;

    country_data = new Time_Series[array_size];

    checkAndResizeArray();

    // std::cout << (std::getline(file, line) && true) << std::endl;

    while (std::getline(file, line)){
        std::istringstream iss(line);

        std::getline(iss, name, ',');
        if (name == country_name){
            std::getline(iss, country_code, ',');
            addSeries(iss);
            break;
        }
    }

    while (std::getline(file, line)){
        std::istringstream iss(line);

        std::getline(iss, name, ',');

        if (name != country_name){
            break;
        }

        std::getline(iss, country_code, ',');

        addSeries(iss);
    }

    file.close();

    std::cout << "success" << std::endl;
}

void Country_Data::addSeries(std::istringstream& series){
    checkAndResizeArray();

    Time_Series tseries;
    tseries.load(series);

    country_data[last_idx] = tseries;

    last_idx++;
}

void Country_Data::listSeries(){
    for (unsigned int i = 0; i < last_idx; i++){
        std::cout << " " << country_data[i].getSeriesName();
    }
}

void Country_Data::addSeriesElement(std::string series_code, int year, double datum){
    int seriesIdx = returnSeriesIdx(series_code);

    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        country_data[seriesIdx].addSeriesElement(year, datum);
    }
}

void Country_Data::update(std::string series_code, int year, double datum){
    int seriesIdx = returnSeriesIdx(series_code);

    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        country_data[seriesIdx].addSeriesElement(year, datum);
    }
}

void Country_Data::printSeries(std::string series_code){
    int seriesIdx = returnSeriesIdx(series_code);

    // std::cout << country_name << std::endl;

    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        country_data[seriesIdx].print();
    }
}

void Country_Data::deleteSeries(std::string series_code){
    int seriesIdx = returnSeriesIdx(series_code);

    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        std::cout << "success" << std::endl;
        for (int i = seriesIdx + 1; i < last_idx; i++){
            country_data[i - 1] = country_data[i];
        }
        last_idx--;
    }
}

void Country_Data::seriesWithBiggestMean(){
    std::string series_code = "failure";
    double max = 0;
    
    for (unsigned int i = 0; i < last_idx; i++){
        double curr = country_data[i].mean(); 
        if (curr > max) {
            max = curr;
            series_code = country_data[i].getSeriesCode();
        }
    }

    std::cout << series_code << std::endl;
}

void Country_Data::seriesSizeCapacity(std::string series_code){
    int seriesIdx = returnSeriesIdx(series_code);

    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        if (country_data[seriesIdx].hasValidData()){
            std::cout << "size is " << country_data[seriesIdx].getArraySize() << " capacity is " << country_data[seriesIdx].getLastIdx();    
        } else {
            std::cout << "size is " << 0 << " capacity is " << 2;
        }
    }
}

int Country_Data::returnSeriesIdx(std::string series_code){
    // std::cout << last_idx << std::endl;
    for (unsigned int i = 0; i < last_idx; i++){
        // std::cout << country_data[i].getSeriesCode() << " vs " << series_code << std::endl;
        if (country_data[i].getSeriesCode() == series_code){
            return i;
        }
    }
    return -1;
}

bool Country_Data::checkAndResizeArray(){
    bool flag = false;
    size_t new_size = array_size;

    // If statement checks if series needs to be resized.
    if (last_idx >= array_size){ // If ran out of array capacity double array size.
        new_size = array_size * 2;
        flag = true;
        resizeArray(new_size);
    } else if (last_idx <= array_size/4 && last_idx != 0){ // If array size is 4 times larger then array capacity resize it.
        new_size = array_size/2;
        flag = true;
        resizeArray(new_size);
    }

    return flag;
}

void Country_Data::resizeArray(size_t& new_size){
    // Declare two new temporary arrays with size new_size.
    Time_Series* temp_data = new Time_Series[new_size];

    // Copy all array values into new array.
    for (unsigned int i = 0; i < last_idx; i++){
        temp_data[i] = country_data[i];
    }

    // Delete pointers to old arrays
    delete[] country_data;
    country_data = temp_data;

    // Set array_size to new value.
    array_size = new_size;
}


Country_Data::~Country_Data(){
    delete[] country_data;
}
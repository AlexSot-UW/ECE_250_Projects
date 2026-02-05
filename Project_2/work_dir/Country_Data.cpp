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
    std::cout << country_name << " " << country_code;
    for (unsigned int i = 0; i < last_idx; i++){
        std::cout << " " << country_data[i].getSeriesName();
    }
    std::cout << "" << std::endl;
}

void Country_Data::addSeriesElement(std::string series_code, int year, double datum){
    int seriesIdx = returnSeriesIdx(series_code);

    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        country_data[seriesIdx].add(year, datum);
    }
}

/*
* Description: Update time series specified by series code value in series.
*              Checks if series exists.
*              Negative data causes series entry to be removed.
*              Positive entry that exists updates existing series entry.
*              Nothing is done if entry is not in series.
* Input:       std::string series_Code, int: year , double: datum
* Output:      Prints failure if series not stored in country_data array.
*              Prints failure if updated data value is below 0. 
*              Print failure if series element does not exist.
*              Print success if series element exists, and data value above 0.
*/
void Country_Data::update(std::string series_code, int year, double datum){
    // Returns index of series.
    int seriesIdx = returnSeriesIdx(series_code);

    //
    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        country_data[seriesIdx].update(year, datum);
    }
}

/*
* Description: Prints all valid data in series specified by the series code, in format (year, data).
*              Invalid data is a datapoint equal to -1, these data entries are ignored.
*              Prints failure if no valid data entries.
* Input:       std::string: series_code (the series code by which the time series will be identified in the array).
*/
void Country_Data::printSeries(std::string series_code){

    // Returns index of series in the series array, needed in order to find right series to call method on.
    int seriesIdx = returnSeriesIdx(series_code);

    // If series idx is less then zero (-1) that means the series wasnt found, and "failure" is printed, otherwise call the print method on the series, to print its contents.
    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        country_data[seriesIdx].print();
    }
}

/*
* Description: Deletes a series specified by the series code, from the array of Time_Series stored in the class.
* Input:       std::string: series_code (the series code by which the time series will be identified in the array).
*/
void Country_Data::deleteSeries(std::string series_code){

    // Returns index of series in the series array, needed in order to find right series to remove.
    int seriesIdx = returnSeriesIdx(series_code);

    // If series idx is less then zero (-1) that means the series wasnt found, and "failure" is printed, otherwise remove element from the country_data array.
    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        std::cout << "success" << std::endl;
        // Removes the series element from the array of series.
        for (int i = seriesIdx + 1; i < last_idx; i++){
            country_data[i - 1] = country_data[i];
        }
        // Decreases last_idx by 1 (decreasing array size pointer).
        last_idx--;
    }

    // Checks if array needs to be resized or not.
    checkAndResizeArray();
}

void Country_Data::seriesWithBiggestMean(){
    std::string series_code = "failure";
    double max;
    
    double curr = country_data[0].mean();
    
    unsigned int counter = 1;

    while (curr == 0 && counter < last_idx){
        curr = country_data[counter].mean();
        counter++;
    }

    max = curr;

    for (unsigned int i = 0; i < last_idx; i++){
        curr = country_data[i].mean();
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
            std::cout << "size is " <<  country_data[seriesIdx].getLastIdx() << " capacity is " <<  country_data[seriesIdx].getArraySize() << std::endl;    
        } else {
            std::cout << "size is " << 0 << " capacity is " << 2 << std::endl;
        }
    }
}

int Country_Data::returnSeriesIdx(std::string series_code){
    for (unsigned int i = 0; i < last_idx; i++){
        if (country_data[i].getSeriesCode() == series_code){
            return i;
        }
    }
    return -1;
}

/*
* Description: Checks class array needs to be resized. Resizes it if one of two seperate conditions are met:
*                   If array size is less then or equal to array capacity ran out of space then resize. Return true.
*                   If array capacity is 4 times larger then array size. Return true.
* Output:      bool: Whether function was resized or not.
*/
bool Country_Data::checkAndResizeArray(){
    bool flag = false;
    size_t new_size = array_size;

    if (last_idx >= array_size){
        new_size = array_size * 2;
        flag = true;
        resizeArray(new_size);
    } else if (last_idx <= array_size/4 && last_idx != 0){
        new_size = array_size/2;
        flag = true;
        resizeArray(new_size);
    }

    return flag;
}

/*
* Description: Resizes array of time series stored by this class.
* Input:       size_t: new_size (new size of array.)
*/
void Country_Data::resizeArray(size_t& new_size){
    // Declare new temporary array with size new_size.
    Time_Series* temp_data = new Time_Series[new_size];

    // Copy all array values into new array.
    for (unsigned int i = 0; i < last_idx; i++){
        temp_data[i] = country_data[i];
    }

    // Delete pointers to old array
    delete[] country_data;
    country_data = temp_data;

    // Set array_size to new value.
    array_size = new_size;
}


Country_Data::~Country_Data(){
    delete[] country_data;
}
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

/*
* Description: Load csv file series data for a country.
*              Loads every line of time series data associated with a country.
* Input:       std::string: c_name (name of country)
*/
void Country_Data::load(std::string c_name){
    // Deallocates country_data array to prevent memory leaks.
    delete[] country_data;
    country_data = nullptr;
    country_name = c_name;
    country_code = "";

    // Creates new file stream/string variables which will be used to read from file/stored important data.
    std::ifstream file(DATA_FILE_NAME);
    std::string line;
    std::string name = "";

    // Resets array capacity/size variables
    last_idx = 0;
    array_size = MIN_ARRAY_SIZE;

    // Allocates new array of Time_Series objects which will store all of the data.
    country_data = new Time_Series[array_size];

    // Loops until the first series with the correct country name is found.
    while (std::getline(file, line)){
        std::istringstream iss(line);

        std::getline(iss, name, ',');
        // Checks if series name is equal to the country name, if it is then save country code, add the series and break the loop.
        if (name == country_name){
            std::getline(iss, country_code, ',');
            
            // Add series object to the class array.
            addSeries(iss);
            break;
        }
    }

    // Loops until the first series with the incorrect country name is found.
    while (std::getline(file, line)){
        std::istringstream iss(line);

        std::getline(iss, name, ',');
        
        // Checks if series name is equal to the country name, if it isnt then break the loop.
        if (name != country_name){
            break;
        }

        std::getline(iss, country_code, ',');
        
        // Add series object to the class array.
        addSeries(iss);
    }

    file.close();

    std::cout << "success" << std::endl;
}


/*
* Description: Adds a new series to array of country_data, and loads data into it using the load method.
* Input:       std::istringstream&: series (the line of data read from the csv file, which will then be processed by the load method of the Time_Series class, thus saving data into the object).
*/
void Country_Data::addSeries(std::istringstream& series){
    // Checks if array needs to be resized or not before addign new element.
    checkAndResizeArray();

    // Declares new Time_Series variable, and calls the laod method on it thus loading all the data into it.
    Time_Series tseries;
    tseries.load(series);

    // Adds time series object into the country_data array.
    country_data[last_idx] = tseries;

    // Increases array size.
    last_idx++;
}

/*
* Description: List all the series, preceded by country name and country code.
*/
void Country_Data::listSeries(){
    // Print country name and country code.
    std::cout << country_name << " " << country_code;

    // Loops through array of time series, and print out their names.
    for (unsigned int i = 0; i < last_idx; i++){
        std::cout << " " << country_data[i].getSeriesName();
    }
    std::cout << "" << std::endl;
}

/*
* Description: Add a element to series, specified by series code, and whether or not operation is successful, print to console, either success or failure.
*/
void Country_Data::addSeriesElement(std::string series_code, int year, double datum){
    // Returns series idx (-1 if not found)
    int seriesIdx = returnSeriesIdx(series_code);

    // Check if series idx is less then zero (print failure if it is, otherwise add it to series).
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
    // Returns index of series. Returns -1 to signify series not found in that array.
    int seriesIdx = returnSeriesIdx(series_code);

    // Checks if series in array, if it is then calls the update method on it.
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

/*
* Description: Prints out the series code, of the series which has the largest mean out of all of the series stored inside of the time series array.
*/
void Country_Data::seriesWithBiggestMean(){
    // If country has no series then automatically print failure.
    if (last_idx == 0) {
        std::cout << "failure" << std::endl;
        return;
    }

    // Set the output equal to failure as default
    std::string series_code = "failure";

    // Initalize the max/curr variables which will store mean values throughout the loop.
    double max;
    
    double curr = country_data[0].mean();
    
    unsigned int counter = 1;

    // Loops until finds the first mean that isnt zero (first series with valid data).
    while (curr == 0 && counter < last_idx){
        curr = country_data[counter].mean();
        counter++;
    }

    max = curr;

    // Loops through the time series array and thus finds the maximum mean value, by comparing current maximum with current mean.
    for (unsigned int i = 0; i < last_idx; i++){
        curr = country_data[i].mean(); // Sets curr equal to current mean.

        // Checks if this new series mean is greater then the maximum.
        if (curr > max) {
            max = curr;
            series_code = country_data[i].getSeriesCode();
        }
    }

    // Prints out the series code of the series with the largest mean.
    std::cout << series_code << std::endl;
}

/*
* Description: Prints out the capacity/array size of series specified by series code.
*/
void Country_Data::seriesSizeCapacity(std::string series_code){
    // Returns index of series (-1 if doesnt exist)
    int seriesIdx = returnSeriesIdx(series_code);

    // Checks if series exists, if it doesn then commits to logic, otherwise prints failure.
    if (seriesIdx < 0){
        std::cout << "failure" << std::endl;
    } else {
        // Check if series has valid data, if not then prints default values of 0/2 for array size/capacity.
        if (country_data[seriesIdx].hasValidData()){
            std::cout << "size is " <<  country_data[seriesIdx].getLastIdx() << " capacity is " <<  country_data[seriesIdx].getArraySize() << std::endl;    
        } else {
            std::cout << "size is " << 0 << " capacity is " << 2 << std::endl;
        }
    }
}

/*
* Description: Returns index of series, specified by series_code (index inside the global country_data array).
*/
int Country_Data::returnSeriesIdx(std::string series_code){
    // Loops through the country_data array until it finds a series with a matching code, otherwise it outputs -1 if series not found.
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
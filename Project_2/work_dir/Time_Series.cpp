#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include "Time_Series.hpp"

Time_Series::Time_Series()
    : MIN_ARRAY_SIZE(2),
      FIRST_YEAR(1960),
      LAST_YEAR(2023),
      MISSING_DATA_INDICATOR(-1.0),
      series_name(""),
      series_code(""),
      years(nullptr),
      data(nullptr),
      array_size(0),
      last_idx(0)
{}

/*
* Description: Load csv file series data.
*              Loads first 4 lines of csv file including series name and series code.
*              Sequentially loads and stores all data from csv file, including invalid data.
* Input:       std::string: filename
*/
void Time_Series::load(std::istringstream& input_line){
    // Deletes array, and reinitializes all variables related to file size/capacity.
    delete[] years; 
    years = nullptr;
    delete[] data;
    data  = nullptr;

    array_size = MIN_ARRAY_SIZE;
    last_idx = 0;

    // Initializes new arrays which will contain series data.
    years = new int[array_size];
    data = new double[array_size];

    std::stringstream ss;
    std::string line;

    // Reads first 2 entries of line.
    std::getline(input_line, series_name, ',');
    // std::cout << series_name << std::endl;
    std::getline(input_line, series_code, ',');
    // std::cout << series_code << std::endl;

    // Reads data stored in csv and stores it in the arrays. Reads until runs out of file space.
    while (std::getline(input_line, line, ',')){
            std::stringstream ss(line);

            double data_point = std::stod(ss.str());

            int year = FIRST_YEAR + last_idx;
            double datum;
            ss >> datum;
            addSeriesLoad(year, datum);
    }
    // std::cout << series_code << std::endl;
}

/*
* Description: Prints all valid data in series, in format (year, data).
*              Invalid data is a datapoint equal to -1, these data entries are ignored.
*              Prints failure if no valid data entries.
*/
void Time_Series::print(){
    // Sets numValidData variable to 0.
    int numValidData = 0;
    for (size_t i = 0; i < last_idx; i++){
        // If data entry is invalid then don't print series element.
        if (data[i] != MISSING_DATA_INDICATOR){
            numValidData++; // Increases by 1, to ensure program knows there is valid data in series.
            std::cout << "(" << years[i] << "," << data[i] << ") ";
        }
    }

    // If no valid data, then print failure.
    if (numValidData == 0){
        std::cout << "failure";
    }
    std::cout << std::endl;
}

/*
* Description: Update existing time series value in series.
*              Negative data causes series entry to be removed.
*              Positive entry that exists updates existing series entry.
*              Nothing is done if entry is not in series.
* Input:       int: year , double: datum
* Output:      Prints failure if updated data value is below 0. 
*              Print failure if series element does not exist.
*              Print success if series element exists, and data value above 0.
*/
void Time_Series::update(int year, double datum){
    // Return series element index.
    int idx = returnYearIdx(year);

    // Check if element is in series. If not then do nothing and output failure.
    if (!(idx < 0 || years[idx] != year) && data[idx] != MISSING_DATA_INDICATOR){
        // If new data entry below zero, remove this series member. Else, update with new values.
        if (datum < 0){
            removeSeriesElement(idx);
            std::cout << "success" << std::endl;
        } else {
            data[idx] = datum;
            std::cout << "success" << std::endl;
        }
    }
    else {
        std::cout << "failure" << std::endl;
    }
}

/*
* Description: Prints mean of series elements data entries
*              Invalid data is ignored.
* Output:      If no valid data then prints failure.
*              If valid data exists, print: "mean is " <series mean>.  
*/
void Time_Series::mean_p1(){
    // Calculates the mean of series.
    double series_mean = mean();

    // Outputs mean of series, or failure if mean is zero (no valid data).
    if (series_mean != 0){
        std::cout << "mean is " + std::to_string(series_mean) << std::endl;
    } else {
        std::cout << "failure" << std::endl;
    }
    
}

/*
* Description: Calculates, and outputs mean of series.
*              Invalid data is ignored when doing the summation.
*              If no valid data then output 0.
* Output:      double: mean (mean of data series)
*/
double Time_Series::mean(){
    // Set to inital value of zero.
    double mean = 0;

    int numValidData = 0;
    
    // Loops through data series
    for (unsigned int i = 0; i < last_idx; i++){
        // Increases mean 
        if (data[i] != MISSING_DATA_INDICATOR){
            mean += data[i];
            numValidData++;
        }
    }
    
    // Checks if mean is zero, if it isn't then divides it by numValidData
    if (mean != 0){
        mean /= numValidData;
    }
    return mean;
}

/*
* Description: Checks if series data is monotonic, that is it is either always increasing, or always decreasing.
*              Invalid data is ignored.
*              Series is monotonic if it only has one element.
*              Series is not monotonic if it only has one element.
* Output:      bool: is series monotonic or not.
*/
bool Time_Series::is_monotonic() {
    if (last_idx == 0) {
        std::cout << "failure" << std::endl;
        return false;
    }

    // Find first valid data entry in series.
    unsigned int j = 0;
    while (j < last_idx && data[j] == MISSING_DATA_INDICATOR){
        j++;
    }

    // If no valid series data entries.
    if (j == last_idx) {
        std::cout << "failure" << std::endl;
        return false;
    }

    // Find second valid data entry in series.
    unsigned int k = j + 1;
    while (k < last_idx && data[k] == MISSING_DATA_INDICATOR){
        k++;
    }

    // If only one valid data entry.
    if (k == last_idx) {
        std::cout << "series is monotonic" << std::endl;
        return true;
    }

    // Check if fucntion is decreasing or increasing.
    bool nonDecreasing = (data[k] >= data[j]);
    double prev = data[k];

    // Loop trhough until, last_idx reached, or until return false, from function being non-monotonous.
    for (unsigned int i = k + 1; i < last_idx; i++) {
        if (data[i] == MISSING_DATA_INDICATOR) continue;

        if (nonDecreasing) {
            if (data[i] < prev) {
                std::cout << "series is not monotonic" << std::endl;
                return false;
            }
        } else {
            if (data[i] > prev) {
                std::cout << "series is not monotonic" << std::endl;
                return false;
            }
        }
        prev = data[i];
    }

    std::cout << "series is monotonic" << std::endl;
    return true;
}

/*
* Description: Finds the linear function of best fit, of the form (y = mx +b), for the series.
*              Sets m, b to 0 if no valid data, and prints failure, otherwise prints the best fit function.
* Input:       double&: m (represents reference to variable holding slope of best fit function), double&: b (represents bias of best fit function).
* Output:      bool: return true, if valid data exists, false if no valid data.
*/
bool Time_Series::best_fit(double &m, double &b){
    // Sets variables to initial values.
    m = 0;
    b = 0;
    
    int numValidData = 0;

    // Declare member variables.
    int sigma_xi = 0;
    double sigma_yi = 0;
    double dot_sigma_x_y = 0;
    int sigma_x_squared = 0;

    // Loops through entire series, iterate all the necessary variables which are part of the best fit function.
    for (int i = 0; i < last_idx; i++){
        if (data[i] != MISSING_DATA_INDICATOR){
            numValidData++;
            sigma_xi += years[i];
            sigma_yi += data[i];
            dot_sigma_x_y += years[i] * data[i];
            sigma_x_squared += years[i] * years[i];
        }
    }

    // If valid data exists then return true and print the slope/bias to console.
    if (numValidData > 0){
        // Use the best fit formula to compute the values of m and b.
        m = (numValidData * dot_sigma_x_y - sigma_xi * sigma_yi)/(numValidData * sigma_x_squared - sigma_xi * sigma_xi);
        b = (sigma_yi - m * sigma_xi)/numValidData;   
        std::cout << "slope is " + std::to_string(m) + " intercept is " + std::to_string(b) << std::endl;
        return true;
    }

    // If no valid data print failure and return false.
    std::cout << "failure" << std::endl;
    return false;
}

/*
* Description: Add a element to series, and whether or not operation is successful, print to console, either success or failure.
*/
void Time_Series::add(int year, double datum){
    // Checks whether has reached max capacity, and resizes if needed.
    checkAndResizeSeries();

    // Add element to series, and if operation succesful, prints success, otherwise prints failure.
    bool flag = addSeriesElement(year, datum);
    if(!flag){
        std::cout << "failure" << std::endl;
    } else {
        std::cout << "success" << std::endl;
    }
}

/*
* Description: Function used in place of regular addSeriesElement() function, to add series element during LOAD_P1 command execution.
*              Used because size of series is predetermined (1960 to 2023), and complex logic is simply not needed.
* Input:       int: year (entry year), double: datum (data to be added)
*/
void Time_Series::addSeriesLoad(int year, double datum){
    // Checks wether function needs to be resized or not.
    checkAndResizeSeries();

    // Adds series element with appropriate year, and datum value
    years[last_idx] = year;
    data[last_idx] = datum;
    // Iterates last_idx by 1.
    last_idx++;
}

/*
* Description: Add a element (year,data) to series.
*              If series entry does not exist, or it does but it has negative value, add entry to series.
* Input:       int: year (entry year), double: datum (data to be added)
* Output:      bool: isSucces (outputs true if successfully added value, false otherwise)
*/
bool Time_Series::addSeriesElement(int year, double datum){
    // Checks if series array is at full capacity
    checkAndResizeSeries();

    // Returns index of entry which has this year (if none exists, outputs either -1 or nearest index less then input year)
    int value_idx = returnYearIdx(year);
    
    // If value_idx < 0, means input year is less then smallest entry in series.
    // If an index is returned, but entry does not currently exist at that index, insert entry right after returned index.
    // If entry exists in series, then check if it has valid data, if not then add valid data.
    if (value_idx < 0){
        // Insert new entry, at the front of the series (as first entry)
        insertSeriesElement(year, datum, 0);
        return true;
    } else if (years[value_idx] != year){
        // Insert new entry, right after returned index
        insertSeriesElement(year, datum, value_idx + 1);
        return true;
    } else if (years[value_idx] == year && data[value_idx] == MISSING_DATA_INDICATOR){
        // Update entry with valid data
        data[value_idx] = datum;
        return true;
    } 
    return false;
}

/*
* Description: Removes entry from series.
* Input:       int: idx (index of element to be removed).
*/
void Time_Series::removeSeriesElement(int idx){
    // Loops through array after the index to be removed, thus shifting all the values down by one.
    for (int i = idx + 1; i < last_idx; i++){
        years[i - 1] = years[i];
        data[i - 1] = data[i];
    }
    
    // Decrement last_idx by one
    last_idx--;
}

/*
* Description: Insert element into series, between two elements, or at the end, or beginning of series.
* Input:       int: year (entry year), double: datum (data to be added), size_t: element_idx (idx of element to be added)
*/
void Time_Series::insertSeriesElement(int year, double datum, size_t element_idx){
    // Checks and resizes series, in case it is at max capacity.
    checkAndResizeSeries();

    // Increase last_idx variable.
    last_idx++;

    // Set the new last entries to a temporary value of zero.
    years[last_idx] = 0;
    data[last_idx] = 0;

    // Declare temporary variables.
    int temp_year = 0;
    int temp_datum = 0;

    // Shift all values of series to the right by one.
    for (unsigned int i = element_idx + 1; i < last_idx; i++){
        temp_year = years[i];
        temp_datum = data[i];

        years[i] = years[i - 1];
        data[i] = data[i - 1];
    }
}

/*
* Description: Checks if series needs to be resized. Resizes it if one of two seperate conditions are met:
*                   If array size is less then or equal to array capacity ran out of space then resize. Return true.
*                   If array capacity is 4 times larger then array size. Return true.
* Output:      bool: Whether function was resized or not.
*/
bool Time_Series::checkAndResizeSeries(){  
    bool flag = false;
    size_t new_size = array_size;

    // If statement checks if series needs to be resized.
    if (last_idx >= array_size){ // If ran out of array capacity double array size.
        new_size = array_size * 2;
        flag = true;
        resizeSeries(new_size);
    } else if (last_idx <= array_size/4 && last_idx != 0){ // If array size is 4 times larger then array capacity resize it.
        new_size = array_size/2;
        flag = true;
        resizeSeries(new_size);
    }

    return flag;
}

/*
* Description: Resizes series with new size.
* Input:       size_t&: new_size (new array size).
*/
void Time_Series::resizeSeries(size_t& new_size){
    // Declare two new temporary arrays with size new_size.
    int* temp_years = new int[new_size];
    double* temp_data = new double[new_size];

    // Copy all array values into new array.
    for (unsigned int i = 0; i < last_idx; i++){
        temp_years[i] = years[i];
        temp_data[i] = data[i];
    }

    // Delete pointers to old arrays
    delete[] years;
    delete[] data;

    // Reassign array points to new arrays.
    years = temp_years;
    data = temp_data;

    // Set array_size to new value.
    array_size = new_size;
}

/*
* Description: Use binary search to return index of year in the series.
*              If year is less then first year value in series, then return -1.
*              If year is not in series return index of largest year smaller then the input year.
* Input:       int: year (year to be found)
* Output:      int: idx (idx of year in series)
*/
int Time_Series::returnYearIdx(int year){
    // If input year less then first year return -1.
    if (year < years[0]){
        return -1;
    }
    
    // Set variables
    int start = 0;
    int end = last_idx - 1;
    int mid = (end + start) / 2;
    
    // While end variable is greather then or equal to start variable keep looping.
    while(end >= start){
        // Change end/start based on how large the value of year is compared to array elements.
        if (years[mid] > year){
            end = mid - 1;
        } else if (years[mid] < year){
            start = mid + 1;
        } else {
            return mid;
        }
        mid = (end + start) / 2;
    }
    return mid;
}

std::string Time_Series::getSeriesName(){
    return series_name;
}

std::string Time_Series::getSeriesCode(){
    return series_code;
}

std::size_t Time_Series::getArraySize(){
    return array_size;
}

unsigned int Time_Series::getLastIdx(){
    return last_idx;
}  

bool Time_Series::hasValidData(){
    for (unsigned int i = 0; i < last_idx; i++){
        if (last_idx != MISSING_DATA_INDICATOR){
            return true;
        }
    }
    return false;
}

Time_Series& Time_Series::operator=(const Time_Series& other){
    if (this == &other) {
        return *this;
    }

    series_name = other.series_name;
    series_code = other.series_code;
    array_size  = other.array_size;
    last_idx    = other.last_idx;

    int* new_years = nullptr;
    double* new_data = nullptr;

    if (other.array_size > 0) {
        new_years = new int[other.array_size];
        new_data  = new double[other.array_size];

        for (unsigned int i = 0; i < other.last_idx; i++) {
            new_years[i] = other.years[i];
            new_data[i]  = other.data[i];
        }
    }

    delete[] years;
    delete[] data;

    years = new_years;
    data  = new_data;

    return *this;
}


Time_Series::~Time_Series(){
    delete[] years;
    delete[] data;
}
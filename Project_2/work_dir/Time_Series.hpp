#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#ifndef TIME_SERIES_H
#define TIME_SERIES_H


class Time_Series {
private:
    int MIN_ARRAY_SIZE;
    int FIRST_YEAR;
    int LAST_YEAR;
    double MISSING_DATA_INDICATOR;

    std::string series_name;
    std::string series_code;

    int* years;
    double* data;

    std::size_t array_size;
    unsigned int last_idx;

public:
    Time_Series();
    ~Time_Series();
    
    void load(std::istringstream& input_line);
    bool addSeriesElement(int year, double datum);
    void addSeriesLoad(int year, double datum);
    void removeSeriesElement(int idx);
    bool checkAndResizeSeries();
    void resizeSeries(size_t& new_size);
    void print();
    void add(int year, double datum);
    void update(int year, double datum);
    double mean();
    void mean_p1();
    bool is_monotonic();
    bool best_fit(double &m, double &b);
    void insertSeriesElement(int year, double data, size_t element_idx);
    int returnYearIdx(int year);

// P2 New Methods:
    std::string getSeriesName();
    std::string getSeriesCode();
    std::size_t getArraySize();
    unsigned int getLastIdx(); 
    bool hasValidData();   
    Time_Series& operator=(const Time_Series& other);
};
#endif

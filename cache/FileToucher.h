//
// Created by hilla on 1/8/19.
//

#ifndef HNJ_2NDROAD_FILETOUCHER_H
#define HNJ_2NDROAD_FILETOUCHER_H

#include <string>

using namespace std;

class FileToucher {
    static FileToucher *instance;
    FileToucher() = default;
public:
    static FileToucher *getInstance();

    string getByKey(string fileName, string key);

    void writeToFile(string fileName, string SolFormat,string ProbFormat);

};


#endif //HNJ_2NDROAD_FILETOUCHER_H
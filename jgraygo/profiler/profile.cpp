/*
 *  profile.cpp
 *
 *  Created by Jonathan Maletic.
 *  Copyright 2021 Kent State University. All rights reserved.
 *  Spring 2021
 *  Modified by:
 *
 */

#include "profile.hpp"

////////////////////////////////////////////////////////////////////////
// Prints out the profile.
//
// TODO: Very simple output, need to make it into columns with nice headings.
// 
std::ostream& operator<< (std::ostream& out, const profile& p) {
    
    out << std::endl << "File: " << p.fname << std::endl;
    out << "<============================================>" << std::endl;
    out << "Line Number/Name\t\tTimes Called" << std::endl;
    for(std::map<std::string, int>::const_iterator i = p.stmt.begin(); i != p.stmt.end(); ++i) {
        out << i->first;
        if (i->first.length() > 7 && i->first.length() <= 15) out << "\t\t\t";
        else if (i->first.length() > 15 && i->first.length() <= 23) out << "\t\t";
        else if (i->first.length() > 23) out << "\t";
        else out << "\t\t\t\t";
        out << i->second << std::endl;
    }
    return out;
}



////////////////////////////////////////////////////////// 
// REQUIRES:  n >= 0
// ENSURES: Returns a text version of a positive integer long
std::string intToString(int n) {
    assert(n >= 0);
    std::string result;
    
    if (n == 0) return "0";
    while (n > 0) {
        result = char(int('0') + (n % 10)) + result;
        n = n / 10;
    }  
    return result;
}

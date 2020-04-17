/**\
**
**  Written by Manuel Fischer
**
\**/

#include "gstream.hpp"

#include <vector>
#include <iostream>

int main()
{
    using namespace gsExperimental;
    using namespace std::literals;

    std::string myString = "Hello World";
    std::string output;

    gsYieldFrom(myString) | gsMap([](char c){ return std::toupper(c); }) | gsInsertBack(output);
    std::cout << output << '\n';

    gsYieldFrom(myString) | gsMap([](char c){ return std::tolower(c); }) | gsOverwriteForward(output.begin());
    std::cout << output << '\n';

    std::fill(output.begin(), output.end(), '.');

    auto overwriter = gsOverwriteForward(output.begin());
    //gsYieldFromCopy("Hello"s) | std::move(overwriter);
    gsYieldFromCopy("Hello"s) | gsRef(overwriter);
    gsYieldFromCopy("."s)     | gsRef(overwriter);
    gsYieldFromCopy("World"s) | gsRef(overwriter);
    //gsYieldFromCopy("Hello"s) | &overwriter;
    //gsYieldFromCopy("world"s) | &overwriter;

    std::cout << output << '\n';
}

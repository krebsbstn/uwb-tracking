#pragma once

/*Define datatype for uwb addresses*/
typedef long long uwb_addr;

/*Define datatype for Coordinates*/
struct coordinate
{
    double x;
    double y;
    double z;
};

/*Define roles of a the PCB*/
enum role{
    reserved = 0,
    tag = 1,
    anchor = 2
};
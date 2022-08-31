#pragma once

struct scr_size_data
{
    scr_size_data(){}

    scr_size_data(size_t scr_height, size_t scr_width, size_t scr_starty, size_t scr_startx)
    : height{scr_height}
    , width{scr_width}
    , starty{scr_starty}
    , startx{scr_startx}
    {}

    size_t height;
    size_t width; 
    size_t starty; 
    size_t startx;
};
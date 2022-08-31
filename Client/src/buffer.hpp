#pragma once
#include <string>
#include <deque>

struct buffer
{
    buffer(size_t win_width)
    : maxlen{win_width}
    {}

    std::deque<std::string> lines;

    void insert_char(char c, int nline, int ncol);

    void delete_char(int nline, int ncol);

    void insert_str(std::string const &str, int nline, int ncol);
    
    void insert_line(std::string const &line, int n);
    
    void append_line(std::string const &line);

    void remove_line(int n);

    void clear();

private:
    void align_lines(int start_line);

    size_t maxlen;
};
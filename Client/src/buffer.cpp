#include "buffer.hpp"

void buffer::insert_line(std::string const &line, int n)
{
    lines.insert(lines.begin()+n, line);
    align_lines(n);
}

void buffer::append_line(std::string const &line)
{
    lines.push_back(line);
    align_lines(lines.size() - 1);
}

void buffer::remove_line(int n)
{
    lines.erase(lines.begin()+n);
}

void buffer::insert_char(char c, int nline, int ncol)
{
    lines[nline].insert(ncol, 1, c);
    
    int n = nline;
    while (lines[n].length() >= maxlen)
    {
        c = lines[n].back();
        if (n+1 == lines.size())
        {
            lines.push_back(std::string(1, c));
        }
        else
        {
            lines[n+1].insert(0, 1, c);
        }
        lines[n].pop_back();
        ++n;
    }
}

void buffer::delete_char(int nline, int ncol)
{
    lines[nline].erase(ncol, 1);
    
    int n = nline;
    while ((n != lines.size() - 1) && (lines[n].length() < maxlen - 1))
    {
        char32_t c = lines[n+1][0];
        lines[n].push_back(c);
        lines[n+1].erase(0, 1);
        ++n;
    }
    if ((nline < lines.size() - 1) && (lines.back().empty()))
    {
        lines.pop_back();
    }
}

void buffer::insert_str(std::string const &str, int nline, int ncol)
{
    lines[nline].insert(ncol, str.c_str(), str.length());
    align_lines(nline);
}

void buffer::clear()
{
    lines.clear();
    lines.push_back("");
}

void buffer::align_lines(int start_line)
{
    int n = start_line;
    while (lines[n].length() >= maxlen)
    {
        std::string str = lines[n].substr(maxlen-1, lines[n].length() - maxlen + 1);
        if (n + 1 == lines.size())
        {
            lines.push_back(str);
        }
        else
        {
            lines[n+1].insert(0, str.c_str(), str.length());
        }
        lines[n].erase(maxlen-1, lines[n].length() - maxlen + 1);
        ++n;
    }
}
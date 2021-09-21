#pragma once

#include <string>

namespace EB
{
    template <typename T>
    class Matrix
    {
    private:
        T*     array_ptr;
        size_t rows;
        size_t cols;
        size_t total_elements;

        bool   is_row_index  = true;
        size_t index_row    = 0;
        size_t index_col    = 0;
        size_t access_count = 0;

        size_t get_converted_index()
        {
            return (this->get_col_count() * this->index_row) + this->index_col;
        }

    public:
        Matrix(size_t rows, size_t cols, T init_val) : rows(rows), cols(cols)
        {
            this->total_elements = rows * cols;
            this->array_ptr      = new T[this->total_elements];

            for(int i=0; i < this->total_elements; i++) this->array_ptr[i] = init_val;
        }

        ~Matrix()
        {
            delete[] this->array_ptr;
        }

        size_t get_row_count()
        {
            return this->rows;
        }

        size_t get_col_count()
        {
            return this->cols;
        }

        T* get_array_ptr()
        {
            return this->array_ptr;
        }

        std::string get_display_str(bool pretty=false)
        {
            size_t most_longest_number_str_len = 1;
            std::string  display_str           = "";

            if(!pretty) display_str += "[ ";

            if(pretty)
            {
                for(size_t i=0; i < this->total_elements; i++)
                {
                    size_t number_str_len = std::to_string(this->get_array_ptr()[i]).length();

                    if(number_str_len > most_longest_number_str_len)
                        most_longest_number_str_len = number_str_len;
                }
            }

            for(size_t row=0; row < this->get_row_count(); row++)
            {
                if(pretty) display_str += "| ";
                else       display_str += "[ ";

                for(size_t col=0; col < this->get_col_count(); col++ )
                {
                    T elem = (*this)[row][col].get();

                    std::string number_str = std::to_string(elem);
                    display_str += number_str;

                    if(pretty)
                    {
                        if(number_str.length() < most_longest_number_str_len)
                            for(int i=0; i < most_longest_number_str_len - number_str.length(); i++)
                                display_str += " ";
                    }
                    
                    if(col != this->get_col_count() - 1)
                    {
                        if(pretty) display_str += " ";
                        else       display_str += ", ";
                    }
                }

                if(pretty) display_str += " |";
                else       display_str += " ]";

                if(row != this->get_row_count() - 1)
                {
                    if(pretty) display_str += "\n";
                    else       display_str += ", ";
                }
            }

            if(pretty) display_str += "\n";

            if(!pretty) display_str += " ]";

            return display_str;
        }

        T get()
        {
            if(access_count != 2) throw std::range_error("");

            access_count = 0;

            return this->get_array_ptr()[this->get_converted_index()];
        }

        void set(T val)
        {
            if(access_count != 2) throw std::range_error("");

            access_count = 0;

            this->get_array_ptr()[this->get_converted_index()] = val;
        }

        Matrix& operator[](size_t index)
        {

            if(this->is_row_index)
            {
                if(index < 0 || index > this->get_row_count() - 1)
                    throw std::out_of_range("");
                
                this->index_row = index;
            }
            else
            {
                if(index < 0 || index > this->get_col_count() - 1)
                    throw std::out_of_range("");

                this->index_col = index;
            }

            this->is_row_index = !this->is_row_index;
            
            if(access_count < 2) access_count++;

            return *this;
        }

        void operator=(T val)
        {
            if(access_count != 2) throw std::range_error("");

            this->get_array_ptr()[this->get_converted_index()] = val;
        }
    };
}

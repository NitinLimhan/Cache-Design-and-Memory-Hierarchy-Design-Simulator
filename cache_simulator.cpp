#include<iostream>
#include<stdlib.h>
#include<fstream>
#include<vector>
#include<math.h>

using namespace std;

//Class Declaration
class generic_cache
{
    private://Data Declarations
        vector<string> input;

        //Variables for command-line arguments
        int block_size;
        int l1_size;
        int l1_associativity;
        int l2_size;
        int l2_associativity;
        int replacement_policy;
        int inclusion_property;
        string filename;

        //Variables for storing the number of bits in address fields
        int block_offset;
        int l1_sets;
        int l2_sets;
        int l1_index;
        int l2_index;
        int l1_tag;
        int l2_tag;

        //Variables for internal calculations
        string current_input;
        string current_address;
        string current_l1_tag;
        int current_l1_index_decimal;
        string current_l1_index_binary;
        string current_l2_tag;
        int current_l2_index_decimal;
        string current_l2_index_binary;
        bool read = 0;
        bool write = 0;
        int l1_counter = 1;
        int l2_counter = 1;

        //Variables for printing the outputs
        int l1_read = 0;
        int l1_read_misses = 0;
        int l1_write = 0;
        int l1_write_misses = 0;
        float l1_miss_rate = 0;
        int l1_write_backs = 0;
        int l2_read = 0;
        int l2_read_misses = 0;
        int l2_write = 0;
        int l2_write_misses = 0;
        float l2_miss_rate = 0;
        int l2_write_backs = 0;
        int invalidation_inclusive = 0;
        int total_memory_traffic = 0;

        //Variables for storing the different caches, indexes, dirty bit and lru order
        vector<vector<string>> l1_cache;
        vector<vector<string>> l1_indexes;
        vector<vector<string>> l1_dirty;
        vector<vector<int>> l1_lru;
        vector<vector<string>> l2_cache;
        vector<vector<string>> l2_indexes;
        vector<vector<string>> l2_dirty;
        vector<vector<int>> l2_lru;

    public://Functions of the class
        generic_cache(int bsize, int l1size, int l1assoc, int l2size, int l2assoc, int repl_policy, int incl_property, string fname);
        void read_file(string filename);
        string hexadecimal_to_binary(string str1);
        int binary_to_decimal(string str1);
        string binary_to_hexadecimal(string str1);
        void execute_replacement_policy();
        void lru_policy();
        void pseudo_lru_policy();
        void optimal_policy();
        void print_outputs();
};

//Constructor of the class to create caches
generic_cache::generic_cache(int bsize, int l1size, int l1assoc, int l2size, int l2assoc, int repl_policy, int incl_property, string fname)
{
    //Initialising command-line arguments
    block_size = bsize;
    l1_size = l1size;
    l1_associativity = l1assoc;
    l2_size = l2size;
    l2_associativity = l2assoc;
    replacement_policy = repl_policy;
    inclusion_property = incl_property;
    filename = fname;

    //Calculating different fields of address fields
    block_offset = log2(block_size);
    l1_sets = (l1_size/(l1_associativity*block_size));
    l1_index = log2(l1_sets);
    l1_tag = 32 - l1_index - block_offset;

    if (l2_size != 0)
    {
        l2_sets = (l2_size/(l2_associativity*block_size));
        l2_index = log2(l2_sets);
        l2_tag = 32 - l2_index - block_offset;           
    }
    else
    {
        l2_index = 0;
        l2_tag = 0;
    }

    //Resizing the vectors according to above calculations
    l1_cache.resize(l1_sets);
    l1_indexes.resize(l1_sets);
    l1_dirty.resize(l1_sets);
    l1_lru.resize(l1_sets);
    for (int i = 0; i < l1_sets; ++i)
    {
        l1_cache[i].resize(l1_associativity);
        l1_indexes[i].resize(l1_associativity);
        l1_dirty[i].resize(l1_associativity);
        l1_lru[i].resize(l1_associativity);
    }

    if (l2_size != 0)
    {
        l2_cache.resize(l2_sets);
        l2_indexes.resize(l2_sets);
        l2_dirty.resize(l2_sets);
        l2_lru.resize(l2_sets);
        for (int i = 0; i < l2_sets; ++i)
        {
            l2_cache[i].resize(l2_associativity);
            l2_indexes[i].resize(l2_associativity);
            l2_dirty[i].resize(l2_associativity);
            l2_lru[i].resize(l2_associativity);
        }               
    }
}

//Function to read data from the trace files
void generic_cache::read_file(string filename)
{
    string instruction;
    ifstream ipfile(filename.c_str());
    
    if(ipfile.is_open())
    {
        while(!ipfile.eof())
        {
            getline(ipfile,instruction);
            input.push_back(instruction);
        }
        ipfile.close();
    }
    else
        cout<<"File not found or unable to open file";
}

//Function to convert hexadecimal value to binary value
string generic_cache::hexadecimal_to_binary(string str1)
{
    int n = 8 - str1.length();
    string str2 = "";

    if(str1.length() != 8)
    {
        if (n != 0)
        {
            for (int i = 0; i < n; i++)
            {
                 str1 = "0" + str1;
            } 
        }
    }

    for (int i = 0; i < 8; i++)
    {
        switch (str1[i])
        {
        case '0' :
            str2 = str2 + "0000";
            break;
        case '1' :
            str2 = str2 + "0001";
            break;
        case '2' :
            str2 = str2 + "0010";
            break;
        case '3' :
            str2 = str2 + "0011";
            break;
        case '4' :
            str2 = str2 + "0100";
            break;
        case '5' :
            str2 = str2 + "0101";
            break;
        case '6' :
            str2 = str2 + "0110";
            break;
        case '7' :
            str2 = str2 + "0111";
            break;
        case '8' :
            str2 = str2 + "1000";
            break;
        case '9' :
            str2 = str2 + "1001";
            break;
        case 'a' :
            str2 = str2 + "1010";
            break;
        case 'A' :
            str2 = str2 + "1010";
            break;
        case 'b' :
            str2 = str2 + "1011";
            break;
        case 'B' :
            str2 = str2 + "1011";
            break;
        case 'c' :
            str2 = str2 + "1100";
            break;
        case 'C' :
            str2 = str2 + "1100";
            break;
        case 'd' :
            str2 = str2 + "1101";
            break;
        case 'D' :
            str2 = str2 + "1101";
            break;
        case 'e' :
            str2 = str2 + "1110";
            break;
        case 'E' :
            str2 = str2 + "1110";
            break;
        case 'f' :
            str2 = str2 + "1111";
            break;
        case 'F' :
            str2 = str2 + "1111";
            break;
        default:
            cout<<"Invalid Hexadecimal Digit";
            break;
        }
    }
    return str2;
}

//Function to convert binary value to decimal value
int generic_cache::binary_to_decimal(string str1)
{
    int n = str1.length();
    int num = 0;

    for (int i = n - 1, power = 1; i >= 0; i--, power *= 2)
    {
        if (str1[i] == '1')
        {
            num += power;
        }
    }
    return num;
}

//Function to convert binary value to hexadecimal value
string generic_cache::binary_to_hexadecimal(string str1)
{
    /*int n = str1.length();
    int num = 0;

    for (int i = n - 1, power = 1; i >= 0; i--, power *= 2)
    {
        if (str1[i] == '1')
        {
            num += power;
        }
    }*/

    int num = generic_cache::binary_to_decimal(str1);

    int i = 0;
    string str2 = "";
    char hex_char[20];

    while(num != 0)
    {
        int remainder = 0;
        remainder = num % 16;

        if (remainder < 10)
        {
            hex_char[i] = remainder + 48;
            i++;
        }
        else
        {
            hex_char[i] = remainder + 55;
            i++;
        }
        num = num / 16;
    }

    for (int j = i - 1; j >= 0; j--)
    {
        str2 = str2 + hex_char[j];
        //cout<<str2<<endl;
    }

    //cout<<str2;
    return str2;
}

//Function to choose the replacement policy
void generic_cache::execute_replacement_policy()
{
    switch (replacement_policy)
    {
    case 0 :
        for (int i = 0; i < input.size(); i++)
        {
            if (input[i] != "")
            {
                current_input = input[i];
                lru_policy();
            }
        }
        print_outputs();            
        break;
    case 1 :
        for (int i = 0; i < input.size(); i++)
        {
            if (input[i] != "")
            {
                current_input = input[i];
                pseudo_lru_policy();
            }
        }
        print_outputs();            
        break;
    case 2 :
        for (int i = 0; i < input.size(); i++)
        {
            if (input[i] != "")
            {
                current_input = input[i];
                optimal_policy();
            }
        }
        print_outputs();            
        break;        
    default:
        cout<<"Invalid Replacement Policy";
        break;
    }
}

//Function to execute/simulate the lru policy
void generic_cache::lru_policy()
{
    //Full address
    current_address = generic_cache::hexadecimal_to_binary(current_input.substr(2));

    //Calculating respective address fields for L1
    current_l1_tag = generic_cache::binary_to_hexadecimal(current_address.substr(0, l1_tag));
    current_l1_index_binary = current_address.substr(l1_tag, l1_index);
    current_l1_index_decimal = generic_cache::binary_to_decimal(current_l1_index_binary);

    //Calculating respective address fields for L2
    if (l2_size != 0)
    {
        current_l2_tag = generic_cache::binary_to_hexadecimal(current_address.substr(0, l2_tag));
        current_l2_index_binary = current_address.substr(l2_tag, l2_index);
        current_l2_index_decimal = generic_cache::binary_to_decimal(current_l2_index_binary);
    }
    
    //Checking for read or write instructions
    switch (current_input[0])
    {
    case 'r':
        read = 1;
        break;
    case 'w':
        write = 1;
        break;    
    default:
        cout<<"Invalid Operation";
        break;
    }

    int i, j, l1, l2, temp1, temp2;

    if (read == 1)
    {
        //If there is only an L1 cache
        if (l2_size == 0)
        {
            l1_read++;

            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    read = 0;                
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l1_read_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                }
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                read = 0;
                l1_counter++;
            }
        }

        //If there are L1 and L2 caches
        if (l2_size != 0)
        {
            l1_read++;

            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    read = 0;                
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l2_read++;
                l1_read_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                    l2_write++;
                }
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                read = 0;
                l1_counter++;
            }

            for (j = 0; j < l2_associativity; j++)
            {
                if (l2_indexes[current_l2_index_decimal][j] == current_address.substr(0, l2_tag))
                {
                    l2_lru[current_l2_index_decimal][j] = l2_counter;
                    read = 0;
                    l2_counter++;
                    break;
                }
            }

            if (j == l2_associativity)
            {
                l2_read_misses++;

                l2 = 0;
                temp1 = l2_lru[current_l2_index_decimal][l2];
                temp2 = l2;

                if (temp1 != 0)
                {
                    for (l2 = 1; l2 < l2_associativity; l2++)
                    {
                        if (l2_lru[current_l2_index_decimal][l2] == 0)
                        {
                            temp2 = l2;
                            break;
                        }
                        if (l2_lru[current_l2_index_decimal][l2] > 0 && l2_lru[current_l2_index_decimal][l2] < temp1)
                        {
                            temp1 = l2_lru[current_l2_index_decimal][l2];
                            temp2 = l2;
                        }
                    }
                }

                if (inclusion_property == 1)
                {
                    for (i = 0; i < l1_sets; i++)
                    {
                        for (j = 0; j < l1_associativity; j++)
                        {
                            if (l1_indexes[i][j] == l2_indexes[current_l2_index_decimal][temp2])
                            {
                                if (l1_cache[i][j] == "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;
                                    l1_write_backs++;
                                    invalidation_inclusive++;
                                }
                                if (l1_cache[i][j] != "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;                                
                                }                      
                            }
                        }
                    }
                }
                
                l2_cache[current_l2_index_decimal][temp2] = current_l2_tag;
                l2_indexes[current_l2_index_decimal][temp2] = current_address.substr(0, l2_tag);
                if (l2_dirty[current_l2_index_decimal][temp2] == "D")
                {
                    l2_dirty[current_l2_index_decimal][temp2] = "";
                    l2_write_backs++;
                }
                l2_lru[current_l2_index_decimal][temp2] = l2_counter;
                l2_counter++;
                read = 0;
            }                        
        }
   
    }
    else
    {
        //If there is only an L1 cache
        if (l2_size == 0)
        {
            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_write++;

                    l1_cache[current_l1_index_decimal][i] = current_l1_tag;
                    l1_dirty[current_l1_index_decimal][i] = "D";
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    write = 0;
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l1_write_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_write++;
                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                }
                l1_dirty[current_l1_index_decimal][temp2] = "D";
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                write = 0;
                l1_counter++;
            }
        }

        //If there are L1 and L2 caches
        if (l2_size != 0)
        {
            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_write++;
                    //l1_write_backs++;

                    l1_cache[current_l1_index_decimal][i] = current_l1_tag;
                    l1_dirty[current_l1_index_decimal][i] = "D";
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    write = 0;
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l2_read++;
                l1_write_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_write++;
                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                    l2_write++;
                }
                l1_dirty[current_l1_index_decimal][temp2] = "D";
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                write = 0;
                l1_counter++;
            }

            for (j = 0; j < l2_associativity; j++)
            {
                if (l2_indexes[current_l2_index_decimal][j] == current_address.substr(0, l2_tag))
                {
                    if (l2_dirty[current_l2_index_decimal][j] == "D")
                    {
                        l2_dirty[current_l2_index_decimal][j] = "";
                        l2_write_backs++;
                    }

                    l2_cache[current_l2_index_decimal][j] = current_l2_tag;
                    l2_dirty[current_l1_index_decimal][j] = "D";
                    l2_lru[current_l2_index_decimal][j] = l2_counter;
                    write = 0;
                    l2_counter++;
                    break;
                }
            }            
        
            if (j == l2_associativity)
            {
                l2_read_misses++;

                l2 = 0;
                temp1 = l2_lru[current_l2_index_decimal][l2];
                temp2 = l2;

                if (temp1 != 0)
                {
                    for (l2 = 1; l2 < l2_associativity; l2++)
                    {
                        if (l2_lru[current_l2_index_decimal][l2] == 0)
                        {
                            temp2 = l2;
                            break;
                        }
                        if (l2_lru[current_l2_index_decimal][l2] > 0 && l2_lru[current_l2_index_decimal][l2] < temp1)
                        {
                            temp1 = l2_lru[current_l2_index_decimal][l2];
                            temp2 = l2;
                        }
                    }
                }

                if (inclusion_property == 1)
                {
                    for (i = 0; i < l1_sets; i++)
                    {
                        for (j = 0; j < l1_associativity; j++)
                        {
                            if (l1_indexes[i][j] == l2_indexes[current_l2_index_decimal][temp2])
                            {
                                if (l1_cache[i][j] == "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;
                                    l1_write_backs++;
                                    invalidation_inclusive++;
                                }
                                if (l1_cache[i][j] != "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;                                
                                }                      
                            }
                        }
                    }
                }
                
                l2_cache[current_l2_index_decimal][temp2] = current_l2_tag;
                l2_indexes[current_l2_index_decimal][temp2] = current_address.substr(0, l2_tag);
                if (l2_dirty[current_l2_index_decimal][temp2] == "D")
                {
                    l2_dirty[current_l2_index_decimal][temp2] = "";
                    l2_write_backs++;
                }
                l2_lru[current_l2_index_decimal][temp2] = l2_counter;
                l2_counter++;
                l2_dirty[current_l2_index_decimal][temp2] = "";
                write = 0;
            }
        }  
    }
}

////Function to execute/simulate the lru policy
void generic_cache::pseudo_lru_policy()
{
    //Full address
    current_address = generic_cache::hexadecimal_to_binary(current_input.substr(2));

    //Calculating respective address fields for L1
    current_l1_tag = generic_cache::binary_to_hexadecimal(current_address.substr(0, l1_tag));
    current_l1_index_binary = current_address.substr(l1_tag, l1_index);
    current_l1_index_decimal = generic_cache::binary_to_decimal(current_l1_index_binary);

    //Calculating respective address fields for L2
    if (l2_size != 0)
    {
        current_l2_tag = generic_cache::binary_to_hexadecimal(current_address.substr(0, l2_tag));
        current_l2_index_binary = current_address.substr(l2_tag, l2_index);
        current_l2_index_decimal = generic_cache::binary_to_decimal(current_l2_index_binary);
    }
    
    //Checking for read or write instructions
    switch (current_input[0])
    {
    case 'r':
        read = 1;
        break;
    case 'w':
        write = 1;
        break;    
    default:
        cout<<"Invalid Operation";
        break;
    }

    int i, j, l1, l2, temp1, temp2;

    if (read == 1)
    {
        //If there is only an L1 cache
        if (l2_size == 0)
        {
            l1_read++;

            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    read = 0;                
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l1_read_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                }
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                read = 0;
                l1_counter++;
            }
        }

        //If there are L1 and L2 caches
        if (l2_size != 0)
        {
            l1_read++;

            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    read = 0;                
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l2_read++;
                l1_read_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                    l2_write++;
                }
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                read = 0;
                l1_counter++;
            }

            for (j = 0; j < l2_associativity; j++)
            {
                if (l2_indexes[current_l2_index_decimal][j] == current_address.substr(0, l2_tag))
                {
                    l2_lru[current_l2_index_decimal][j] = l2_counter;
                    read = 0;
                    l2_counter++;
                    break;
                }
            }

            if (j == l2_associativity)
            {
                l2_read_misses++;

                l2 = 0;
                temp1 = l2_lru[current_l2_index_decimal][l2];
                temp2 = l2;

                if (temp1 != 0)
                {
                    for (l2 = 1; l2 < l2_associativity; l2++)
                    {
                        if (l2_lru[current_l2_index_decimal][l2] == 0)
                        {
                            temp2 = l2;
                            break;
                        }
                        if (l2_lru[current_l2_index_decimal][l2] > 0 && l2_lru[current_l2_index_decimal][l2] < temp1)
                        {
                            temp1 = l2_lru[current_l2_index_decimal][l2];
                            temp2 = l2;
                        }
                    }
                }

                if (inclusion_property == 1)
                {
                    for (i = 0; i < l1_sets; i++)
                    {
                        for (j = 0; j < l1_associativity; j++)
                        {
                            if (l1_indexes[i][j] == l2_indexes[current_l2_index_decimal][temp2])
                            {
                                if (l1_cache[i][j] == "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;
                                    l1_write_backs++;
                                    invalidation_inclusive++;
                                }
                                if (l1_cache[i][j] != "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;                                
                                }                      
                            }
                        }
                    }
                }
                
                l2_cache[current_l2_index_decimal][temp2] = current_l2_tag;
                l2_indexes[current_l2_index_decimal][temp2] = current_address.substr(0, l2_tag);
                if (l2_dirty[current_l2_index_decimal][temp2] == "D")
                {
                    l2_dirty[current_l2_index_decimal][temp2] = "";
                    l2_write_backs++;
                }
                l2_lru[current_l2_index_decimal][temp2] = l2_counter;
                l2_counter++;
                read = 0;
            }                        
        }
   
    }
    else
    {
        //If there is only an L1 cache
        if (l2_size == 0)
        {
            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_write++;

                    l1_cache[current_l1_index_decimal][i] = current_l1_tag;
                    l1_dirty[current_l1_index_decimal][i] = "D";
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    write = 0;
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l1_write_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_write++;
                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                }
                l1_dirty[current_l1_index_decimal][temp2] = "D";
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                write = 0;
                l1_counter++;
            }
        }

        //If there are L1 and L2 caches
        if (l2_size != 0)
        {
            for (i = 0; i < l1_associativity; i++)
            {
                if (l1_indexes[current_l1_index_decimal][i] == current_address.substr(0, l1_tag))
                {
                    l1_write++;
                    //l1_write_backs++;

                    l1_cache[current_l1_index_decimal][i] = current_l1_tag;
                    l1_dirty[current_l1_index_decimal][i] = "D";
                    l1_lru[current_l1_index_decimal][i] = l1_counter;
                    write = 0;
                    l1_counter++;
                    break;
                }
            }

            if (i == l1_associativity)
            {
                l2_read++;
                l1_write_misses++;

                l1 = 0;
                temp1 = l1_lru[current_l1_index_decimal][l1];
                temp2 = l1;

                if (temp1 != 0)
                {
                    for (l1 = 1; l1 < l1_associativity; l1++)
                    {
                        if (l1_lru[current_l1_index_decimal][l1] == 0)
                        {
                            temp2 = l1;
                            break;
                        }
                        if (l1_lru[current_l1_index_decimal][l1] > 0 && l1_lru[current_l1_index_decimal][l1] < temp1)
                        {
                            temp1 = l1_lru[current_l1_index_decimal][l1];
                            temp2 = l1;
                        }
                    }
                }

                l1_write++;
                l1_cache[current_l1_index_decimal][temp2] = current_l1_tag;
                l1_indexes[current_l1_index_decimal][temp2] = current_address.substr(0, l1_tag);
                if (l1_dirty[current_l1_index_decimal][temp2] == "D")
                {
                    l1_dirty[current_l1_index_decimal][temp2] = "";
                    l1_write_backs++;
                    l2_write++;
                }
                l1_dirty[current_l1_index_decimal][temp2] = "D";
                l1_lru[current_l1_index_decimal][temp2] = l1_counter;
                write = 0;
                l1_counter++;
            }

            for (j = 0; j < l2_associativity; j++)
            {
                if (l2_indexes[current_l2_index_decimal][j] == current_address.substr(0, l2_tag))
                {
                    if (l2_dirty[current_l2_index_decimal][j] == "D")
                    {
                        l2_dirty[current_l2_index_decimal][j] = "";
                        l2_write_backs++;
                    }

                    l2_cache[current_l2_index_decimal][j] = current_l2_tag;
                    l2_dirty[current_l1_index_decimal][j] = "D";
                    l2_lru[current_l2_index_decimal][j] = l2_counter;
                    write = 0;
                    l2_counter++;
                    break;
                }
            }            
        
            if (j == l2_associativity)
            {
                l2_read_misses++;

                l2 = 0;
                temp1 = l2_lru[current_l2_index_decimal][l2];
                temp2 = l2;

                if (temp1 != 0)
                {
                    for (l2 = 1; l2 < l2_associativity; l2++)
                    {
                        if (l2_lru[current_l2_index_decimal][l2] == 0)
                        {
                            temp2 = l2;
                            break;
                        }
                        if (l2_lru[current_l2_index_decimal][l2] > 0 && l2_lru[current_l2_index_decimal][l2] < temp1)
                        {
                            temp1 = l2_lru[current_l2_index_decimal][l2];
                            temp2 = l2;
                        }
                    }
                }

                if (inclusion_property == 1)
                {
                    for (i = 0; i < l1_sets; i++)
                    {
                        for (j = 0; j < l1_associativity; j++)
                        {
                            if (l1_indexes[i][j] == l2_indexes[current_l2_index_decimal][temp2])
                            {
                                if (l1_cache[i][j] == "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;
                                    l1_write_backs++;
                                    invalidation_inclusive++;
                                }
                                if (l1_cache[i][j] != "D")
                                {
                                    l1_indexes[i][j] = "";
                                    l1_dirty[i][j] = "";
                                    l1_lru[i][j] = 0;                                
                                }                      
                            }
                        }
                    }
                }
                
                l2_cache[current_l2_index_decimal][temp2] = current_l2_tag;
                l2_indexes[current_l2_index_decimal][temp2] = current_address.substr(0, l2_tag);
                if (l2_dirty[current_l2_index_decimal][temp2] == "D")
                {
                    l2_dirty[current_l2_index_decimal][temp2] = "";
                    l2_write_backs++;
                }
                l2_lru[current_l2_index_decimal][temp2] = l2_counter;
                l2_counter++;
                l2_dirty[current_l2_index_decimal][temp2] = "";
                write = 0;
            }
        }  
    }
}

//Function to execute/simulate the lru policy
void generic_cache::optimal_policy()
{

}

//Function to print the simulation outputs
void generic_cache::print_outputs()
{
    cout<<endl<<"===== Simulator configuration =====";
    cout<<endl<<"BLOCKSIZE:             "<<block_size;
    cout<<endl<<"L1_SIZE:               "<<l1_size;
    cout<<endl<<"L1_ASSOC:              "<<l1_associativity;
    cout<<endl<<"L2_SIZE:               "<<l2_size;
    cout<<endl<<"L2_ASSOC:              "<<l2_associativity;

    switch (replacement_policy)
    {
    case 0 :
        cout<<endl<<"REPLACEMENT POLICY:    LRU";
        break;
    case 1 :
        cout<<endl<<"REPLACEMENT POLICY:    Pseudo-LRU";
        break;
    case 2 :
        cout<<endl<<"REPLACEMENT POLICY:    Optimal";
        break;
    default:
        cout<<"Invalid Replacement Policy";
        break;
    }

    switch (inclusion_property)
    {
    case 0 :
        cout<<endl<<"INCLUSION PROPERTY:    non-inclusive";
        break;
    case 1 :
        cout<<endl<<"INCLUSION PROPERTY:    inclusive";
        break;
    default:
        cout<<"Invalid Inclusion Property";
        break;
    }

    cout<<endl<<"trace_file:            "<<filename;

    cout<<endl<<"===== L1 contents =====";
    for (int i = 0; i < l1_sets; i++)
    {
        cout<<endl<<"Set     "<<i<<":   ";
        for (int j = 0; j < l1_associativity; j++)
        {
            cout<<l1_cache[i][j]<<" "<<l1_dirty[i][j]<<"    ";
        }
    }

    if (l2_size != 0)
    {
        cout<<endl<<"===== L2 contents =====";
        for (int i = 0; i < l2_sets; i++)
        {
            cout<<endl<<"Set     "<<i<<":   ";
            for (int j = 0; j < l2_associativity; j++)
            {
                cout<<l2_cache[i][j]<<" "<<l2_dirty[i][j]<<"    ";
            }
        }
    }

    l1_miss_rate = ((float)l1_read_misses + (float)l1_write_misses)/((float)l1_read + (float)l1_write);
    if (l2_size != 0)
    {
        l2_miss_rate = ((float)l2_read_misses + (float)l2_write_misses)/((float)l2_read + (float)l2_write);
        if (inclusion_property == 1)
        {
            total_memory_traffic = l2_read_misses + l2_write_misses + l2_write_backs + invalidation_inclusive++;
        }
        else
        {
            total_memory_traffic = l2_read_misses + l2_write_misses + l2_write_backs;
        }
    }
    else
    {
        l2_miss_rate = 0;
        total_memory_traffic = l1_read_misses + l1_write_misses + l1_write_backs;
    }
    
    cout<<endl<<"===== Simulation results (raw) =====";
    cout<<endl<<"a. number of L1 reads:        "<<l1_read;
    cout<<endl<<"b. number of L1 read misses:  "<<l1_read_misses;
    cout<<endl<<"c. number of L1 writes:       "<<l1_write;
    cout<<endl<<"d. number of L1 write misses: "<<l1_write_misses;
    cout<<endl<<"e. L1 miss rate:              "<<l1_miss_rate;
    cout<<endl<<"f. number of L1 writebacks:   "<<l1_write_backs;
    cout<<endl<<"g. number of L2 reads:        "<<l2_read;
    cout<<endl<<"h. number of L2 read misses:  "<<l2_read_misses;
    cout<<endl<<"i. number of L2 writes:       "<<l2_write;
    cout<<endl<<"j. number of L2 write misses: "<<l2_write_misses;
    cout<<endl<<"k. L2 miss rate:              "<<l2_miss_rate;
    cout<<endl<<"l. number of L2 writebacks:   "<<l2_write_backs;
    cout<<endl<<"m. total memory traffic:      "<<total_memory_traffic;
}

//Main function
int main(int argc, char*argv[])
{
    generic_cache fcmh(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), argv[8]);
    fcmh.read_file(argv[8]);
    fcmh.execute_replacement_policy();
    return 0;
}
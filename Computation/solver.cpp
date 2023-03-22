#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <map>

std::pair<std::map<std::string, long long>, long long> read_set(std::ifstream& file) {
    /* Function to read input/output from stream
     * In:
     * file - stream
     * Out:
     * (map: from names to amount; sum of cash)
     */

    // Initializing variables
    std::map<std::string, long long> dict;
    long long num, sum = 0;
    std::string line, buff_line;
    size_t sz;

    // Reading size
    std::getline(file, line);
    size_t pos = line.find(':');
    if (pos == std::string::npos) {
        throw std::invalid_argument("Wrong file format");
    }
    sz = std::stoull(line.substr(pos + 1));

    for (size_t i = 0; i < sz; ++i) {
        // Reading people
        std::getline(file, buff_line);
        pos = buff_line.find(':');
        num = std::stoll(buff_line.substr(pos + 1));
        dict[buff_line.substr(0, pos)] += num;
        sum += num;
    }

    return std::make_pair(dict, sum);
}

long long get_input(std::vector<std::pair<std::string, long long>>& input_arr,
               std::vector<std::pair<std::string, long long>>& output_arr,
               std::ifstream& data_file) {
    /* Function to read all data from the stream
     *
     * In:
     * input_arr - input cash var - changes
     * output_arr - output cash var - changes
     * data_file - stream
     * Out:
     * C - commission
     */

    // Reading sets
    auto in_d = read_set(data_file);
    auto out_d = read_set(data_file);

    // Calculating commission
    long long C = in_d.second - out_d.second;

    // Checking for output wallets
    for (auto & it : in_d.first) {
        long long sz;
        if (out_d.first.find(it.first) != out_d.first.end()) {
            sz = it.second - out_d.first[it.first];

            if (sz >= 0) {
                out_d.first.erase(it.first);
            } else {
                out_d.first[it.first] = -sz;
            }
        } else {
            sz = it.second;
        }

        if ((sz > 0) && (sz < C)) {
            C -= sz;
            sz = 0;
        }
        if (sz > 0) {
            input_arr.emplace_back(std::make_pair(it.first, sz));
        }
    }

    // Writing input wallets
    for (auto & it : out_d.first) {
        output_arr.emplace_back(std::make_pair(it.first, it.second));
    }

    return C;
}

// Comparator for sort
bool el_comparator(std::pair<long long,std::vector<bool>>& a1, std::pair<long long,std::vector<bool>>& a2) {
    return a1.first < a2.first;
}

// Class for line representation
class Set_line {
public:
    // Class constructor
    explicit Set_line(std::vector<std::pair<std::string, long long>>& arr) {
        pos = 0;
        step = 1;
        max_sz = arr.size();

        for (auto & it: arr) {
            values.emplace_back(it.second);
            names.emplace_back(it.first);
        }

        std::vector<bool> bit_ar(arr.size(), false);
        base_ans = std::make_pair(0LL, bit_ar);

        // calculating line for the first step
        for (int i = 0; i < arr.size(); ++i) {
            bit_ar[i] = true;
            line.emplace_back(std::make_pair(values[i], bit_ar));
            next.emplace_back(std::make_pair(values[i], bit_ar));
            bit_ar[i] = false;
        }

        std::sort(line.begin(), line.end(), el_comparator);
    }


    // increase sets in line
    void increase_size() {
        pos = 0;
        if (step < max_sz) {
            ++step;
            std::vector<std::pair<long long,std::vector<bool>>> new_next;
            for (auto & it: next) {
                auto fin = std::find(it.second.rbegin(), it.second.rend(), true);
                size_t l_pos = values.size() - (fin - it.second.rbegin() + 1) + 1;
                while (l_pos < values.size()) {
                    it.second[l_pos] = true;
                    line.emplace_back(std::make_pair(it.first + values[l_pos], it.second));
                    if (!it.second[it.second.size()-1]) {
                        new_next.emplace_back(std::make_pair(it.first + values[l_pos], it.second));
                    }
                    it.second[l_pos] = false;
                    ++l_pos;
                }
            }
            std::sort(line.begin(), line.end(), el_comparator);
            next = new_next;
        }
    };

    // Move in line element
    auto next_el() {
        if (pos + 1 < line.size()) {
            pos += 1;
            return line[pos];
        }
        return base_ans;
    }

    // Get next element
    auto get_next() {
        if (pos + 1 < line.size()) {
            return line[pos + 1];
        }
        return base_ans;
    }

    // Get current element
    auto now_el() {
        if (pos < line.size()) {
            return line[pos];
        }
        return base_ans;
    }

    // Get size of set
    [[nodiscard]] size_t size() const {
        return max_sz;
    }

    // Get name of user
    [[nodiscard]] auto get_name(size_t i) const {
        return names[i];
    }

private:
    /* pos - position in line.
     * line - line for algorithm
     * next - elements for next step of algo
     * values - elements values
     * names - elements names
     * base_ans - 'end' (empty) element
     */
    size_t pos, step, max_sz;
    std::vector<std::pair<long long,std::vector<bool>>> line, next;
    std::vector<long long> values;
    std::vector<std::string> names;
    std::pair<long long,std::vector<bool>> base_ans;
};


void verdict_amb(std::ofstream& output, std::pair<std::vector<bool>, std::vector<bool>>& ver1,
                 std::pair<std::vector<bool>, std::vector<bool>>& ver2,
                 Set_line& first, Set_line& second) {
    /*Printing verdict ambiguous to file
     *
     * ver1, ver2 - answer versions
     *
     * first, second - element lines
     */
    output << "# VERDICT AMBIGUOUS" << std::endl;
    output << "VERSION 1\n";
    output << "IN\n";
    for (int i = 0; i < ver1.first.size(); ++i) {
        if (ver1.first[i]) {
            output << first.get_name(i) << "\n";
        }
    }
    output << "OUT\n";
    for (int i = 0; i < ver1.second.size(); ++i) {
        if (ver1.second[i]) {
            output << second.get_name(i) << "\n";
        }
    }
    output << "VERSION 2\n";
    output << "IN\n";
    for (int i = 0; i < ver2.first.size(); ++i) {
        if (ver2.first[i]) {
            output << first.get_name(i) << "\n";
        }
    }
    output << "OUT\n";
    for (int i = 0; i < ver2.second.size(); ++i) {
        if (ver2.second[i]) {
            output << second.get_name(i) << "\n";
        }
    }
}

void simple_verdict(std::ofstream& output, Set_line& first, Set_line& second) {
    /*Printing verdict to file
     *
     * first, second - element lines
     */
    output << "# NON-SHARED" << std::endl;
    size_t counter = 0;
    output <<"\nIN\n";
    ++counter;
    for (int i = 0; i < first.size(); ++i) {
        output << first.get_name(i) << "\n";
    }
    output << "OUT\n";
    for (int i = 0; i < second.size(); ++i) {
        output << second.get_name(i) << "\n";
    }

}

void true_verdict(std::ofstream& output, std::vector<std::pair<std::vector<bool>, std::vector<bool>>>& match_pairs,
                  Set_line& first, Set_line& second) {
    /*Printing verdict to file
     *
     * match_pairs - answer version
     *
     * first, second - element lines
     */
    if (match_pairs.size() == 1) {
    	output << "# VERDICT SIMPLE" << std::endl;
    } else {
    	output << "# VERDICT SEPARABLE" << std::endl;
    }
    
    size_t counter = 0;
    for (auto& it: match_pairs) {
        output << "GROUP " << counter <<"\nIN\n";
        ++counter;
        for (int i = 0; i < it.first.size(); ++i) {
            if (it.first[i]) {
                output << first.get_name(i) << "\n";
            }
        }
        output << "OUT\n";
        for (int i = 0; i < it.second.size(); ++i) {
            if (it.second[i]) {
                output << second.get_name(i) << "\n";
            }
        }
    }
}

// Check if one set is in another
bool check_in(std::vector<bool>& a, std::vector<bool>& b) {
    for (int i = 0; i < a.size(); ++i) {
        if (a[i] and (not b[i])) {
            return false;
        }
    }
    return true;
}
// Check set intersection
bool check_inter(std::vector<bool>& a, std::vector<bool>& b) {
    for (int i = 0; i < a.size(); ++i) {
        if (a[i] and  b[i]) {
            return true;
        }
    }
    return false;
}

// Print time out
void time_out(std::ofstream& output) {
    output << "# TIME LIMIT EXTENDED" << std::endl;
}

int check_transaction(std::ifstream& input_file, std::ofstream& output_file, long long& time) {
    // Main function, checks one transaction from input_stream and writes answer to second stream
    // time - max time for the check (in seconds)

    std::string buff;
    std::getline(input_file, buff);
    output_file << buff << std::endl;


    // Creating timer for
    
    std::time_t start = std::time(nullptr);

    // Getting inputs
    std::vector<std::pair<std::string, long long>> input_ar, output_ar;
    long long C;
    C = get_input(input_ar, output_ar, input_file);

    // Setting lines
    Set_line line_in(input_ar);
    Set_line line_out(output_ar);
    if (line_in.size() == 1 or line_out.size() == 1) {
        simple_verdict(output_file, line_in, line_out);
        return 0;
    }

    size_t full_steps = std::max(line_in.size(), line_out.size());

    std::vector<std::pair<std::vector<bool>, std::vector<bool>>> match_pairs;
    for (int i = 0; i < full_steps; ++i) {
        match_pairs.clear();
        // get elements for the cycle
        auto el_in = line_in.now_el();
        auto el_out = line_out.now_el();
        while (el_in.first != 0 && el_out.first != 0) {
            // Check timeout
            if (std::time(nullptr) - start > time) {
                time_out(output_file);
                return 1;
            }

            // Check if there is the distance
            if (el_in.first < el_out.first) {
                el_in = line_in.next_el();
                continue;
            }
            if (el_in.first > el_out.first + C) {
                el_out = line_out.next_el();
                continue;
            }

            // Check intersections
            if ((line_in.get_next().first == 0) || (line_in.get_next().first > el_out.first + C)) {
                if ((line_out.get_next().first == 0) || (line_out.get_next().first > el_in.first)) {
                    match_pairs.emplace_back(std::make_pair(el_in.second, el_out.second));
                    el_in = line_in.next_el();
                    el_out = line_out.next_el();
                } else {
                    std::pair<std::vector<bool>, std::vector<bool>> l1 =
                            std::make_pair(line_in.now_el().second, line_out.now_el().second);
                    std::pair<std::vector<bool>, std::vector<bool>> l2 =
                            std::make_pair(line_in.now_el().second, line_out.next_el().second);
                    verdict_amb(output_file, l1, l2, line_in, line_out);

                    return 1;
                }
            } else {
                std::pair<std::vector<bool>, std::vector<bool>> l1 =
                        std::make_pair(line_in.now_el().second, line_out.now_el().second);
                std::pair<std::vector<bool>, std::vector<bool>> l2 =
                        std::make_pair(line_in.next_el().second, line_out.now_el().second);
                verdict_amb(output_file, l1, l2, line_in, line_out);
                return 1;
            }

        }

        line_in.increase_size();
        line_out.increase_size();
    }

    // find minimal pairs
    std::vector<std::pair<std::vector<bool>, std::vector<bool>>> ans_match;
    for (int i = 0; i < match_pairs.size(); ++i) {
        bool flag = false;
        for (int j = 0; j < match_pairs.size(); ++j) {
            if ((i != j) and check_in(match_pairs[j].first, match_pairs[i].first) and
            check_in(match_pairs[j].second, match_pairs[i].second)) {
                flag = true;
            }
        }
        if (not flag) {
            ans_match.push_back(match_pairs[i]);
        }
    }

    // Check second axiom
    for (int i = 0; i < ans_match.size(); ++i) {
        for (int j = i; j < ans_match.size(); ++j) {
            if ((i != j) and check_inter(ans_match[j].first, ans_match[i].first) and
                check_inter(ans_match[j].second, ans_match[i].second)) {
                verdict_amb(output_file, ans_match[i], ans_match[j], line_in, line_out);
                return 1;
            }
        }
    }

    true_verdict(output_file, ans_match, line_in, line_out);
    return 0;
}

int main(int argc, char* argv[]) {
    // Standard values
    std::string input_file = "input.txt", output_file = "output.txt";
    long long max_time = 3600; // An Hour

    // Reading parameters
    if (argc >= 2) {
        input_file = argv[1];
    }
    if (argc >= 3) {
        output_file = argv[2];
    }
    if (argc >= 4) {
        max_time = std::stoll(argv[3]);
    }
    // Setting up files
    std::ifstream data_in_file;
    data_in_file.open(input_file);
    if (!data_in_file.is_open()) throw std::invalid_argument("File does not exist");

    std::ofstream data_out_file;
    data_out_file.open(output_file);
    // Starting sequence
    std::string line;
    std::getline(data_in_file, line);
    data_out_file << line << std::endl;
    std::getline(data_in_file, line);
    data_out_file << line << std::endl;
    long long checks = stoll(line);

    for (int i = 0; i < checks; ++i) {
        //std::cout << "\rDone" << i << "checks out of" << checks << std::flush;

        check_transaction(data_in_file, data_out_file, max_time);
    }
    //std::cout << "\rFinished                         " << std::endl;
    data_in_file.close();
    data_out_file.close();
}

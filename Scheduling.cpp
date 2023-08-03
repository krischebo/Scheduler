// Scheduling.cpp : To display availability of a member on a given time for a given day
// @author: Krishna Chebolu

#include <iostream>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

// Class Employee to store the data coming from the excel file
// HDA = Hall Desk Associates
struct HDA {
	string timestamp;
	string name;

	vector<int> availability;

	friend ostream& operator<<(ostream&, const HDA&);
};

// Max hours
const int HOUR_LIMIT = 3;

// Function prototypes
void read_file_into_hda(const char*, vector<HDA>&);
vector<string> break_string(string&, char);
void display_all(vector<HDA>);
string to_lower(const string&);
vector<int> generate_matrix(vector<vector<string>>);
bool has(const string&, const vector<string>&);
string trim_nonsense(const string&);
bool is_available(const HDA&, int);
int count_hours(const string&, map<int, string>&);
void find_schedule(int, vector<HDA>&, map<int, string>&, int, map<int, string>&, int&);
vector<int> find_one_member_slots(vector<HDA>&, const map<int, string>&);
void greedily_initialize(vector<HDA>&, map<int, string>&);

// MAIN -----------------------------------------------------------------------
int main()
{
	// Variable for read_file_into_hda
	const char* filepath = "C:/Users/krish/Desktop/Desk Scheduling/Scheduling/test.csv";
	vector<HDA> associates;

	// Reads excel file and puts stuff into the struct above
	read_file_into_hda(filepath, associates);
	// display_all(associates);
	
	// This will hold the main schedule
	map <int, string> schedule;
	// Will hold best schedule found, even if not perfect
	map <int, string> best_schedule;
	// Max time slots filled tolerance
	int filled = 0;

	// Greedy initialization
	map<int, string> initial_schedule;
	greedily_initialize(associates, initial_schedule);

	find_schedule(0, associates, initial_schedule, HOUR_LIMIT, best_schedule, filled);

	if (filled > 0) {
		std::cout << "Best Schedule Found!" << std::endl;
		for (const auto& entry : best_schedule) {
			std::cout << "Time Slot " << entry.first << ": " << entry.second << std::endl;
		}
	}
	else {
		std::cout << "No valid schedule found." << std::endl;
	}

	return 0;

}
// ----------------------------------------------------------------------------

// Function to read the time availability csv file into a vector of HDA objects
void read_file_into_hda(const char* filepath, vector<HDA>& team) {
	// Importing the file into the program
	ifstream infile;
	infile.open(filepath);

	// Checking if file can be opened
	if (!infile.is_open()) { cerr << "ERROR: file cannot be opened."; }

	// Variable for each line
	string line;
	// Let's get that data in; main loop
	while (infile.good()) {
		// Creating stuff to store in before they go into an instance of HDA
		istringstream ss(line);
		string timestamp, name, temp;
		vector<string> m, t, w, th, f, sa, s;

		// Getting the timestamp and name right off the bat
		getline(infile, timestamp, ',');
		getline(infile, name, ',');

		getline(infile, temp, ',');
		m = break_string(temp, ';');
		temp = "";
		getline(infile, temp, ',');
		t = break_string(temp, ';');
		temp = "";
		getline(infile, temp, ',');
		w = break_string(temp, ';');
		temp = "";
		getline(infile, temp, ',');
		th = break_string(temp, ';');
		temp = "";
		getline(infile, temp, ',');
		f = break_string(temp, ';');
		temp = "";
		getline(infile, temp, ',');
		sa = break_string(temp, ';');
		temp = "";
		getline(infile, temp, '\n');
		s = break_string(temp, ';');
		temp = "";

		vector <vector<string>> week = { m, t, w, th, f, sa, s };

		vector<int> availability = generate_matrix(week);

		HDA associate{ timestamp, name, availability };
		team.push_back(associate);
	}

	infile.close();
}

// Function to break a string down and store the various stuff in a vector
vector<string> break_string(string& input, char delimiter) {
	// Creating the vector that will contain broken down string
	vector<string> broken;

	// Creating a stream for the input
	stringstream ss(input);
	// Creating variable to hold each time availability
	string token;

	// Loop to store the tokens into a vector separately
	while (getline(ss, token, delimiter)) {
		broken.push_back(token);
	}

	return broken;
}

// To display stuff
void display_all(vector<HDA> team) {
	for (HDA member : team) {
		cout << member << endl;
	}
}

// Operator<< overloading for displaying vector contents
ostream& operator<<(ostream& out, const vector<string>& v) {
	for (const auto& str : v) {
		out << str << ", ";
	}
	return out;
}

// Operator<< overloading for displaying HDA elements
ostream& operator<<(ostream& out, const HDA& hda) {
	out << endl;
	out << "Timestamp: " << hda.timestamp << endl;
	out << "Name: " << hda.name << endl;

	int counter = 0;

	for (auto element : hda.availability) {
		out << element << " ";
		counter++;
		if (counter % 9 == 0) {
			out << endl;
		}
	}
	out << endl;

	return out;
}

// Converts anything to completely lower case
string to_lower(const string& str) {
	string result;
	for (char c : str) {
		result += tolower(c);
	}
	return result;
}

// Generates availability matrix accounting for 9 time slots in 7 days
vector<int> generate_matrix(vector<vector<string>> week) {
	vector<string> time_slots = { "08:25","09:25","10:25","11:25","12:25","13:25","14:25","15:25","16:25" };

	vector<int> availability;

	for (vector<string> day : week) {
		for (string t_s : time_slots) {
			if (has(t_s, day)) {
				availability.push_back(1);
			}
			else {
				availability.push_back(0);
			}
		}
	}
	return availability;
}

// Returns true if a string is contained within a vector of strings
bool has(const string& target, const vector<string>& container) {
	for (string element : container) {
		string true_el = trim_nonsense(element);
		if (true_el == target) { return true; }
	}
	return false;
}

// Removes characters other than numbers and colons from a time string
string trim_nonsense(const string& input) {
	string result = input;

	// remove_if is used to move unwanted characters to the end of a string
	// erase is used to remove the characters
	// Lambda function states that the characters not wanted are anything but numbers and a colon
	result.erase(remove_if(result.begin(), result.end(), [](char c) {
		return !(isdigit(c) || c == ':');
		}), result.end());

	return result;
}

// Checks if a person is available for a given slot
bool is_available(const HDA& associate, int slot) {
	return associate.availability[slot] == 1;
}

// Counts the hours someone has been scheduled for
int count_hours(const string& associate_name, map<int, string>& schedule) {
	int count = 0;
	for (const auto& entry : schedule) {
		if (entry.second == associate_name) { count++; }
	}
	return count;
}

// Finds the best schedule possible
void find_schedule(int slot, vector<HDA>& associates, map<int, string>& schedule, int hour_limit,
	map<int, string>& best_schedule, int& current_filled) {
	// Base case; all time slots are somehow filled, the function just returns
	if (slot > 20) {
		int filled_slots = static_cast<int>(schedule.size());
		if (filled_slots > current_filled) {
			current_filled = filled_slots;
			best_schedule = schedule;
		}
		return;
	}

	// Fills up one member only slots
	vector<int> one_only = find_one_member_slots(associates, schedule);
	for (int one_only_slot : one_only) {
		for (HDA& ass : associates) {
			if (is_available(ass, one_only_slot)) {
				//int hours = count_hours(ass.name, schedule);
				if (count_hours(ass.name, schedule) < hour_limit) {
					// If hour limit is not breached, assign this slot to that person
					schedule[one_only_slot] = ass.name;

					// Continues further exploration
					find_schedule(slot + 1, associates, schedule, hour_limit, best_schedule, current_filled);

					// For backtracking
					schedule.erase(one_only_slot);
				}
			}
		}
	}

	// Checks if this time slot has already been filled
	if (schedule.count(slot) > 0) {
		find_schedule(slot + 1, associates, schedule, hour_limit, best_schedule, current_filled);
		return;
	}

	// Continue if one member slots are filled
	if (one_only.empty() || schedule.size() >= current_filled) {
		// Try scheduling available assocaites for this time slot
		for (HDA& associate : associates) {
			if (is_available(associate, slot)) {
				int hours = count_hours(associate.name, schedule);
				if (hours < hour_limit) {
					// If hour limit is not, this slot is assigned to this associate
					schedule[slot] = associate.name;

					// Continues exploring for the next time slot
					find_schedule(slot + 1, associates, schedule, hour_limit, best_schedule, current_filled);

					// Removes assignment for the next exploration
					schedule.erase(slot);
				}
			}
		}
	}
	//find_schedule(slot + 1, associates, schedule, hour_limit, best_schedule, current_filled);
}

// Checks for one member slots
vector<int> find_one_member_slots(vector<HDA>& associates, const map<int, string>& schedule) {

	vector<int> slots;

	// Loops through time slots
	for (int slot = 0; slot < 63; slot++) {
		int count = 0;
		for (const HDA& ass : associates) {
			// If not available, put in a temp vector
			if (is_available(ass, slot)) {
				count++;
			}
		}
		if (count == 1 && schedule.count(slot) == 0) {
			slots.push_back(slot);
		}
	}
	return slots;
}

// A greedy initialization
void greedily_initialize(vector<HDA>& associates, map<int, string>& initial_schedule) {
	vector<string> assigned_names;

	for (int slot = 0; slot < 63; slot++) {
		for (HDA& ass : associates) {
			// If available, hours are below limit, and if the name has not already been assigned
			if (is_available(ass, slot) && count_hours(ass.name, initial_schedule) < HOUR_LIMIT &&
				find(assigned_names.begin(), assigned_names.end(), ass.name) == assigned_names.end()) {
				initial_schedule[slot] = ass.name;
				assigned_names.push_back(ass.name);
				break;
			}
		}
	}
}
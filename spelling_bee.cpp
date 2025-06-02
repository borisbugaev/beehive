#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <cstring>
#include <string_view>
#include <chrono>

/*
Program to solve "NYT Spelling Bee" word puzzle relatively fast
There exists some issue that prevents execution in vscode
However, everything seems to work outside the ide
Current runtime ~30ms (-O0) ~20ms(-O1) ~15ms(-O2)
TODO:
--fix vscode exec
--make it faster overall
*/

const uint32_t ASC_OFST{96};
const uint32_t MAP_OFST{26};
const uint32_t ONE_BIT{1};

/* 
add a 5-bit key character identifier at the head of ui32
as 5 bits is adequate for a unique identifier 1=<i=<26
and the map schema is 26-bit, which leaves 6 bits for additional information
 */
uint32_t map_key_char(char kc, uint32_t str_map)
{
	const uint32_t init_key_map{MAP_OFST - (uint32_t(kc) - 'a')};
	const uint32_t shift_key_map{init_key_map << MAP_OFST};
	return shift_key_map | str_map;
}

/*
Accomplishes mapping for map_char (below)
Not super safe (does not verify valid input)
*/
uint32_t map_gen(char mc)
{
	return (ONE_BIT << (MAP_OFST - (uint32_t(mc) - ASC_OFST)));
}

/*
Let each lowercase ascii letter be l where a=<l=<z
Protects input integrity so that map generation only happens with valid char
Each unique value l returns bitmap as ui32
*/
uint32_t map_char(char wc)
{
	if (wc <= 'z' && wc >= 'a') 
	{
		return map_gen(wc);
	}
	else if (wc >= 'A' && wc <= 'Z')
	{
		auto kui{wc + ' '};
		return map_key_char((char)kui, map_gen((char)kui));
	}
	else
	{
		return 0;
	}
}

/*
Creates bitmap for set of chars in str_v
As str_v is a subset of the alphabet, the bitmap must >= 26 bits
Uses ui32 type to contain the returned bitmap
*/

uint32_t iter_map_str(std::string_view str_v)
{
	uint32_t string_map{0};
	for (int i = 0; i < str_v.length(); i++)
	{
		string_map = string_map | map_char(str_v[i]);
	}
	return string_map;
}

/*
Per NYT Bee rules:
Let c_set represent the set of letters used to spell a candidate word
Let k_set represent the set of valid letters
Let key be a single letter
Where c_set must be a subset of k_set
And c_set must contain key
Function returns 1 if conditions met
Else returns 0
*/
uint32_t map_compare(uint32_t key_map, uint32_t candidate_map)
{
	const uint32_t key_index{key_map >> MAP_OFST}; //this is "destructive" but it doesn't matter
	const uint32_t key_reference{(ONE_BIT << (key_index - ONE_BIT))};
	const uint32_t has_key{(candidate_map & key_reference) / key_reference}; //effectively bool
	const uint32_t is_submap{key_map / (key_map | candidate_map)};
	return (candidate_map * has_key * is_submap) / candidate_map;
}

int main(int argc, char* argv[]) 
{
	std::string kinput{};
	if (argv[1])
	{
		kinput = argv[1];
	}
	else
	{
		std::cout << "Enter Parameters: ";
		std::cin >> kinput;
	}

	auto start = std::chrono::steady_clock::now();
	#ifdef ADV_TIMING
	auto temp_start = std::chrono::steady_clock::now();
	#endif
	const std::string_view k_view{kinput};
	const uint32_t k_map{iter_map_str(k_view)};
	
	std::ifstream file("word_list.txt");
	if (!file.is_open())
	{
        std::cerr << "Error opening file." << std::endl;
        std::cerr << strerror(errno) << std::endl;
	}
	std::ofstream out_file("valid_words.txt");
	std::string line{};
	#ifdef ADV_TIMING
	long long sum_exec{0};
	long long num_exec{0};
	#endif
	while (std::getline(file, line))
	{
		std::string_view candidate{line};
		if (map_compare(k_map, iter_map_str(candidate)) && candidate.length() > 3)
		{
			out_file << candidate << "\n";
		}
		#ifdef ADV_TIMING
		auto call_complete = std::chrono::steady_clock::now();
		sum_exec += std::chrono::duration_cast<std::chrono::nanoseconds>(call_complete - temp_start).count();
		num_exec += 1;
		temp_start = std::chrono::steady_clock::now();
		#endif
	}
	file.close();
	out_file.close();
	
	auto end = std::chrono::steady_clock::now();
	#ifdef ADV_TIMING
	if (num_exec == 0)
	{
		num_exec = 1;
	}
	std::cout << "Average Time to Parse Single Word: " << sum_exec/num_exec << " ns\n";
	std::cout << "Total Words: " << num_exec << std::endl;
	std::cout << "Total Time spent evaluating words: " << sum_exec*0.001 << " mus\n";
	#endif
	std::cout << "Executed in ";
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
	// 
	return 0;
}
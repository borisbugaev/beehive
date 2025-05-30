#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <cstring>
#include <string_view>
#include <bitset>
#include <chrono>

/*
Program to solve "NYT Spelling Bee" word puzzle relatively fast
There exists some issue that prevents execution in vscode
However, everything seems to work outside the ide
Current runtime ~50ms
TODO:
--fix vscode exec
--improve input (currently parameters are hardcoded which is stupid)
--dictionary pruning?
--make it faster overall
*/

const uint32_t ASC_OFST{96};
const uint32_t TO_LCASE{32};
const uint32_t MAP_OFST{26};
const uint32_t ONE_BIT{1};

/* 
add a 5-bit key character identifier at the head of ui32
as 5 bits is adequate for a unique identifier 1=<i=<26
and the map schema is 26-bit, which leaves 6 bits for additional information
 */
uint32_t map_key_char(char kc, uint32_t str_map)
{
	const uint32_t init_key_map{MAP_OFST - (uint32_t(kc) - ASC_OFST) + ONE_BIT};
	const uint32_t shift_key_map{init_key_map << MAP_OFST};
	return shift_key_map | str_map;
}

/*
Let each lowercase ascii letter be l where a=<l=<z
Each unique value l returns bitmap as ui32
*/
uint32_t map_char(char wc)
{
	if (uint32_t(wc) < (ASC_OFST + ONE_BIT) ||
		uint32_t(wc) > (ASC_OFST + MAP_OFST))
		{
			if (uint32_t(wc) >= uint32_t('A') ||
				uint32_t(wc) <= uint32_t('Z'))
				{
					char kc{char(uint32_t(wc) + TO_LCASE)};
					return map_key_char(kc, map_char(kc));
				}
			else
			{
				return 0;
			}
		}
	else 
	{
		return (ONE_BIT << (MAP_OFST - (uint32_t(wc) - ASC_OFST)));
	}
}

/*
Creates bitmap for set of chars in str_v
As str_v is a subset of the alphabet, the bitmap must >= 26 bits
Uses ui32 type to contain the returned bitmap
*/
uint32_t map_str(std::string_view str_v)
{
	if (str_v.length() < 1)
	{
		return 0;
	}
	else
	{
		const char chr{str_v[0]};
		const uint32_t map_pref{map_char(str_v[0])};
		str_v.remove_prefix(1);
		return map_pref | map_str(str_v);
	}
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
	const std::string_view k_view{kinput};
	const uint32_t k_map{map_str(k_view)};
	
	std::ifstream file("word_list.txt");
	if (!file.is_open())
	{
        std::cerr << "Error opening file." << std::endl;
        std::cerr << strerror(errno) << std::endl;
    }
	std::ofstream out_file("valid_words.txt");
	std::string line{};
	while (std::getline(file, line))
	{
		std::string_view candidate{line};
		if (map_compare(k_map, map_str(candidate)) && candidate.length() > 3)
		{
			out_file << candidate << '\n';
		}
	}
	file.close();
	out_file.close();
	
	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << "ns" << std::endl;
	// 
	return 0;
}
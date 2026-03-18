import urllib.request
import json
from pathlib import Path
import time

# Browser header to avoid http rejection
opener = urllib.request.build_opener()
opener.addheaders = [('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36')]
urllib.request.install_opener(opener)

# Generation ranges
GEN_RANGES = {
    1: (1, 151),
    2: (152, 251),
    3: (252, 386),
    4: (387, 493)
}

# Maximum Pokemon ID per game version
VERSION_MAX_ID = {
    # Gen 1 games
    'red': 151, 'blue': 151, 'yellow': 151,
    # Gen 2 games
    'gold': 251, 'silver': 251, 'crystal': 251,
    # Gen 3 games
    'ruby': 386, 'sapphire': 386, 'emerald': 386,
    'firered': 386, 'leafgreen': 386,
    # Gen 4 games
    'diamond': 493, 'pearl': 493, 'platinum': 493,
    'heartgold': 493, 'soulsilver': 493
}

def get_all_versions():
    url = "https://pokeapi.co/api/v2/pokemon-species/1/"
    response = urllib.request.urlopen(url)
    data = json.loads(response.read().decode('utf-8'))

    versions = set()
    for entry in data['flavor_text_entries']:
        if entry['language']['name'] == 'en':
            versions.add(entry['version']['name'])

    # Filter to only include our desired games and sort them in a specific order
    ordered_versions = []
    for game in ['red', 'blue', 'yellow', 'gold', 'silver', 'crystal',
                 'ruby', 'sapphire', 'emerald', 'firered', 'leafgreen',
                 'diamond', 'pearl', 'platinum', 'heartgold', 'soulsilver']:
        if game in versions:
            ordered_versions.append(game)

    return ordered_versions

def get_flavor_text_for_pokemon(poke_id, version):
    url = f"https://pokeapi.co/api/v2/pokemon-species/{poke_id}/"
    response = urllib.request.urlopen(url)
    data = json.loads(response.read().decode('utf-8'))

    for entry in data['flavor_text_entries']:
        if (entry['language']['name'] == 'en' and
            entry['version']['name'] == version):
            text = entry['flavor_text']
            # Clean up the text: replace newlines and form feeds, escape quotes
            return text.replace('\n', ' ').replace('\f', ' ').replace('"', '\\"')

    return ""

def generate_all_versions():
    versions = get_all_versions()
    MAX_POKEMON_ID = 493

    print(f"Found {len(versions)} versions to include: {versions}")

    # Generate header file
    header_content = """#ifndef DATA_POKE_FLAVOR_H
#define DATA_POKE_FLAVOR_H

#include <string_view>

"""
    for version in versions:
        # Replace dashes with underscores for valid C++ identifiers
        cpp_version = version.replace('-', '_')
        header_content += f"extern const std::string_view kPokeFlavor_{cpp_version}[494];\n"
    header_content += "\n#endif // DATA_POKE_FLAVOR_H\n"

    # Create directories
    Path("include").mkdir(exist_ok=True)
    Path("src").mkdir(exist_ok=True)

    with open("include/data_poke_flavor.h", "w") as f:
        f.write(header_content)

    # Generate source file - OPTIMIZED VERSION
    source_content = """#include "data_poke_flavor.h"

"""
    for version in versions:
        cpp_version = version.replace('-', '_')
        source_content += f"const std::string_view kPokeFlavor_{cpp_version}[494] = {{\n"
        source_content += f'    "",  // 0\n'

        max_id = VERSION_MAX_ID.get(version, 493)

        for poke_id in range(1, MAX_POKEMON_ID + 1):
            if poke_id <= max_id:
                text = get_flavor_text_for_pokemon(poke_id, version)
                if not text:
                    print(f"WARNING: No flavor text for {version} #{poke_id}")
                source_content += f'    "{text}",  // {poke_id}\n'

                # Rate limiting only for actual API calls
                if poke_id % 20 == 0:
                    print(f"{version}: Processed {poke_id}/{max_id}...")
                    time.sleep(0.5)
                else:
                    time.sleep(0.1)
            else:
                # Fast path for non-existent Pokemon - no API call
                source_content += f'    "",  // {poke_id}\n'

            # Progress reporting for the whole array
            if poke_id % 100 == 0 and poke_id > max_id:
                print(f"{version}: Filled {poke_id}/493...")

        source_content += "};\n\n"

    with open("src/data_poke_flavor.cpp", "w") as f:
        f.write(source_content)

    print(f"\nGenerated include/data_poke_flavor.h and src/data_poke_flavor.cpp")
    print(f"Arrays created for versions: {versions}")

if __name__ == "__main__":
    generate_all_versions()

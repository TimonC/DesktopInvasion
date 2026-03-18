import urllib.request
import json
from pathlib import Path
import time

#Browser header to avoid http rejection
opener = urllib.request.build_opener()
opener.addheaders = [('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36')]
urllib.request.install_opener(opener)

def get_all_versions():
    url = "https://pokeapi.co/api/v2/pokemon-species/1/"
    response = urllib.request.urlopen(url)
    data = json.loads(response.read().decode('utf-8'))

    versions = set()
    for entry in data['flavor_text_entries']:
        if entry['language']['name'] == 'en':
            versions.add(entry['version']['name'])

    return sorted(list(versions))

def get_flavor_text_for_pokemon(poke_id, version):
    url = f"https://pokeapi.co/api/v2/pokemon-species/{poke_id}/"
    response = urllib.request.urlopen(url)
    data = json.loads(response.read().decode('utf-8'))

    for entry in data['flavor_text_entries']:
        if (entry['language']['name'] == 'en' and
            entry['version']['name'] == version):
            text = entry['flavor_text']
            return text.replace('\n', ' ').replace('\f', ' ').replace('"', '\\"')

    return ""

def generate_all_versions():
    versions = get_all_versions()
    MAX_POKEMON_ID = 493

    print(f"Found {len(versions)} versions: {versions}")

    header_content = """#ifndef DATA_POKE_FLAVOR_H
#define DATA_POKE_FLAVOR_H

#include <string_view>

"""
    for version in versions:
        # Replace dashes with underscores for valid C++ identifiers
        cpp_version = version.replace('-', '_')
        header_content += f"extern const std::string_view kPokeFlavor{cpp_version}[494];\n"
    header_content += "\n#endif // DATA_POKE_FLAVOR_H\n"

    with open("include/data_poke_flavor.h", "w") as f:
        f.write(header_content)


    source_content = """#include "data_poke_flavor.h"

"""
    for version in versions:
        cpp_version = version.replace('-', '_')
        source_content += f"const std::string_view kPokeFlavor{cpp_version}[494] = {{\n"
        source_content += f'    "",  // 0\n'

        for poke_id in range(1, MAX_POKEMON_ID + 1):
            time.sleep(0.1)
            text = get_flavor_text_for_pokemon(poke_id, version)
            source_content += f'    "{text}",  // {poke_id}\n'

            if poke_id % 20 == 0:
                print(f"{version}: Processed {poke_id}/493...")

        source_content += "};\n\n"

    Path("include").mkdir(exist_ok=True)
    Path("src").mkdir(exist_ok=True)


    with open("src/data_poke_flavor.cpp", "w") as f:
        f.write(source_content)

    print(f"Generated include/data_poke_flavor.h and src/data_poke_flavor.cpp")
    print(f"Arrays created for versions: {versions}")

generate_all_versions()

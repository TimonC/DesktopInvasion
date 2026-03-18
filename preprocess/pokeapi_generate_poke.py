from skip_move_ids import SKIP_MOVE_IDS #This list of moves to exclude was manually compiled to drastically reduce complexity of DesktopInvasion

import urllib.request
import json
from pathlib import Path
import time
import random


MAX_POKEMON_ID = 493
BASE_DELAY =1
JITTER = 0.3

#Browser header to avoid http rejection
opener = urllib.request.build_opener()
opener.addheaders = [('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36')]
urllib.request.install_opener(opener)

GEN4_VERSION_GROUPS = [
    'diamond-pearl',
    'platinum',
    'heartgold-soulsilver'
]

POKEMON_WITH_VALID_DASH = ["Ho-oh", "Porygon-Z"]

pokemons = []
def format_type_enum(type_name):
    if not type_name:
        return 'Type::Null'
    return f'Type::{type_name.capitalize()}'

def extract_types(type_list):
    types = ['', '']
    for i, type_obj in enumerate(type_list[:2]):
        if type_obj and type_obj.get('type'):
            types[i] = type_obj['type'].get('name', '')
    return types

def extract_base_stats(stats_list):
    stat_order = ['hp', 'attack', 'defense', 'special-attack', 'special-defense', 'speed']
    stats = {}

    for stat_entry in stats_list:
        if stat_entry.get('stat'):
            stat_name = stat_entry['stat'].get('name', '')
            if stat_name in stat_order:
                stats[stat_name] = stat_entry.get('base_stat', 0)

    return [stats.get(stat_name, 0) for stat_name in stat_order]

def extract_eligible_moves(moves_list):
    eligible_moves = []
    move_dict = {}

    for move_entry in moves_list:
        version_details = move_entry.get('version_group_details', [])

        target_details = []
        for detail in version_details:
            version_group = detail.get('version_group', {}).get('name', '')
            if version_group in GEN4_VERSION_GROUPS:
                target_details.append(detail)

        if not target_details:
            continue

        for detail in target_details:
            learn_method = detail.get('move_learn_method', {}).get('name', '')
            level_learned = detail.get('level_learned_at', 0)

            move_url = move_entry.get('move', {}).get('url', '')
            if move_url:
                parts = move_url.rstrip('/').split('/')
                if parts:
                    move_id = int(parts[-1])
                    if move_id not in SKIP_MOVE_IDS:

                        is_level_up = (learn_method == 'level-up')

                        if not is_level_up:
                            level_learned = -1

                        if move_id not in move_dict:
                            move_dict[move_id] = (level_learned, is_level_up)
                        elif is_level_up and not move_dict[move_id][1]:
                            move_dict[move_id] = (level_learned, is_level_up)
                        elif is_level_up and level_learned > 0 and level_learned < move_dict[move_id][0]:
                            move_dict[move_id] = (level_learned, is_level_up)

    for move_id, (level, _) in move_dict.items():
        eligible_moves.append({'move_id': move_id, 'level': level})

    eligible_moves.sort(key=lambda x: (x['level'], x['move_id']))
    return eligible_moves

def extract_evolution_data(species_url, current_poke_id):
    evolves_to = []

    response = urllib.request.urlopen(species_url)
    species_data = json.loads(response.read().decode('utf-8'))

    evolution_chain_url = species_data.get('evolution_chain', {}).get('url')
    if not evolution_chain_url:
        return evolves_to

    response = urllib.request.urlopen(evolution_chain_url)
    evolution_data = json.loads(response.read().decode('utf-8'))

    chain = evolution_data.get('chain', {})
    evolves_to = find_next_evolutions(chain, current_poke_id)

    return evolves_to

def find_next_evolutions(chain, target_id):
    evolves_to = []

    species_url = chain.get('species', {}).get('url', '')
    if species_url:
        parts = species_url.rstrip('/').split('/')
        if parts:
            chain_id = int(parts[-1])

            if chain_id == target_id:
                for evolution in chain.get('evolves_to', []):
                    evolved_species_url = evolution.get('species', {}).get('url', '')
                    if evolved_species_url:
                        parts = evolved_species_url.rstrip('/').split('/')
                        if parts:
                            evolved_id = int(parts[-1])
                            if 1 <= evolved_id <= MAX_POKEMON_ID:
                                evolution_condition = extract_evolution_condition(evolution)
                                evolves_to.append({
                                    'pokedex_id': evolved_id,
                                    'level': evolution_condition
                                })
                return evolves_to

    for evolution in chain.get('evolves_to', []):
        result = find_next_evolutions(evolution, target_id)
        if result:
            return result

    return evolves_to

def extract_evolution_condition(evolution):
    details = evolution.get('evolution_details', [])

    if not details:
        return -1

    detail = details[0]

    min_level = detail.get('min_level')
    if min_level is not None:
        return min_level

    if detail.get('trigger', {}).get('name') == 'level-up':
        return -1

    return -1

def format_pokemon_name(name):
    if not name:
        return name
    if name.lower() not in [p.lower() for p in POKEMON_WITH_VALID_DASH]:
        name = name.split("-")[0]
    return name.capitalize()

for poke_id in range(1, MAX_POKEMON_ID + 1):
    delay = BASE_DELAY + random.uniform(-JITTER, JITTER)
    time.sleep(delay)

    url = f"https://pokeapi.co/api/v2/pokemon/{poke_id}/"
    response = urllib.request.urlopen(url)
    contents = response.read()
    poke_data = json.loads(contents.decode('utf-8'))

    types = extract_types(poke_data.get('types', []))

    if poke_id == 487: #For Giratina use origin form base stats
        #https://bulbapedia.bulbagarden.net/wiki/Giratina_(Pok%C3%A9mon)#Origin_Forme
        base_stats = ["150", "120", "100", "120", "100", "90"]
    else:
        base_stats = extract_base_stats(poke_data.get('stats', []))

    eligible_moves = extract_eligible_moves(poke_data.get('moves', []))
    base_experience = poke_data.get('base_experience', 0)

    species_url = poke_data.get('species', {}).get('url', '')
    catch_rate = 0
    eligible_evolves = []

    if species_url:
        response = urllib.request.urlopen(species_url)
        species_data = json.loads(response.read().decode('utf-8'))
        catch_rate = species_data.get('capture_rate', 0)
        eligible_evolves = extract_evolution_data(species_url, poke_id)

    pokemons.append({
        'id': poke_id,
        'name': poke_data.get('name', ''),
        'types': types,
        'base_stats': base_stats,
        'catch_rate': catch_rate,
        'base_experience': base_experience,
        'eligible_moves': eligible_moves,
        'eligible_evolves': eligible_evolves
    })

    move_count = len(eligible_moves)
    evolve_count = len(eligible_evolves)
    evolve_str = f"{evolve_count} evolve" + ("s" if evolve_count != 1 else "")

    print(f"Added Pokémon: {poke_id:03d} - {format_pokemon_name(poke_data.get('name', ''))} ({move_count} moves, {evolve_str}, catch rate: {catch_rate}, base XP: {base_experience})")

# Propagate moves from pre-evolutions to evolutions
evolution_parents = {}

for pokemon in pokemons:
    for evolve in pokemon['eligible_evolves']:
        child_id = evolve['pokedex_id']
        if child_id not in evolution_parents:
            evolution_parents[child_id] = []
        evolution_parents[child_id].append(pokemon['id'])

for pokemon in pokemons:
    poke_id = pokemon['id']
    if poke_id in evolution_parents:
        parent_ids = evolution_parents[poke_id]
        all_parent_moves = []

        for parent_id in parent_ids:
            parent_pokemon = next(p for p in pokemons if p['id'] == parent_id)
            all_parent_moves.extend(parent_pokemon['eligible_moves'])

        current_move_ids = {move['move_id'] for move in pokemon['eligible_moves']}
        for parent_move in all_parent_moves:
            if parent_move['move_id'] not in current_move_ids:
                pokemon['eligible_moves'].append(parent_move)

        pokemon['eligible_moves'].sort(key=lambda x: (x['level'], x['move_id']))

        new_count = len(pokemon['eligible_moves'])
        old_count = len(current_move_ids)
        if new_count > old_count:
            print(f"Updated {pokemon['name']} with {new_count - old_count} moves from parents {parent_ids}")

def generate_pokemon_data_direct():
    source_content = """#include "data_poke.h"

"""

    for pokemon in pokemons:
        poke_id = pokemon['id']

        eligible_moves = pokemon['eligible_moves']
        if eligible_moves:
            source_content += f"static constexpr EligibleMove eligible_moves_{poke_id}[] = {{\n"
            for move in eligible_moves:
                source_content += f"    {{{move['move_id']}, {move['level']}}},\n"
            source_content += "};\n"
            eligible_move_count = len(eligible_moves)
        else:
            source_content += f"static constexpr EligibleMove eligible_moves_{poke_id}[] = {{}};\n"
            eligible_move_count = 0

        eligible_evolves = pokemon['eligible_evolves']
        if eligible_evolves:
            source_content += f"static constexpr EligibleEvolve eligible_evolves_{poke_id}[] = {{\n"
            for evolve in eligible_evolves:
                source_content += f"    {{{evolve['pokedex_id']}, {evolve['level']}}},\n"
            source_content += "};\n"
            eligible_evolve_count = len(eligible_evolves)
        else:
            source_content += f"static constexpr EligibleEvolve eligible_evolves_{poke_id}[] = {{}};\n"
            eligible_evolve_count = 0

        _type1 = pokemon['types'][0]
        _type2 = pokemon['types'][1]

        if _type1=="fairy":
            _type1="normal"
        elif _type2=="fairy":
            _type2=None
        type1 = format_type_enum(_type1)
        type2 = format_type_enum(_type2) if _type2 else 'Type::Null'
        formatted_name = format_pokemon_name(pokemon['name']).replace('"', '\\"')
        stats = pokemon['base_stats']
        stats_str = "{" + f"{stats[0]}, {stats[1]}, {stats[2]}, {stats[3]}, {stats[4]}, {stats[5]}" + "}"
        catch_rate = pokemon['catch_rate']
        base_experience = pokemon['base_experience']

        source_content += f"""static constexpr Poke poke_{poke_id} = {{
    {poke_id},
    "{formatted_name}",
    {{{type1}, {type2}}},
    {stats_str},
    {catch_rate},
    {base_experience},
    eligible_moves_{poke_id},
    {eligible_move_count},
    eligible_evolves_{poke_id},
    {eligible_evolve_count}
}};

"""

    source_content += f"const Poke* const kPokesByIndex[{MAX_POKEMON_ID + 1}] = {{\n"
    source_content += f"    nullptr,  // 0\n"

    for poke_id in range(1, MAX_POKEMON_ID + 1):
        if poke_id % 10 == 0:
            source_content += f"    // IDs {poke_id:03d}-{min(poke_id+9, MAX_POKEMON_ID):03d}\n"
        source_content += f"    &poke_{poke_id},  // {poke_id}\n"

    source_content += "};\n"
    return source_content

source_content = generate_pokemon_data_direct()
output_path = Path("src/data_poke.cpp")
output_path.parent.mkdir(parents=True, exist_ok=True)

with open(output_path, 'w', encoding='utf-8') as f:
    f.write(source_content)

print(f"\nGenerated src/data_poke.cpp")
print(f"Total Pokémon: {len(pokemons)}")

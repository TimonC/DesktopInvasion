from skip_move_ids import SKIP_MOVE_IDS

import urllib.request
import json
from pathlib import Path
import time
import random


MAX_POKEMON_ID = 493
BASE_DELAY = 1
JITTER = 0.3

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
    tm_moves = set()
    non_tm_levels = {}

    for move_entry in moves_list:
        version_details = move_entry.get('version_group_details', [])
        target_details = [
            d for d in version_details
            if d.get('version_group', {}).get('name', '') in GEN4_VERSION_GROUPS
        ]
        if not target_details:
            continue

        move_url = move_entry.get('move', {}).get('url', '')
        if not move_url:
            continue
        parts = move_url.rstrip('/').split('/')
        if not parts:
            continue
        move_id = int(parts[-1])
        if move_id in SKIP_MOVE_IDS:
            continue

        for detail in target_details:
            learn_method = detail.get('move_learn_method', {}).get('name', '')
            level_learned = detail.get('level_learned_at', 0)

            if learn_method == 'machine':
                tm_moves.add(move_id)
            else:
                if learn_method == 'level-up' and level_learned > 0:
                    candidate = level_learned
                else:
                    candidate = 40
                if move_id not in non_tm_levels or candidate < non_tm_levels[move_id]:
                    non_tm_levels[move_id] = candidate

    eligible_moves = []
    all_move_ids = tm_moves | set(non_tm_levels.keys())
    for move_id in all_move_ids:
        if move_id in tm_moves:
            eligible_moves.append({'move_id': move_id, 'level': -1})
        if move_id in non_tm_levels:
            eligible_moves.append({'move_id': move_id, 'level': non_tm_levels[move_id]})

    eligible_moves.sort(key=lambda x: (x['level'] if x['level'] != -1 else float('inf'), x['move_id']))
    return eligible_moves

def extract_evolution_data(species_url, current_poke_id):
    response = urllib.request.urlopen(species_url)
    species_data = json.loads(response.read().decode('utf-8'))
    evolution_chain_url = species_data.get('evolution_chain', {}).get('url')
    if not evolution_chain_url:
        return []
    response = urllib.request.urlopen(evolution_chain_url)
    evolution_data = json.loads(response.read().decode('utf-8'))
    return find_next_evolutions(evolution_data.get('chain', {}), current_poke_id)

def find_next_evolutions(chain, target_id):
    species_url = chain.get('species', {}).get('url', '')
    if species_url:
        parts = species_url.rstrip('/').split('/')
        if parts:
            chain_id = int(parts[-1])
            if chain_id == target_id:
                evolutions = []
                for evolution in chain.get('evolves_to', []):
                    evolved_species_url = evolution.get('species', {}).get('url', '')
                    if evolved_species_url:
                        parts = evolved_species_url.rstrip('/').split('/')
                        if parts:
                            evolved_id = int(parts[-1])
                            if 1 <= evolved_id <= MAX_POKEMON_ID:
                                evolutions.append({
                                    'pokedex_id': evolved_id,
                                    'level': extract_evolution_condition(evolution)
                                })

                level_up_levels = [e['level'] for e in evolutions if e['level'] > 0]
                fallback = min(level_up_levels) if level_up_levels else 30

                for e in evolutions:
                    if e['level'] == -1:
                        e['level'] = fallback

                return evolutions

    for evolution in chain.get('evolves_to', []):
        result = find_next_evolutions(evolution, target_id)
        if result is not None and len(result) > 0:
            return result
    return []

def extract_evolution_condition(evolution):
    details = evolution.get('evolution_details', [])
    if not details:
        return -1
    detail = details[0]
    min_level = detail.get('min_level')
    if min_level is not None:
        return min_level
    return -1

def format_pokemon_name(name):
    if not name:
        return name
    if name.lower() not in [p.lower() for p in POKEMON_WITH_VALID_DASH]:
        name = name.split("-")[0]
    return name.upper()

print("Fetching Pokémon data from PokéAPI...")
for poke_id in range(1, MAX_POKEMON_ID + 1):
    time.sleep(BASE_DELAY + random.uniform(-JITTER, JITTER))

    url = f"https://pokeapi.co/api/v2/pokemon/{poke_id}/"
    response = urllib.request.urlopen(url)
    poke_data = json.loads(response.read().decode('utf-8'))

    types = extract_types(poke_data.get('types', []))

    if poke_id == 487:
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

    print(f"  Fetched {poke_id:03d} - {format_pokemon_name(poke_data.get('name', ''))} (raw moves: {len(eligible_moves)})")

print("\nBuilding evolution parent map...")
evolution_parents = {}
for pokemon in pokemons:
    for evolve in pokemon['eligible_evolves']:
        child_id = evolve['pokedex_id']
        if child_id not in evolution_parents:
            evolution_parents[child_id] = []
        evolution_parents[child_id].append(pokemon['id'])

print("Propagating moves through evolution chains (forward only)...")
for pokemon in pokemons:
    pokemon['move_dict'] = {}
    for move in pokemon['eligible_moves']:
        mid = move['move_id']
        lvl = move['level']
        if mid not in pokemon['move_dict'] or lvl < pokemon['move_dict'][mid]:
            pokemon['move_dict'][mid] = lvl

changed = True
iteration = 0
while changed:
    changed = False
    iteration += 1
    for pokemon in pokemons:
        if pokemon['id'] not in evolution_parents:
            continue
        cur = pokemon['move_dict']
        for parent_id in evolution_parents[pokemon['id']]:
            parent = next(p for p in pokemons if p['id'] == parent_id)
            for mid, lvl in parent['move_dict'].items():
                if mid not in cur or lvl < cur[mid]:
                    cur[mid] = lvl
                    changed = True
        pokemon['move_dict'] = cur
    print(f"  Propagation iteration {iteration}: {'changed' if changed else 'stable'}")

print("Converting move dictionaries back to list format...")
for pokemon in pokemons:
    moves = [{'move_id': mid, 'level': lvl} for mid, lvl in pokemon['move_dict'].items()]
    moves.sort(key=lambda x: (x['level'] if x['level'] != -1 else float('inf'), x['move_id']))
    pokemon['eligible_moves'] = moves

print("\nFinal move counts after propagation:")
for pokemon in pokemons:
    if pokemon['id'] in [1, 2, 3, 4, 5, 6, 7, 8, 9, 25, 26, 133, 134, 135, 136, 196, 197, 470, 471]:
        print(f"  {pokemon['id']:03d} - {format_pokemon_name(pokemon['name'])}: {len(pokemon['eligible_moves'])} moves")

def generate_pokemon_data_direct():
    source_content = '#include "data_poke.h"\n\n'

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
            source_content += f"static constexpr EligibleMove eligible_moves_{poke_id}[] = {{{{0, 0}}}};\n"
            eligible_move_count = 0

        eligible_evolves = pokemon['eligible_evolves']
        if eligible_evolves:
            source_content += f"static constexpr EligibleEvolve eligible_evolves_{poke_id}[] = {{\n"
            for evolve in eligible_evolves:
                source_content += f"    {{{evolve['pokedex_id']}, {evolve['level']}}},\n"
            source_content += "};\n"
            eligible_evolve_count = len(eligible_evolves)
        else:
            source_content += f"static constexpr EligibleEvolve eligible_evolves_{poke_id}[] = {{{{0, 0}}}};\n"
            eligible_evolve_count = 0

        _type1 = pokemon['types'][0]
        _type2 = pokemon['types'][1]
        if _type1 == "fairy":
            _type1 = "normal"
        elif _type2 == "fairy":
            _type2 = None
        type1 = format_type_enum(_type1)
        type2 = format_type_enum(_type2) if _type2 else 'Type::Null'
        formatted_name = format_pokemon_name(pokemon['name']).replace('"', '\\"')
        stats = pokemon['base_stats']
        stats_str = "{" + f"{stats[0]}, {stats[1]}, {stats[2]}, {stats[3]}, {stats[4]}, {stats[5]}" + "}"

        source_content += f"""static constexpr Poke poke_{poke_id} = {{
    {poke_id},
    "{formatted_name}",
    {{{type1}, {type2}}},
    {stats_str},
    {pokemon['catch_rate']},
    {pokemon['base_experience']},
    eligible_moves_{poke_id},
    {eligible_move_count},
    eligible_evolves_{poke_id},
    {eligible_evolve_count}
}};

"""

    source_content += f"const Poke* const kPokesByIndex[{MAX_POKEMON_ID + 1}] = {{\n"
    source_content += "    nullptr,\n"
    for poke_id in range(1, MAX_POKEMON_ID + 1):
        source_content += f"    &poke_{poke_id},\n"
    source_content += "};\n\n"

    # Generate cumulative weights array for inverse catch rate spawning
    source_content += "// Cumulative weights for inverse catch rate spawning\n"
    source_content += "// Weight = 256 - catch_rate (lower catch rate = higher spawn chance)\n"
    source_content += "constexpr int kCatchRateCumulativeWeights[] = {\n"
    source_content += "    0,  // Index 0 unused\n"

    cumulative = 0
    for poke_id in range(1, MAX_POKEMON_ID + 1):
        pokemon = next(p for p in pokemons if p['id'] == poke_id)
        weight = 256 - pokemon['catch_rate']
        cumulative += weight
        source_content += f"    {cumulative},  // {pokemon['name'].upper()} (catch_rate: {pokemon['catch_rate']}, weight: {weight})\n"

    source_content += "};\n\n"
    source_content += f"const int kTotalCatchRateWeight = {cumulative};\n"

    return source_content

source_content = generate_pokemon_data_direct()
output_path = Path("src/data_poke.cpp")
output_path.parent.mkdir(parents=True, exist_ok=True)

with open(output_path, 'w', encoding='utf-8') as f:
    f.write(source_content)

print(f"\nGenerated src/data_poke.cpp")
print(f"Total Pokémon processed: {len(pokemons)}")

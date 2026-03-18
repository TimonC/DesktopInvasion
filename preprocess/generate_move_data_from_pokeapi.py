import urllib.request
import json
from pathlib import Path

# List of move IDs to skip
SKIP_MOVE_IDS = [
    6,
    12,
    18,
    46,
    69,
    90,
    99,
    101,
    102,
    160,
    164,
    169,
    170,
    191,
    193,
    194,
    195,
    199,
    203,
    212,
    213,
    262,
    264,
    265,
    266,
    267,
    270,
    271,
    272,
    274,
    275,
    277,
    278,
    287,
    288,
    289,
    290,
    293,
    300,
    335,
    343,
    353,
    364,
    368,
    373,
    374,
    376,
    377,
    380,
    381,
    382,
    383,
    385,
    390,
    415,
    432,
    445,
    447,
    449
]

moves = []
pokeApiFieldsToInclude = [
    "id",
    "name",
    "accuracy",
    "effect_chance",
    "priority",
    "power",
]

def should_skip_move(move_id):
    """Check if a move should be skipped"""
    return move_id in SKIP_MOVE_IDS or move_id >= 10000

def extract_stat_changes(stat_changes_list):
    """
    Extract stat changes into a list of 5 integers: [attack, sp_attack, defense, sp_defense, speed]
    Index 0 = attack, 1 = sp_attack, 2 = defense, 3 = sp_defense, 4 = speed
    """
    stats = [0, 0, 0, 0, 0]
    stat_map = {
        'attack': 0,
        'special-attack': 1,
        'defense': 2,
        'special-defense': 3,
        'speed': 4
    }

    for stat_change in stat_changes_list:
        stat_name = stat_change.get('stat', {}).get('name', '')
        change_value = stat_change.get('change', 0)
        if stat_name in stat_map:
            stats[stat_map[stat_name]] = change_value

    return stats

def extract_type_name(type_obj):
    """Extract the name field from the type object"""
    if type_obj:
        return type_obj.get('name', '')
    return ''

def extract_learned_by_pokemon_ids(learned_by_list):
    """Extract Pokemon IDs from the learned_by_pokemon list"""
    pokemon_ids = []
    for pokemon in learned_by_list:
        url = pokemon.get('url', '')
        if url:
            parts = url.rstrip('/').split('/')
            if parts:
                try:
                    pokemon_id = int(parts[-1])
                    if pokemon_id <= 493:
                        pokemon_ids.append(pokemon_id)
                except ValueError:
                    pass
    return pokemon_ids

def extract_gen4_english_flavor_text(flavor_text_entries):
    """
    Extract the English flavor text from Generation 4 (Diamond/Pearl)
    Looking for version_group: 'diamond-pearl'
    """
    for entry in flavor_text_entries:
        language = entry.get('language', {}).get('name', '')
        version_group = entry.get('version_group', {}).get('name', '')
        if language == 'en' and version_group == 'diamond-pearl':
            return entry.get('flavor_text', '')
    for entry in flavor_text_entries:
        language = entry.get('language', {}).get('name', '')
        version_group = entry.get('version_group', {}).get('name', '')
        if language == 'en' and version_group in ['platinum', 'heartgold-soulsilver']:
            return entry.get('flavor_text', '')
    return ''

def clean_flavor_text(text):
    """Clean flavor text: replace newlines with spaces and normalize"""
    if not text:
        return ""

    # Replace newlines with spaces
    text = text.replace('\n', ' ')
    text = text.replace('\r', ' ')
    text = text.replace('\f', ' ')  # Form feed

    # Replace multiple spaces with single space
    while '  ' in text:
        text = text.replace('  ', ' ')

    # Strip leading/trailing whitespace
    text = text.strip()

    return text

# Fetch all moves from generations 1-3
for i in range(1, 4):
    response = urllib.request.urlopen(f"https://pokeapi.co/api/v2/generation/{i}/")
    contents = response.read()
    data = json.loads(contents.decode('utf-8'))
    for move in data.get('moves', []):
        move_url = move.get('url')
        move_response = urllib.request.urlopen(move_url)
        move_data = json.loads(move_response.read().decode('utf-8'))

        move_id = move_data.get('id')
        if should_skip_move(move_id):
            continue

        moveFilled = {}
        for field in pokeApiFieldsToInclude:
            moveFilled[field] = move_data.get(field)
        moveFilled['stat_changes'] = extract_stat_changes(move_data.get('stat_changes', []))
        moveFilled['type'] = extract_type_name(move_data.get('type'))
        moveFilled['learned_by_pokemon'] = extract_learned_by_pokemon_ids(move_data.get('learned_by_pokemon', []))

        # Extract and clean flavor text
        raw_flavor_text = extract_gen4_english_flavor_text(move_data.get('flavor_text_entries', []))
        moveFilled['flavor_text'] = clean_flavor_text(raw_flavor_text)

        moves.append(moveFilled)
        print(f"Added move: {moveFilled['id']} - {moveFilled['name']}")

def generate_moves_data_direct():
    """Generate C++ source using direct pointer array (simpler & faster)"""
    if not moves:
        return ""

    # Filter and sort valid moves
    valid_moves = [m for m in moves if not should_skip_move(m['id'])]
    valid_moves.sort(key=lambda x: x['id'])

    if not valid_moves:
        return ""

    max_move_id = max(m['id'] for m in valid_moves)

    # Generate source file content
    source_content = """#include "data_move.h"

namespace {
"""

    # Create all moves in anonymous namespace
    for move in valid_moves:
        move_id = move['id']

        # Learned by pokemon as static array (not vector for initialization)
        learned_by = move['learned_by_pokemon']
        if learned_by:
            source_content += f"    static const int learned_by_{move_id}[] = {{"
            source_content += ", ".join(str(p) for p in learned_by)
            source_content += "};\n"
            learned_count = len(learned_by)
        else:
            source_content += f"    static const int learned_by_{move_id}[] = {{0}};\n"
            learned_count = 0

        # Move definition
        name = move['name'].replace('"', '\\"')

        # Escape the flavor text properly (already cleaned)
        flavor = move['flavor_text'].replace('"', '\\"')

        move_type = move['type'].replace('"', '\\"')

        accuracy = move['accuracy'] if move['accuracy'] is not None else -1
        effect_chance = move['effect_chance'] if move['effect_chance'] is not None else -1
        power = move['power'] if move['power'] is not None else -1

        stats = "{" + ", ".join(str(s) for s in move['stat_changes']) + "}"

        source_content += f"""    static const Move move_{move_id} = {{
        {move_id},
        "{name}",
        {accuracy},
        {effect_chance},
        {move['priority']},
        {power},
        "{move_type}",
        {stats},
        "{flavor}",
        learned_by_{move_id},  // Pointer to static array
        {learned_count}        // Size of array
    }};

"""

    source_content += "} // namespace\n\n"

    # Create the direct pointer array (kMovesByIndex)
    source_content += f"const Move* const kMovesByIndex[{max_move_id + 1}] = {{\n"

    # Build the sparse array
    move_dict = {move['id']: f"&move_{move['id']}" for move in valid_moves}

    for i in range(max_move_id + 1):
        if i % 10 == 0:
            source_content += f"    // IDs {i:03d}-{min(i+9, max_move_id):03d}\n"

        if i in move_dict:
            source_content += f"    {move_dict[i]},  // {i}\n"
        else:
            source_content += f"    nullptr,  // {i}\n"

    source_content += "};\n\n"

    # Constants
    source_content += f"const int kMaxMoveId = {max_move_id};\n"
    source_content += f"const int kMoveCount = {len(valid_moves)};\n"

    return source_content
# Generate the C++ file
source_content = generate_moves_data_direct()

if source_content:
    # Ensure the src directory exists
    output_path = Path("src/data_move.cpp")
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Write source file
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(source_content)

    valid_move_count = len([m for m in moves if not should_skip_move(m['id'])])
    print(f"\nGenerated ../src/data_move.cpp")
    print(f"Total moves: {valid_move_count}")
    print(f"Max move ID: {max(m['id'] for m in moves if not should_skip_move(m['id']))}")
    print(f"Array size: {max(m['id'] for m in moves if not should_skip_move(m['id'])) + 1}")
    print(f"Skipped move IDs: {SKIP_MOVE_IDS}")

    # Calculate memory usage
    array_size = max(m['id'] for m in moves if not should_skip_move(m['id'])) + 1
    pointer_array_memory = array_size * 8  # 8 bytes per pointer on 64-bit
    print(f"Pointer array memory: {pointer_array_memory / 1024:.2f} KB")

else:
    print("Error: No valid moves to generate")

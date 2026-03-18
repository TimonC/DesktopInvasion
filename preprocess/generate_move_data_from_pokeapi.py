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

# Approved ailments (matching the Ailment enum)
APPROVED_AILMENTS = [
    'none',
    'burn',
    'freeze',
    'paralysis',
    'poison',
    'sleep',
    'confusion'
]

moves = []
pokeApiFieldsToInclude = [
    "id",
    "name",
    "accuracy",
    "priority",
    "power",
]

def should_skip_move(move_id):
    """Check if a move should be skipped"""
    return move_id in SKIP_MOVE_IDS or move_id >= 10000

def should_skip_move_by_ailment(ailment_name):
    """Check if a move should be skipped based on its ailment"""
    return ailment_name not in APPROVED_AILMENTS

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

def extract_damage_class_name(damage_class_obj):
    """Extract the name field from the damage_class object"""
    if damage_class_obj:
        return damage_class_obj.get('name', '')
    return ''

def format_type_enum(type_name):
    """Convert type name to Type enum format (e.g., 'normal' -> 'Type::Normal')"""
    if not type_name:
        return 'Type::Null'
    if type_name == "fairy":
        type_name = "normal"
    return f'Type::{type_name.capitalize()}'

def format_category_enum(damage_class_name):
    """Convert damage class name to MoveCategory enum format"""
    if not damage_class_name:
        return 'MoveCategory::NonDamaging'

    category_map = {
        'physical': 'MoveCategory::PhysicalAtk',
        'special': 'MoveCategory::SpecialAtk',
        'status': 'MoveCategory::NonDamaging'
    }

    return category_map.get(damage_class_name, 'MoveCategory::NonDamaging')

def format_ailment_enum(ailment_name):
    """Convert ailment name to Ailment enum format (e.g., 'burn' -> 'Ailment::Burn')"""
    if not ailment_name or ailment_name == 'none':
        return 'Ailment::Null'
    return f'Ailment::{ailment_name.capitalize()}'

def format_move_name(name):
    """Format move name: capitalize first letter and first letter after each dash, replace dashes with spaces"""
    if not name:
        return name

    words = name.split('-')
    formatted_words = [word.capitalize() for word in words]
    return ' '.join(formatted_words)

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

def extract_meta_data(meta):
    """Extract meta data fields from the move's meta object"""
    if not meta:
        return None

    ailment_name = meta.get('ailment', {}).get('name', 'none')

    # Check if this move should be skipped based on ailment
    if should_skip_move_by_ailment(ailment_name):
        return None

    def convert_null_to_neg_one(value):
        """Convert None to -1, keep 0 as 0"""
        if value is None:
            return -1
        return value

    meta_data = {
        'ailment': ailment_name,
        'min_hits': convert_null_to_neg_one(meta.get('min_hits')),
        'max_hits': convert_null_to_neg_one(meta.get('max_hits')),
        'min_turns': convert_null_to_neg_one(meta.get('min_turns')),
        'max_turns': convert_null_to_neg_one(meta.get('max_turns')),
        'drain': meta.get('drain', 0),
        'healing': meta.get('healing', 0),
        'crit_rate': meta.get('crit_rate', 0),
        'ailment_chance': meta.get('ailment_chance', 0),
        'flinch_chance': meta.get('flinch_chance', 0),
        'stat_chance': meta.get('stat_chance', 0)
    }

    return meta_data

# Fetch all moves from generations 1-4
for i in range(1, 5):
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

        # Extract meta data and check if move should be skipped
        meta_data = extract_meta_data(move_data.get('meta'))
        if meta_data is None:
            print(f"Skipped move (unapproved ailment): {move_id} - {move_data.get('name')}")
            continue

        moveFilled = {}
        for field in pokeApiFieldsToInclude:
            moveFilled[field] = move_data.get(field)
        moveFilled['stat_changes'] = extract_stat_changes(move_data.get('stat_changes', []))
        moveFilled['type'] = extract_type_name(move_data.get('type'))
        moveFilled['damage_class'] = extract_damage_class_name(move_data.get('damage_class'))


        # Extract and clean flavor text
        raw_flavor_text = extract_gen4_english_flavor_text(move_data.get('flavor_text_entries', []))
        moveFilled['flavor_text'] = clean_flavor_text(raw_flavor_text)

        # Add meta data to move
        moveFilled['meta'] = meta_data

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


        # Move definition
        raw_name = move['name']
        formatted_name = format_move_name(raw_name).replace('"', '\\"')

        # Escape the flavor text properly (already cleaned)
        flavor = move['flavor_text'].replace('"', '\\"')

        type_enum = format_type_enum(move['type'])
        category_enum = format_category_enum(move['damage_class'])

        accuracy = move['accuracy'] if move['accuracy'] is not None else -1
        power = move['power'] if move['power'] is not None else -1

        stats = "{" + ", ".join(str(s) for s in move['stat_changes']) + "}"

        # Extract meta fields
        meta = move['meta']
        ailment_enum = format_ailment_enum(meta['ailment'])

        source_content += f"""    static const Move move_{move_id} = {{
        {move_id},
        "{formatted_name}",
        "{flavor}",
        {type_enum},
        {power},
        {accuracy},
        {move['priority']},
        {category_enum},
        {stats},
        {ailment_enum},
        {meta['min_hits']},
        {meta['max_hits']},
        {meta['min_turns']},
        {meta['max_turns']},
        {meta['drain']},
        {meta['healing']},
        {meta['crit_rate']},
        {meta['ailment_chance']},
        {meta['flinch_chance']},
        {meta['stat_chance']}
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

else:
    print("Error: No valid moves to generate")

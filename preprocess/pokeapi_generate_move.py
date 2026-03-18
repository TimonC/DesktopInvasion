import urllib.request
import json
from pathlib import Path

#Browser header to avoid http rejection
opener = urllib.request.build_opener()
opener.addheaders = [('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36')]
urllib.request.install_opener(opener)

# List of move IDs to skip
#todo: sonic boom, dragon rage, counter, mirror coat, metronome, selfdestruct, dream eater, DITTO, super fang, SWAGGER/DRACOMETEOR/FLATTER,
#pain split? 220
SKIP_MOVE_IDS = [
    3,
    4,
    5,
    6,
    12,
    13,
    18,
    19,
    20,
    24,
    26,
    28,
    31,
    32,
    32,
    35,
    41,
    42,
    46,
    50,
    54,
    63,
    67,
    69,
    76,
    80,
    83,
    90,
    99,
    100,
    101,
    102,
    104,
    107,
    108,
    113,
    114,
    115,
    116,
    117,
    128,
    129,
    130,
    131,
    134,
    136,
    140,
    143,
    144,
    146,
    148,
    154,
    155,
    156,
    160,
    161,
    164,
    165,
    166,
    167,
    168,
    169,
    170,
    171,
    173,
    174,
    175,
    176,
    179,
    180,
    182,
    185,
    187,
    191,
    193,
    194,
    195,
    197,
    198,
    199,
    200,
    201,
    203,
    205,
    210,
    212,
    213,
    214,
    215,
    216,
    217,
    218,
    219,
    220,
    226,
    227,
    228,
    229,
    230,
    233,
    237,
    240,
    241,
    244,
    248,
    250,
    251,
    252,
    253,
    254,
    255,
    256,
    258,
    259,
    262,
    263,
    264,
    265,
    266,
    267,
    268,
    269,
    270,
    271,
    272,
    273,
    274,
    275,
    277,
    278,
    279,
    281,
    282,
    283,
    285,
    286,
    287,
    288,
    289,
    290,
    291,
    292,
    293,
    300,
    301,
    307,
    308,
    311,
    312,
    316,
    323,
    328,
    329,
    331,
    333,
    335,
    338,
    340,
    343,
    346,
    350,
    353,
    356,
    357,
    358,
    360,
    361,
    362,
    363,
    364,
    365,
    366,
    367,
    368,
    369,
    371,
    372,
    373,
    374,
    375,
    376,
    377,
    378,
    379,
    380,
    381,
    382,
    383,
    384,
    385,
    386,
    387,
    388,
    389,
    390,
    391,
    392,
    393,
    415,
    416,
    419,
    432,
    433,
    439,
    445,
    446,
    447,
    449,
    458,
    459,
    460,
    461,
    462,
    463,
    467,
]

# Approved ailments (matching the Ailment enum). 'Badly poisoned' is inserted based on known move name
APPROVED_AILMENTS = ['none','burn', 'freeze', 'paralysis', 'poison', 'sleep', 'confusion']

moves = []
pokeApiFieldsToInclude = ["id", "name", "accuracy", "priority", "power"]

def should_skip_move(move_id):
    """Check if a move should be skipped"""
    return move_id in SKIP_MOVE_IDS or move_id >= 10000

def should_skip_move_by_ailment(ailment_name):
    """Check if a move should be skipped based on its ailment"""
    return ailment_name not in APPROVED_AILMENTS

def extract_stat_changes(stat_changes_list):
    """Extract stat changes into a list of 5 integers: [attack, sp_attack, defense, sp_defense, speed]"""
    stats = [0, 0, 0, 0, 0]
    stat_map = {'attack': 0, 'special-attack': 1, 'defense': 2, 'special-defense': 3, 'speed': 4}
    for stat_change in stat_changes_list:
        stat_name = stat_change.get('stat', {}).get('name', '')
        change_value = stat_change.get('change', 0)
        if stat_name in stat_map:
            stats[stat_map[stat_name]] = change_value
    return stats

def extract_type_name(type_obj):
    """Extract the name field from the type object"""
    return type_obj.get('name', '') if type_obj else ''

def extract_damage_class_name(damage_class_obj):
    """Extract the name field from the damage_class object"""
    return damage_class_obj.get('name', '') if damage_class_obj else ''

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

def format_ailment_enum(ailment_name, move_name):
    if should_skip_move_by_ailment(ailment_name):
        return 'Ailment::Null'
    """Convert ailment name to Ailment enum format (e.g., 'burn' -> 'Ailment::Burn')"""
    if not ailment_name or ailment_name == 'none':
        return 'Ailment::Null'
    if move_name=="toxic" or move_name =="poison-fang":  #exception for "badly poisoned"
        ailment_name = "toxic"
    return f'Ailment::{ailment_name.capitalize()}'

def format_move_name(name):
    """Format move name: capitalize first letter and first letter after each dash, replace dashes with spaces"""
    if not name:
        return name
    words = name.split('-')
    formatted_words = [word.capitalize() for word in words]
    return ' '.join(formatted_words)

def extract_platinum_english_flavor_text(flavor_text_entries):
    """Extract the English flavor text from Generation 4)"""
    for entry in flavor_text_entries:
        language = entry.get('language', {}).get('name', '')
        version_group = entry.get('version_group', {}).get('name', '')
        if language == 'en' and version_group == 'platinum':
            return entry.get('flavor_text', '')
    return ''

def clean_flavor_text(text):
    """Clean flavor text: replace newlines with spaces and normalize"""
    if not text:
        return ""
    text = text.replace('\n', ' ').replace('\r', ' ').replace('\f', ' ')
    while '  ' in text:
        text = text.replace('  ', ' ')
    return text.strip()

def extract_meta_data(meta):
    """Extract meta data fields from the move's meta object"""
    if not meta:
        return None
    ailment_name = meta.get('ailment', {}).get('name', 'none')

    def convert_null_to_neg_one(value):
        return -1 if value is None else value

    return {
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

        meta_data = extract_meta_data(move_data.get('meta'))
        if meta_data is None:
            print(f"Skipped move (unapproved ailment): {move_id} - {move_data.get('name')}")
            continue

        moveFilled = {field: move_data.get(field) for field in pokeApiFieldsToInclude}
        moveFilled['stat_changes'] = extract_stat_changes(move_data.get('stat_changes', []))
        moveFilled['type'] = extract_type_name(move_data.get('type'))
        moveFilled['damage_class'] = extract_damage_class_name(move_data.get('damage_class'))
        raw_flavor_text = extract_platinum_english_flavor_text(move_data.get('flavor_text_entries', []))
        moveFilled['flavor_text'] = clean_flavor_text(raw_flavor_text)
        moveFilled['meta'] = meta_data
        moves.append(moveFilled)
        print(f"Added move: {moveFilled['id']} - {moveFilled['name']}")

def generate_moves_data_direct():
    """Generate C++ source using direct pointer array (simpler & faster)"""
    if not moves:
        return ""

    valid_moves = [m for m in moves if not should_skip_move(m['id'])]
    valid_moves.sort(key=lambda x: x['id'])
    if not valid_moves:
        return ""

    max_move_id = max(m['id'] for m in valid_moves)

    source_content = """#include "data_move.h"

namespace {
"""

    # Create all moves in anonymous namespace with const qualifiers
    for move in valid_moves:
        move_id = move['id']
        raw_name = move['name']
        formatted_name = format_move_name(raw_name).replace('"', '\\"')
        flavor = move['flavor_text'].replace('"', '\\"')
        type_enum = format_type_enum(move['type'])
        category_enum = format_category_enum(move['damage_class'])
        accuracy = move['accuracy'] if move['accuracy'] is not None else -1
        power = move['power'] if move['power'] is not None else -1
        stats = "{" + ", ".join(str(s) for s in move['stat_changes']) + "}"

        meta = move['meta']
        ailment_enum = format_ailment_enum(meta['ailment'], move['name'])

        source_content += f"""    static constexpr Move move_{move_id} = {{
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

    source_content += "} \n\n"

    # Create the direct pointer array (kMovesByIndex)
    source_content += f"const Move* const kMovesByIndex[{max_move_id + 1}] = {{\n"

    move_dict = {move['id']: f"&move_{move['id']}" for move in valid_moves}
    for i in range(max_move_id + 1):
        if i in move_dict:
            source_content += f"    {move_dict[i]}, \n"
        else:
            source_content += f"    nullptr,  \n"

    source_content += "};\n\n"
    source_content += f"const int kMaxMoveId = {max_move_id};\n"
    source_content += f"const int kMoveCount = {len(valid_moves)};\n"

    return source_content

# Generate the C++ file
source_content = generate_moves_data_direct()
if source_content:
    output_path = Path("src/data_move.cpp")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(source_content)
    valid_move_count = len([m for m in moves if not should_skip_move(m['id'])])
    print(f"\nGenerated ../src/data_move.cpp")
    print(f"Total moves: {valid_move_count}")
    print(f"Max move ID: {max(m['id'] for m in moves if not should_skip_move(m['id']))}")
    print(f"Array size: {max(m['id'] for m in moves if not should_skip_move(m['id'])) + 1}")
else:
    print("Error: No valid moves to generate")

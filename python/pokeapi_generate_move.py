from skip_move_ids import SKIP_MOVE_IDS #This list of moves to exclude was manually compiled to drastically reduce complexity of DesktopInvasion

import urllib.request
import json
from pathlib import Path

#Browser header to avoid http rejection
opener = urllib.request.build_opener()
opener.addheaders = [('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36')]
urllib.request.install_opener(opener)


#This list was made by feeding all the stat-changing moves to DeepSeek, and defining exceptions as:
#"when a positive stat change applies to opponent, and when a negative stat change applies to player"
MOVE_EFFECT_EXCEPTION_LIST = [
    207,  # Swagger: +2 Attack to TARGET (confuses, but buffs opponent)
    260,  # Flatter: +1 Sp. Atk to TARGET (confuses, but buffs opponent)
    276,  # Superpower: -1 Attack, -1 Defense to USER (self-debuff after damage)
    315,  # Overheat: -2 Sp. Atk to USER (self-debuff after damage)
    354,  # Psycho Boost: -2 Sp. Atk to USER (self-debuff after damage)
    359,  # Hammer Arm: -1 Speed to USER (self-debuff after damage)
    370,  # Close Combat: -1 Defense, -1 Sp. Defense to USER (self-debuff after damage)
    434,  # Draco Meteor: -2 Sp. Atk to USER (self-debuff after damage)
    437,  # Leaf Storm: -2 Sp. Atk to USER (self-debuff after damage)
]

#This list was generated in this script, and then inserted with manually set break points inside "filled"
filled_move_names_longer_than_eight_characters = [
    ("supersonic", "Super-sonic"),
    ("flamethrower", "Flame-thrower"),
    ("submission", "Sub-mission"),
    ("thunderbolt", "Thunder-bolt"),
    ("earthquake", "Earth-quake"),
    ("confusion", "Con-fusion"),
    ("smokescreen", "Smoke-screen"),
    ("waterfall", "Water-fall"),
    ("constrict", "Con-strict"),
    ("crabhammer", "Crab-hammer"),
    ("aeroblast", "Aero-blast"),
    ("octazooka", "Octa-zooka"),
    ("magnitude", "Magni-tude"),
    ("synthesis", "Syn-thesis"),
    ("moonlight", "Moon-light"),
    ("superpower", "Super-power"),
    ("extrasensory", "Extra-sensory"),
    ("discharge", "Dis-charge")
]
move_names_longer_than_eight_characters = []

# Approved ailments (matching the Ailment enum). 'Badly poisoned' is inserted based on known move name
APPROVED_AILMENTS = ['none','burn', 'freeze', 'paralysis', 'poison', 'sleep', 'confusion']
pokeApiFieldsToInclude = ["id", "name", "accuracy", "priority", "power"]

moves = []

def should_skip_move(move_id):
    """Check if a move should be skipped"""
    return move_id in SKIP_MOVE_IDS or move_id >= 10000

def should_skip_move_by_ailment(ailment_name):
    """Check if a move should be skipped based on its ailment"""
    return ailment_name not in APPROVED_AILMENTS

def extract_stat_changes(stat_changes_list):
    """Extract stat changes into a list of 7 integers: [attack, sp_attack, defense, sp_defense, speed, accuracy, evasion]"""
    stats = [0, 0, 0, 0, 0, 0, 0]
    stat_map = {'attack': 0, 'special-attack': 1, 'defense': 2, 'special-defense': 3, 'speed': 4, 'accuracy': 5, 'evasion': 6}
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

    return {
        'ailment': ailment_name,
        'drain': meta.get('drain', 0),
        'healing': meta.get('healing', 0),
        'crit_rate': meta.get('crit_rate', 0),
        'ailment_chance': meta.get('ailment_chance', 0),
        'flinch_chance': meta.get('flinch_chance', 0),
        'stat_chance': meta.get('stat_chance', 0)
    }

def is_tm_move(machines_list):
    """Return True if the move is learnable via TM in any generation"""
    return len(machines_list) > 0

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

        moveFilled = {field: move_data.get(field) for field in pokeApiFieldsToInclude}
        moveFilled['stat_changes'] = extract_stat_changes(move_data.get('stat_changes', []))
        moveFilled['type'] = extract_type_name(move_data.get('type'))
        moveFilled['damage_class'] = extract_damage_class_name(move_data.get('damage_class'))
        raw_flavor_text = extract_platinum_english_flavor_text(move_data.get('flavor_text_entries', []))
        moveFilled['flavor_text'] = clean_flavor_text(raw_flavor_text)
        moveFilled['meta'] = meta_data
        moveFilled['is_tm'] = is_tm_move(move_data.get('machines', []))
        moves.append(moveFilled)

        name = moveFilled['name']
        if len(name)>8 and not "-" in name:
            move_names_longer_than_eight_characters.append(name)
        print(f"Added move: {moveFilled['id']} - {name}")

def generate_moves_data_direct():
    """Generate C++ source using direct pointer array (simpler & faster)"""
    if not moves:
        return ""

    valid_moves = [m for m in moves if not should_skip_move(m['id'])]
    valid_moves.sort(key=lambda x: x['id'])
    if not valid_moves:
        return ""

    max_move_id = max(m['id'] for m in valid_moves)

    tm_move_ids = sorted([m['id'] for m in valid_moves if m['is_tm']])

    source_content = """#include "data_move.h"

namespace {
"""

    # Create all moves in anonymous namespace with const qualifiers
    for move in valid_moves:

        move_id = move['id']
        raw_name = move['name']

        #If longer than eight chars, fill in dash at the manually defined break point
        longer_than_eight = False
        formatted_name = "THIS SHOULD ALWAYS BE OVERWRITTEN"
        for original, formatted in filled_move_names_longer_than_eight_characters:
            longer_than_eight = raw_name == original
            if longer_than_eight:
                formatted_name = formatted
                break
        if not longer_than_eight:
            formatted_name = format_move_name(raw_name)

        flavor = move['flavor_text'].replace('"', '\\"')
        type_enum = format_type_enum(move['type'])
        category_enum = format_category_enum(move['damage_class'])
        accuracy = move['accuracy'] if move['accuracy'] is not None else -1
        power = move['power'] if move['power'] is not None else -1
        if formatted_name == "Magnitude" or formatted_name == "Magni-tude":
           power =  70

        #Stat changes are Atk/SpAtk/Def/SpDef in PokeApi, DesktopInvasion has Atk/Def/S;atk/SpDef so need to swap
        stat_changes  = move['stat_changes']
        spAtk = stat_changes[1]
        stat_changes[1] = stat_changes[2]
        stat_changes[2] = spAtk
        stats = "{" + ", ".join(str(s) for s in stat_changes) + "}"


        meta = move['meta']
        ailment_enum = format_ailment_enum(meta['ailment'], move['name'])

        if should_skip_move_by_ailment(meta['ailment']):
            raise RuntimeError(f"Move {move_id}, {raw_name} should be not be included, invalid ailment")

        #Below block assumes a move never does both negative and positive status changes, curse is not included so we good
        stat_change_target = -1
        if stats != "{0, 0, 0, 0, 0, 0, 0}":
            effect_target = True if "-" in stats else False
            if move_id in MOVE_EFFECT_EXCEPTION_LIST:
                effect_target = not effect_target
            stat_change_target = 1 if effect_target else 0


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
        {meta['drain']},
        {meta['healing']},
        {meta['crit_rate']},
        {meta['ailment_chance']},
        {meta['flinch_chance']},
        {meta['stat_chance']},
        {stat_change_target}
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
    source_content += f"const int kMoveCount = {len(valid_moves)};\n\n"

    # Create the TM move ID list (kAllTmIds)
    tm_ids_str = ", ".join(str(mid) for mid in tm_move_ids)
    source_content += f"const int kTmCount = {len(tm_move_ids)};\n"
    source_content += f"const int kAllTmIds[{len(tm_move_ids)}] = {{{tm_ids_str}}};\n"

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
    print(f"TM moves: {len([m for m in moves if not should_skip_move(m['id']) and m['is_tm']])}")

    with open("move_names_longer_than_eight_characters.txt", "w") as t:
        for move in move_names_longer_than_eight_characters:
            t.write(move + "\n")
else:
    print("Error: No valid moves to generate")

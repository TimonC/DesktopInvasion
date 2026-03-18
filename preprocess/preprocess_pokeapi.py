import urllib.request
import json

moves = []
pokeApiFieldsToInclude = [
    "id",
    "name",
    "accuracy",
    "effect_chance",
    "priority",
    "power",
    # "effect_entries,
    # "effect_changes,
    # "learned_by_pokemon",
    # "flavor_text_entries",
    # "type",
    # "stat_changes",
]

for i in range(1, 4):
    response = urllib.request.urlopen(f"https://pokeapi.co/api/v2/generation/{i}/")
    contents = response.read()
    data = json.loads(contents.decode('utf-8'))

    for move in data.get('moves', []):
        move_url = move.get('url')
        move_response = urllib.request.urlopen(move_url)
        move_data = json.loads(move_response.read().decode('utf-8'))

        moveFilled = {}
        for field in pokeApiFieldsToInclude:
            moveFilled[field] = move_data.get(field)  # Changed from move.get() to move_data.get()

        moves.append(moveFilled)
        print(moves[-1])

with open('moves.txt', 'w', encoding='utf-8') as f:
    for move in moves:
        stringToWrite = ""
        for field, value in move.items():
            stringToWrite += f"{field}: {value} "
        f.write(stringToWrite + "\n")

print(f"Total moves: {len(moves)}")
print("Written to 'moves.txt'")

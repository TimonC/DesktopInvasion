import urllib.request
import json

moves = []

for i in range(1, 5):
    response = urllib.request.urlopen(f"https://pokeapi.co/api/v2/generation/{i}/")
    contents = response.read()
    data = json.loads(contents.decode('utf-8'))

    for move in data.get('moves', []):
        move_url = move.get('url')
        move_response = urllib.request.urlopen(move_url)
        move_data = json.loads(move_response.read().decode('utf-8'))

        moves.append({"name": move_data.get('name'), "id": move_data.get('id')})
        print(moves[-1])

# Write to file
with open('moves.txt', 'w', encoding='utf-8') as f:
    for move in moves:
        f.write(f"ID: {move['id']}, Name: {move['name']}\n")

print(f"Total moves: {len(moves)}")
print("Written to 'moves.txt'")

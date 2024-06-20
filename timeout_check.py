import json
import requests
import time
import re

def get_games_by_player(player_id):
    all_games = requests.post(
        'https://www.codingame.com/services/gamesPlayersRanking/findLastBattlesByAgentId',
        json = [str(player_id), None]
    ).json()

    games_id = []

    for game in all_games:
        games_id.append((player_id, game['gameId']))

    return games_id

def analyze_games(games):

    hurdle_games = 0
    archery_games = 0
    skating_games = 0
    diving_games = 0

    command_cnt = {}

    timeouts = 0
    won_games = 0

    for player_id, game_id in games:

        time.sleep(0.05)

        replay = requests.post(
            'https://www.codingame.com/services/gameResult/findByGameId',
            json = [str(game_id), None]
        ).json()

        print("game_id: ", game_id)        
        # print(replay)

        # print(replay)

        player_agent_id = -1
        for i in range(3):
            if replay['agents'][i]['agentId'] == player_id:
                player_agent_id = i
                break

        if replay['scores'][player_agent_id] == -1:
            print(f"TIMEOUT in {game_id} by {player_id}")
            timeouts += 1
        
        print("timeouts: ", timeouts)
        
        continue
        
        if replay['ranks'][2] != player_agent_id:
            continue
            
        won_games += 1
            
        for frame in replay['frames']:
        
            if 'summary' in frame:
                summary = frame['summary']
                    
                for report in summary.split('Game over'):
                    if f'{player_agent_id} earns a GOLD medal' in report:
                        if 'Hurdle' in report:
                            hurdle_games += 3

                        if 'Archery' in report:
                            archery_games += 3

                        if 'Skating' in report:
                            skating_games += 3

                        if 'Diving' in report:
                            diving_games += 3

                    if f'{player_agent_id} earns a SILVER medal' in report:
                        if 'Hurdle' in report:
                            hurdle_games += 1

                        if 'Archery' in report:
                            archery_games += 1

                        if 'Skating' in report:
                            skating_games += 1

                        if 'Diving' in report:
                            diving_games += 1


            if frame['agentId'] != player_agent_id:
                continue

            if 'stdout' in frame:
                action = frame['stdout']

                matches = re.findall(r'\b(LEFT|RIGHT|UP|DOWN)\b', action)

                action = ' '.join(matches)

                if action not in command_cnt:
                    command_cnt[action] = 1
                else:
                    command_cnt[action] += 1


        print("timeouts: ", timeouts)
        # print(hurdle_games / won_games, archery_games / won_games, skating_games / won_games, diving_games / won_games)
        print(command_cnt)

    hurdle_games /= won_games
    archery_games /= won_games
    skating_games /= won_games
    diving_games /= won_games

    print("won games: ", won_games)
    print("timeouts: ", timeouts)
    print(hurdle_games, archery_games, skating_games, diving_games)
    exit(-1)


def get_top_players(cnt):
    ranks = requests.post(
        'https://www.codingame.com/services/Leaderboards/getFilteredChallengeLeaderboard',
        json = [
            "summer-challenge-2024-olymbits",
            "ea0ce7b64a1fb206d6fcfcd0841eaaab5264483",
            "global",
            {
                "active": "true",
                "column": "SCHOOLCOMPANY",
                "filter": "ALL"
            }
        ]
    ).json()['users']

    top_players = []

    for i in range(cnt):
        print(int(ranks[i]['agentId']), ranks[i]['pseudo'])
        top_players.append(int(ranks[i]['agentId']))

    return top_players

# top_players = get_top_players(7)
# print(top_players)

top_players = [5474752]
# print(786967972, 3844625)
# exit(-1)

games = []

for player_id in top_players:
    player_games = get_games_by_player(player_id)
    print(f"gathered: {len(player_games)} games from {player_id}")
    games = games + player_games

print("games: ", len(games))

analyze_games(games)

# 9769 11316 10484 11350
# avg sets in one match
# 5.575913242009133 6.458904109589041 5.9840182648401825 6.478310502283105

# p of move
# {'LEFT': 32502, 'RIGHT': 45302, 'DOWN': 40132, 'UP': 59298, '': 18}
# 0.1833846778834761 0.25560558357877156 0.22643510838778114 0.

# avg of medals in won games
# 9.543859649122806 12.011695906432749 11.286549707602338 10.760233918128655

# avg of medals in lost games
# 8.206030150753769 10.683417085427136 7.693467336683417 8.50251256281407
